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
  framecount=0; fps=0.0;
  gettimeofday( &lst_time, NULL);
  set_fps_interval(24);
  track_fps = false;

  clear_all = false;
  quit = false;
  pause = false;
}

Context::~Context() {
  notice("cu on http://freej.dyne.org");
}

bool Context::init(int wx, int hx) {
  
  screen = new SdlScreen();
  if(!screen->init(wx,hx)) {
    error("Can't initialize the viewport");
    error("This is a fatal error");
    return(false);
  }
  
  /* refresh the list of available plugins */
  plugger.refresh();
  
  /* initialize the On Screen Display */
  osd.init(this);
  osd.active();
  set_osd(osd.status_msg); /* let jutils know about the osd */
  
  return true;
}

void Context::cafudda() {
  quit = false;
  Layer *lay;
  rocknroll(true);

  while(!quit) {

    if(clear_all) screen->clear();
    else osd.clean();


    lay = (Layer *)layers.end();
    if(!lay) {
      osd._print_credits();
      osd._show_fps();
    } else {
      layers.lock();
      while(lay) {
	  if(!pause) 
	      lay->cafudda();
	  lay = (Layer *)lay->prev;
      }
      layers.unlock();
    }

    osd.print();

    screen->show();
  
    rocknroll(true);
    
    calc_fps();
  }

  delete screen;

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
