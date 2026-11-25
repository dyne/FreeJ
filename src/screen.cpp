/*  FreeJ
 *  (c) Copyright 2001 - 2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <config.h>

#include <context.h>
#include <screen.h>
#include <layer.h>
#include <video_encoder.h>
#ifdef WITH_AUDIO
#include <audio_jack.h>
#endif
#include <ringbuffer.h>
#include "engine/frame_storage.h"
#include "engine/surface_lock.h"

#ifdef WITH_GD
#include <gd.h>
#endif

ViewPort::ViewPort()
  : Entry(), frames_(new freej_engine::FrameStore) {

  opengl = false;

  changeres       = false;
  resizing        = false;

  resize_w = 0;
  resize_h = 0;

  jsclass = NULL;
  jsobj = NULL;

  audio = NULL;
  m_SampleRate=NULL;
  indestructible = false;
#ifdef WITH_AUDIO
  // if compiled with audio initialize the audio data pipe
//   audio = ringbuffer_create(1024 * 512);
  audio = ringbuffer_create(4096 * 512 * 8);
#endif
}

ViewPort::~ViewPort() {
  reset();
  if (audio) {
#ifdef WITH_AUDIO
    JackClient::ResetRingbuffer(audio, true);
#endif
    ringbuffer_free(audio);
  }
}

bool ViewPort::init(int w, int h, int bpp) {

  if(bpp!=32) {
    warning("FreeJ is forced to use 32bit pixel formats, hardcoded internally");
    warning("you are initializing a ViewPort with a different bpp value");
    warning("please submit a patch if you can make it :)");
    return false;
  }

  geo.init(w,h,bpp);
  initialized = _init();
  act("screen %s initialized with size %ux%u",
      name, geo.w, geo.h);

  return initialized;

}

bool ViewPort::add_layer(Layer *lay) {
  func("%s",__PRETTY_FUNCTION__);

  std::lock_guard<Linklist<Layer>> list_lock(layers);
  std::lock_guard<Layer> layer_lock(*lay);
  if(lay->list || lay->screen) {
    warning("passing a layer from a screen to another is not (yet) supported");
    return(false);
  }

  if(!lay->opened) {
    error("layer %s is not yet opened, can't add it");
    return(false);
  }

  lay->screen = this;
  
  setup_blits( lay );

  // setup default blit (if any)
  if (lay->blitter) {
    lay->current_blit =
      (Blit*)lay->blitter->default_blit;
    lay->blitter->blitlist.sel(0);
    lay->current_blit->sel(true);
  } 
  // center the position
  //lay->geo.x = (screen->w - lay->geo.w)/2;
  //lay->geo.y = (screen->h - lay->geo.h)/2;
  //  screen->blitter->crop( lay, screen );
  layers.prepend(lay);
  layers.sel(0);
  lay->sel(true);
  lay->active = true;
  if (lay->resume_on_attach_) {
    const int started = lay->start();
    if (started != 0 && started != EBUSY) {
      lay->rem();
      lay->screen = nullptr;
      lay->active = false;
      delete lay->blitter;
      lay->blitter = nullptr;
      lay->current_blit = nullptr;
      error("could not restart layer %s: %s", lay->name, strerror(started));
      return false;
    }
    lay->resume_on_attach_ = false;
  }
  func("layer %s added to screen %s",lay->name, name);
  return(true);
}

#ifdef WITH_AUDIO
bool ViewPort::add_audio(JackClient *jcl) {
  std::lock_guard<Linklist<Layer>> list_lock(layers);
  const Layer *source = nullptr;
  int rate = 0;
  int channels = 0;
  for (auto *layer = layers.begin(); layer; layer = static_cast<Layer *>(layer->next)) {
    int candidate_rate, candidate_channels;
    if (!layer->audio_format(candidate_rate, candidate_channels))
      continue;
    if (source)
      return false;
    source = layer;
    rate = candidate_rate;
    channels = candidate_channels;
  }
  return source && bind_audio(jcl, *source, rate, channels);
}

bool ViewPort::add_audio(JackClient *jcl, const Layer &source) {
  std::lock_guard<Linklist<Layer>> list_lock(layers);
  int rate, channels;
  return source.list == &layers && source.audio_format(rate, channels)
      && bind_audio(jcl, source, rate, channels);
}

bool ViewPort::bind_audio(JackClient *jcl, const Layer &source, int rate, int channels) {
  std::lock_guard<std::mutex> lock(audio_mutex_);
  if (!jcl || !jcl->IsAttached() || !audio)
    return false;
  const int output_rate = jcl->m_SampleRate ? jcl->m_SampleRate : rate;
  if (jcl->SetRingbufferPtr(audio, output_rate, channels) != 0)
    return false;
  audio_source_ = &source;
  audio_rate_ = output_rate;
  audio_channels_ = channels;
  m_SampleRate = &jcl->m_SampleRate;
  return true;
}

int ViewPort::audio_rate(const Layer &source) {
  std::lock_guard<std::mutex> lock(audio_mutex_);
  return audio_source_ == &source ? audio_rate_ : 0;
}

void ViewPort::write_audio(const Layer &source, const float *samples, std::size_t count,
                           int channels, int rate) {
  std::lock_guard<std::mutex> lock(audio_mutex_);
  if (audio_source_ != &source || audio_rate_ != rate || audio_channels_ != channels)
    return;
  const auto bytes = count * sizeof(*samples);
  if (ringbuffer_write_space(audio) >= bytes)
    ringbuffer_write(audio, reinterpret_cast<const char *>(samples), bytes);
}

void ViewPort::flush_audio(const Layer &source) {
  std::lock_guard<std::mutex> lock(audio_mutex_);
  if (audio_source_ == &source)
    JackClient::ResetRingbuffer(audio, false);
}
#endif

void ViewPort::rem_layer(Layer *lay) {
  {
    std::lock_guard<Linklist<Layer>> list_lock(layers);
    std::lock_guard<Layer> layer_lock(*lay);
    if (lay->list != &layers || lay->screen != this)
      return;
    lay->resume_on_attach_ = lay->is_running();
    lay->rem();
  }

  // The worker can take the layer lock to publish or run a queued operation.
  // Unlink first and release both locks before joining it.
  lay->deferred_calls->close();
  lay->stop();
  std::lock_guard<Layer> layer_lock(*lay);
#ifdef WITH_AUDIO
  {
    std::lock_guard<std::mutex> lock(audio_mutex_);
    if (audio_source_ == lay) {
      JackClient::ResetRingbuffer(audio, true);
      audio_source_ = nullptr;
      m_SampleRate = nullptr;
    }
  }
#endif
  lay->screen = nullptr;
  lay->clear_frame();
  delete lay->blitter;
  lay->blitter = nullptr;
  lay->current_blit = nullptr;
  lay->deferred_calls->reopen();
  notice("removed layer %s (but still present as an instance)", lay->name);
}

void ViewPort::reset() {
  stop_encoders();
  while (Layer *layer = layers.begin())
    rem_layer(layer);
  frames_->clear();
  capture_active_ = false;
}

void ViewPort::stop_encoders() {
  while (VideoEncoder *encoder = encoders.begin()) {
    encoder->rem();
    encoder->stop();
    delete encoder;
  }
}

bool ViewPort::add_encoder(VideoEncoder *enc) {
  func("%s",__PRETTY_FUNCTION__);
 
  if(enc->list) {
    error("moving an encoder from one screen to another is not supported");
    return(false);
  }

  func("initializing encoder %s",enc->name);
  if(!enc->init(this)) {
    error("%s : failed initialization", __PRETTY_FUNCTION__);
    return(false);
  }
  func("initialization done");

  encoders.append(enc);
  freej_engine::FrameAccess::capture(*this);
  const int started = enc->start();
  if (started != 0) {
    enc->rem();
    error("could not start encoder %s: %s", enc->name, strerror(started));
    return false;
  }

  enc->active = true;

  encoders.sel(0);

  enc->sel(true);

  act("encoder %s added to screen %s", enc->name, name);
  return true;
}

#ifdef WITH_GD
void ViewPort::save_frame(char *file) {
  FILE *fp;
  gdImagePtr im;
  int *src;
  int x,y;

  im = gdImageCreateTrueColor(geo.w, geo.h);
  src = (int*)coords(0,0);
  for(y=0; y < geo.h; y++) {
    for (x=0; x < geo.w; x++) {
      gdImageSetPixel(im, x, y, src[x] & 0x00FFFFFF);
      //im->tpixels[y][x] = src[x] & 0x00FFFFFF;
    }
    src += geo.w;
  }
  fp = fopen(file, "wb");
  gdImagePng(im,fp);
  fclose(fp);
}
#endif


void ViewPort::blit_layers() {
  std::lock_guard<Linklist<Layer>> list_lock(layers);
  for (Layer *layer = layers.end(); layer; layer = static_cast<Layer *>(layer->prev)) {
    std::lock_guard<Layer> layer_lock(*layer);
    if (!layer->buffer || !layer->active || !layer->opened)
      continue;

    const auto frame = freej_engine::FrameAccess::layer(*layer);
    const freej_engine::SurfaceLock display(*this);
    if (display)
      blit(layer);
  }
}


void ViewPort::handle_resize() {
  {
    std::lock_guard<std::mutex> lock(geometry_mutex_);
    if (resizing) {
      resize(resize_w, resize_h);
      resizing = false;
    }
  }
  
  /* crop all layers to new screen size */
  std::lock_guard<Linklist<Layer>> list_lock(layers);
  Layer *lay = layers.begin ();
  while (lay) {
    lay -> lock ();
    lay -> blitter->crop(lay, this);
    lay -> unlock ();
    lay = (Layer*) lay -> next;
  } 
}

void ViewPort::resize(int resize_w, int resize_h) {  // nop
  resize_w = 0; resize_h = 0;
};

void ViewPort::show() { };

void ViewPort::clear() { };

void ViewPort::fullscreen() { };
