/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <context.h>

#include <sdl_screen.h>

#include <jutils.h>
#include <config.h>


Context::Context() {

  notice("starting %s %s engine",PACKAGE,VERSION);

  screen = NULL;

  /* initialize fps counter */
  framecount=0;
  gettimeofday( &lst_time, NULL);
  fps=0.0;
  set_fps_interval(24);
  track_fps = false;

  clear_all = false;
  quit = false;
}  

Context::~Context() {

}

bool Context::init(int wx, int hx) {

  screen = new SdlScreen();
  if(!screen->init(wx,hx)) {
    error("fatal error in FreeJ initalization");
    return(false);
  }

  /* save context geometry */
  w = wx; h = hx;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  /* allocate virtual RGBA video buffer */
  uint8_t *vidbuf = (uint8_t*)screen->get_surface();
  if(!vidbuf) {
    error("screen->get_surface returns NULL!");
    return(false);
  }
  /* precalculate y lookup tables */
  for(int c=0;c<h;c++)
    prec_y[c] = (uint8_t*)vidbuf + (pitch*c);

  
  /* refresh the list of available plugins */
  plugger.refresh();

  /* initialize the On Screen Display */
  osd.init(this);
  osd.active();
  set_osd(osd.status_msg); /* let jutils know about the osd */


  return true;
}


void *Context::coords(int x, int y) {
  return( 
	 (Uint8 *)prec_y[y] + 
	 (x<<(bpp>>4)) 
	 );
}


int Context::add_layer(Layer *newlayer) {
  int res = -1;
  
  if(!newlayer) {
    warning("Context::add_layer - invalid NULL layer");
    return(res);
  }
  
  if( newlayer->init(this) ) {
    layers.add(newlayer);
    res = layers.len();
  }
  return(res);
}   

int Context::del_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  act("Context::del_layer : TODO");
  return sel;
}

int Context::clear_layers() {
  int ret = layers.len();
  act("Context::clear_layers : TODO");
  return ret;
}

int Context::moveup_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  if( layers.moveup(sel) )
    show_osd("MOVE UP layer %u -> %u",sel,sel-1);
  return(sel-1);
}

int Context::movedown_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  if ( layers.movedown(sel) )
    show_osd("MOVE DOWN layer %u -> %u",sel,sel+1);
  return(sel+1);
}

int Context::active_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  lay->active = !lay->active;
  show_osd("%s layer %s pos %u",
	   lay->active ? "ACTIVATED" : "DEACTIVATED",
	   lay->getname(), sel);
  return lay->active;
}

void Context::cafudda() {
  quit = false;
  Layer *lay;
  rocknroll(true);

  while(!quit) {

    if(clear_all) screen->clear();
    else osd.clean();
    
    layers.lock();
    lay = (Layer *)layers.end();
    while(lay) {
      lay->cafudda();
      lay = (Layer *)lay->prev;
    }
    layers.unlock();
    
    osd.print();
    
    screen->show();
  
    rocknroll(true);
    
    calc_fps();
  }

  lay = (Layer *)layers.begin();
  while(lay) {
    lay->lock();
    layers.rem(1);
    lay->quit = true;
    lay->signal_feed();
    lay->unlock();
    SDL_Delay(500);
    delete lay;
    lay = (Layer *)layers.begin();
  }
  
  if(screen) delete screen;
  plugger.close();

}

/* FPS */

void Context::set_fps_interval(int interval) {
  fps_frame_interval = interval*1000000;
  min_interval = (long)1000000/interval;
}

void Context::calc_fps() {
  /* 1frame : elapsed = X frames : 1000000 */
  gettimeofday( &cur_time, NULL);
  elapsed = cur_time.tv_usec - lst_time.tv_usec;
  if(cur_time.tv_sec>lst_time.tv_sec) elapsed+=1000000;
  
  if(track_fps) {
    framecount++;
    if(framecount==24) {
      fps=(double)1000000/elapsed;
      framecount=0;
    }
  }

  if(elapsed<=min_interval) {
    usleep( min_interval - elapsed ); /* this is not POSIX, arg */
    lst_time.tv_usec += min_interval;
    if( lst_time.tv_usec > 999999) {
      lst_time.tv_usec -= 1000000;
      lst_time.tv_sec++;
    }
  } else {
    lst_time.tv_usec = cur_time.tv_usec;
    lst_time.tv_sec = cur_time.tv_sec;
  }

}

void Context::rocknroll(bool state) {
  layers.lock();
  Layer *l = (Layer *)layers.begin();
  while(l) {
    if(!l->running) {
      l->start();
      //    l->signal_feed();
      while(!l->running) jsleep(0,500);
      l->active = state;
    }
    l = (Layer *)l->next;
  }
  layers.unlock();
}
