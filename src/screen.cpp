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

#include <video_layer.h>

#ifdef WITH_GD
#include <gd.h>
#endif

ViewPort::ViewPort()
  : Entry() {

  opengl = false;

  changeres       = false;
  deleted = false;

  jsclass = NULL;
  jsobj = NULL;

  audio = NULL;
  m_SampleRate=NULL;
  indestructible = false;
#ifdef WITH_AUDIO
  // if compiled with audio initialize the audio data pipe
  audio = ringbuffer_create(1024 * 512);
#endif
}

ViewPort::~ViewPort() {

  if(deleted) {
    warning("double deletion of Screen %s", name);
    return;
  }

  func("screen %s deleting %u layers", name, layers.len() );
  Layer *lay;
  lay = layers.begin();
  while(lay) {
    lay->rem();
    // deleting layers crashes
    //    delete(lay);
    // XXX - you don't create layers... so you don't have to delete them as well!!!
    //       symmetry is of primary importance and we should care about that.
    //       layers should be freed by who created them. Perhaps we could extend 
    //       the Layer api to allow notifications when a layer is removed from a screen
    lay = layers.begin();
  }

  if(audio) ringbuffer_free(audio);

  func("screen %s deleting %u encoders", name, encoders.len() );
  VideoEncoder *enc;
  enc = encoders.begin();
  while(enc) {
    enc->stop();
    enc->rem();
    delete(enc);
    enc = encoders.begin();
  }

  deleted = false;
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

  if(lay->list) {
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
  func("layer %s added to screen %s",lay->name, name);
  return(true);
}

#ifdef WITH_AUDIO
bool ViewPort::add_audio(JackClient *jcl) {
 	if (layers.len() == 0 ) return false;	

	jcl->SetRingbufferPtr(audio, (int) ((VideoLayer*) layers.begin())->audio_samplerate, (int) ((VideoLayer*) layers.begin())->audio_channels);
	m_SampleRate = &jcl->m_SampleRate;
	long unsigned int  m_SampleRate;	
}
#endif

void ViewPort::rem_layer(Layer *lay)
{
    lay->screen = NULL; // symmetry
    lay->rem();
    notice("removed layer %s (but still present as an instance)", lay->name);
}

void ViewPort::reset()
{
    Layer *lay;
    lay = layers.begin();
    while(lay) {
        // TODO - notify the layer that it has been removed from the screen
        lay->rem();
        lay = layers.begin();
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

  enc->start();

  enc->active = true;

  encoders.append(enc);

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
  Layer *lay;

  lay = layers.end();
  if (lay) {
    layers.lock ();
    while (lay) {

      if(lay->buffer) {

	if (lay->active & lay->opened) {

	  lay->lock();
	  lock();
	  blit(lay);
	  unlock();
	  lay->unlock();
	  
	}
      }
      lay = (Layer *)lay->prev;
    }
    layers.unlock ();
  }
  /////////// finish processing layers

}


void ViewPort::handle_resize() {
  lock ();
  if(resizing) {
    resize (resize_w, resize_h);
    resizing = false;
  }
  unlock();
  
  /* crop all layers to new screen size */
  Layer *lay = layers.begin ();
  while (lay) {
    lay -> lock ();
    lay -> blitter->crop(lay, this);
    lay -> unlock ();
    lay = (Layer*) lay -> next;
  } 
}
