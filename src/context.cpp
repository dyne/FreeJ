/*  FreeJ
 *  (c) Copyright 2001 - 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
#include <imagefilter.h>
#include <jsparser.h>

#include <jutils.h>
#include <fastmemcpy.h>
#include <config.h>


Context::Context() {

  notice("starting %s %s engine",PACKAGE,VERSION);

  screen = NULL;
  console = NULL;

  /* initialize fps counter */
  framecount=0; fps=0.0;
  track_fps = true;
  magnification = 0;
  changeres = false;
  clear_all = false;
  start_running = true;
  quit = false;
  pause = false;
  interactive = true;

  fps_speed=25;

}

Context::~Context() {
  close();
  notice("cu on http://freej.dyne.org");
}

bool Context::init(int wx, int hx) {

  screen = new SdlScreen();
  if(!screen->init(wx,hx)) {
    error("Can't initialize the viewport");
    error("This is a fatal error");
    return(false);
  }
  
#ifdef WITH_JAVASCRIPT
  js = new JsParser(this);
#endif
  
  find_best_memcpy();
  
  if( SDL_imageFilterMMXdetect() )
    act("using MMX accelerated blit");

  set_fps_interval(fps_speed);
  gettimeofday( &lst_time, NULL);

  return true;
}

void Context::close() {
  Layer *lay;

  if(console)
    console->close();

  lay = (Layer *)layers.begin();
  while(lay) {
    lay->lock();
    layers.rem(1);
    lay->quit = true;
    lay->signal_feed();
    lay->unlock();
//    SDL_Delay(500);
    delete lay;
    lay = (Layer *)layers.begin();
  }

  if(screen) delete screen;

  plugger.close();

#ifdef WITH_JAVASCRIPT
  delete js;
#endif

}

void Context::cafudda(double secs) {
  Layer *lay;

  if(secs) /* if secs =0 will go out after one cycle */
    now = dtime();

  do {

    /** fetch keyboard events */
    if(interactive) kbd.run();

    /* change resolution if needed */
    if(changeres) {
      screen->lock();
      if(magnification) {
	screen->set_magnification(magnification);
	magnification = 0;
      }
      if(resizing) {
	screen->resize(resize_w, resize_h);
	resizing = false;
      }
      osd.resize();
      screen->unlock();
      /* crop all layers to new screen size */
      Layer *lay = (Layer *)layers.begin();
      while(lay) {
	lay->lock();
	lay->blitter.crop( screen );
	lay->unlock();
	lay = (Layer*)lay->next;
      }
      changeres = false;
    }

    if(console && interactive) console->cafudda();

    /** start layers thread */
    rocknroll();


    // clear screen before each iteration
    if(clear_all)
      screen->clear();
    else if(osd.active)
      osd.clean();

    /** process each layer in the chain */
    lay = (Layer *)layers.end();
    if(lay) {
      layers.lock();
      while(lay) {
	if(!pause)
	  lay->cafudda();
	lay = (Layer *)lay->prev;
      }
      layers.unlock();
    }

    /** print on screen display */
    if(osd.active && interactive) osd.print();


    /** show result on screen */
    screen->show();



    /******* handle timing */

    if(!secs) break; /* just one pass */

    riciuca = (dtime()-now<secs) ? true : false;

    calc_fps();

  } while(riciuca);

}


void Context::resize(int w, int h) {
  resize_w = w;
  resize_h = h;
  resizing = true;
  changeres = true;
}

void Context::magnify(int algo) {
  if(magnification == algo) return;
  magnification = algo;
  changeres = true;
}

/* FPS */

void Context::set_fps_interval(int interval) {
  fps_frame_interval = interval*1000000;
  min_interval = (long)1000000/interval;
}

void Context::calc_fps() {
  struct timespec tmp_rem,*rem;
  rem=&tmp_rem;
  /* 1frame : elapsed = X frames : 1000000 */
  gettimeofday( &cur_time, NULL);
  elapsed = cur_time.tv_usec - lst_time.tv_usec;
  if(cur_time.tv_sec>lst_time.tv_sec) elapsed+=1000000;

  if(track_fps) {
    framecount++;
    if(framecount==24) {
      // this is the only division
      fps=(double)1000000/elapsed;
      framecount=0;
    }
  }

  if(elapsed<=min_interval) {

    slp_time.tv_sec = 0;
    // the following calculus is approximated to bitwise multiplication
    // this wont really hurt our precision, anyway we care more about speed
    slp_time.tv_nsec = (min_interval - elapsed)<<10;

    // handle signals (see man 2 nanosleep)
    while(nanosleep(&slp_time,rem)==-1 && (errno==EINTR));

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

void Context::rocknroll() {

  Layer *l = (Layer *)layers.begin();

  if(!l) // there are no layers
    if(interactive) { // engine running in interactive mode
      osd.credits(true);
      return;
    }

  layers.lock();
  while(l) {
    if(!l->running) {
      if(l->start()==0) {
	//    l->signal_feed(); QUAAA
	while(!l->running) jsleep(0,500);
	l->active = start_running;
      }
      else 
	func("Context::rocknroll() : error creating thread");
    }
    l = (Layer *)l->next;
  }
  layers.unlock();
  
}
