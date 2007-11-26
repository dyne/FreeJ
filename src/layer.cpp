/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */

#include <string.h>

#include <SDL.h>

#include <layer.h>
#include <filter.h>

#include <context.h>
#include <jutils.h>
#include <config.h>

#include <jsparser_data.h>


Layer::Layer()
  :Entry(), JSyncThread() {
  env = NULL;
  quit = false;
  active = false;
  running = false;
  hidden = false;
  fade = false;
  use_audio = false;
  opened = true;
  bgcolor = 0;
  bgmatte = NULL;
  set_name("???");
  filename[0] = 0;
  buffer = NULL;
  screen = NULL;
  is_native_sdl_surface = false;
  jsclass = &layer_class;
  slide_x = 0;
  slide_y = 0;
  parameters = NULL;
}

Layer::~Layer() {
  FilterInstance *f = (FilterInstance*)filters.begin();
  while(f) {
    f->rem();
    delete f;
    f = (FilterInstance*)filters.begin();
  }
  if(bgmatte) jfree(bgmatte);
}

void Layer::_init(int wdt, int hgt) {

  geo.w = wdt;
  geo.h = hgt;
  geo.bpp = 32;
  geo.size = geo.w*geo.h*(geo.bpp/8);
  geo.pitch = geo.w*(geo.bpp/8);

  //  this->freej = freej;
  //  geo.fps = freej->fps;
  geo.x = 0;//(freej->screen->w - geo.w)/2;
  geo.y = 0;//(freej->screen->h - geo.h)/2;
  //  blitter.crop( freej->screen );

  /* initialize the blitter */
  blitter.init(this);

  /* allocate memory for the matte background */
//  bgmatte = jalloc(bgmatte,geo.size);
  
  func("initialized %s layer %ix%i",
	 get_name(), geo.w, geo.h);
}

void Layer::run() {
  void *tmp_buf;

  func("Layer %s :: run :: begin thread %d", name, pthread_self());

  while(!feed()) 
      jsleep (0,50);
  
  func("ok, layer %s in rolling loop",get_name());

  
  
  lock_feed();
  running = true;
  wait_feed();
  
  while(!quit) {

    lock();
    
    tmp_buf = feed();
    
    unlock();

    if(!tmp_buf) 
      func("feed returns NULL on layer %s",get_name());
    else
      buffer = tmp_buf;

    wait_feed();
  }
    
  func("Layer :: run :: end thread %p",pthread_self());
  running = false;
}

bool Layer::cafudda() {

  FilterInstance *filt;

  if(!fade)
    if(!active || hidden)
      return false;

  if(!opened) return false;

  offset = buffer;
  if(!offset) {
    signal_feed();
    return(false);
  }

  /* process thru iterators */
  if(iterators.len()) {
    iterators.lock();
    iter = (Iterator*)iterators.begin();
    while(iter) {
      res = iter->cafudda(); // if cafudda returns -1...
      itertmp = iter;
      iter = (Iterator*) ((Entry*)iter)->next;
      if(res<0) {
	iterators.unlock();
	delete itertmp; // ...iteration ended
	iterators.lock();
	if(!iter)
	  if(fade) { // no more iterations, fade out deactivates layer
	    fade = false;
	    active = false;
	  }
      }
    }
    iterators.unlock();
  }
  


  if( filters.len() ) {
    filters.lock();
    filt = (FilterInstance *)filters.begin();
    while(filt) {
      if(filt->active)
	offset = (void*) filt->process(env->fps_speed, (uint32_t*)offset);
      filt = (FilterInstance *)filt->next;
    }
  }
  
  blitter.blit();
  
  if( filters.len() ) filters.unlock();

  signal_feed();

  return(true);
}

bool Layer::set_parameter(int idx) {

  Parameter *param;
  param = (Parameter*)parameters->pick(idx);
  if( ! param) {
    error("parameter %s not found in layer %s", param->name, name );
    return false;
  } else 
    func("parameter %s found in layer %s at position %u",
	 param->name, name, idx);

  if(!param->layer_func) {
    error("no layer callback function registered in this parameter");
    return false;
  }

  (*param->layer_func)(this, param, idx);

  return true;
}



void Layer::set_filename(char *f) {
  char *p = f + strlen(f);
  while(*p!='/' && (p >= f)) 
      p--;
  strncpy(filename,p+1,256);
}

void Layer::set_position(int x, int y) {
  lock();
  slide_x = geo.x = x;
  slide_y = geo.y = y;
  //blitter.crop( screen );
  unlock();
}

void Layer::slide_position(int x, int y, int speed) {
  
  slide_x = (float)geo.x;
  slide_y = (float)geo.y;

  if(x!=geo.x) {
    iter = new Iterator(&slide_x);
    iter->set_aim((float)x);
    iter->set_step((float)speed);
    iterators.append(iter);
  }

  if(y!=geo.y) {
    iter = new Iterator(&slide_y);
    iter->set_aim((float)y);
    iter->set_step((float)speed);
    iterators.append(iter);
  }

}

void Layer::pulse_alpha(int step, int value) {
  if(!fade) {
    blitter.set_blit("0alpha"); /* by placing a '0' in front of the
				   blit name we switch its value to
				   zero before is being showed */
    fade = true; // after the iterator it should deactivate the layer
    // fixme: doesn't works well with concurrent iterators
  }

  blitter.pulse_value(step,value);
}
