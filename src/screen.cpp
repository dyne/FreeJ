/*  FreeJ
 *  (c) Copyright 2001 - 2009 Denis Roio <jaromil@dyne.org>
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
#include <audio_jack.h>
#include <ringbuffer.h>

#include <scale2x.h>
#include <scale3x.h>

#include <video_layer.h>

ViewPort::ViewPort()
  : Entry() {

  opengl = false;

  magnification   = 0;
  changeres       = false;


  audio = NULL;
  m_SampleRate=NULL;
#ifdef WITH_AUDIO
  // if compiled with audio initialize the audio data pipe
  audio = ringbuffer_create(1024 * 512);
#endif
}

ViewPort::~ViewPort() {

  func("screen %s deleting %u layers", name, layers.len() );
  Layer *lay;
  lay = layers.begin();
  while(lay) {
    lay->rem();
    // deleting layers crashes
    //    delete(lay);
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

}

bool ViewPort::init(int w, int h) {

  this->w = w;
  this->h = h;
  bpp = 32; // we use only RGBA
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  return _init(w, h);

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
    lay->rem();
    notice("removed layer %s (but still present as an instance)", lay->name);
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
}


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
  if (magnification) {
    set_magnification (magnification);
    magnification = 0;
  }
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

void ViewPort::scale2x(uint32_t *osrc, uint32_t *odst) {

      /* apply scale2x to screen */
    int c;
    uint32_t *src, *dst, dw;
    src = osrc;
    dst = odst;
    dw = w*2;

#if defined(__GNUC__) && defined(__i386__)
    scale2x_32_mmx(dst,dst+dw,
		   src,src,src+w,w);
#else
    scale2x_32_def(dst,dst+dw,
		   src,src,src+w,w);
#endif
    dst += dw<<1;
    src += w;
    for(c=0;c<h-2;c++) {
#if defined(__GNUC__) && defined(__i386__)      
      scale2x_32_mmx(dst,dst+dw,
		     src-w,src,src+w,w);
#else
      scale2x_32_def(dst,dst+dw,
		     src-w,src,src+w,w);
#endif
      dst += dw<<1;
      src += w;
    }
#if defined(__GNUC__) && defined(__i386__)
    scale2x_32_mmx(dst,dst+dw,
		   src-w,src,src,w);
    scale2x_mmx_emms();
#else
    scale2x_32_def(dst,dst+dw,
		   src-w,src,src,w);
#endif

}

void ViewPort::scale3x(uint32_t *osrc, uint32_t *odst) {

  /* apply scale3x to screen */
  int c;
  uint32_t *src, *dst, tw;
  src = osrc;
  dst = odst;
  tw = w*3;
  
  scale3x_32_def(dst,dst+tw,dst+tw+tw,
		 src,src,src+w,w);
  dst += tw*3;
  src += w;
  for(c=0;c<h-2;c++) {
    
    scale3x_32_def(dst,dst+tw,dst+tw+tw,
		   src-w,src,src+w,w);
    
    dst += tw*3;
    src += w;
  }
  
  scale3x_32_def(dst,dst+tw,dst+tw+tw,
		 src-w,src,src,w);

}
