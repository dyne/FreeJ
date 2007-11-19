/*  FreeJ
 *  (c) Copyright 2001 - 2007 Denis Rojo aka jaromil <jaromil@dyne.org>
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
#include <sdlgl_screen.h>
#include <SDL_imageFilter.h>
#include <jsparser.h>
#include <video_encoder.h>
#include <audio_input.h>
#include <impl_video_encoders.h>
#include <signal.h>
#include <errno.h>

#include <jutils.h>
#include <fastmemcpy.h>
#include <config.h>

void fsigpipe (int Sig);
int got_sigpipe;

Context::Context() {

  screen          = NULL;
  console         = NULL;
  audio           = NULL;

  /* initialize fps counter */
  framecount      = 0; 
  fps             = 0.0;
  track_fps       = true;
  magnification   = 0;
  changeres       = false;
  clear_all       = false;
  start_running   = true;
  quit            = false;
  pause           = false;
  save_to_file    = false;
  interactive     = true;
  
  fps_speed       = 25;


}

Context::~Context() {

  Layer *lay;
  Controller *ctrl;
  VideoEncoder *enc;


#ifdef WITH_FT2
  // free font path array
  int c;
  if(font_files) {
    for(c=0; c<num_fonts; c++)
      free( font_files[c] );
    num_fonts = 0;
    free(font_files);
  }
#endif
  
  
  if (console) {
    console->close ();
    console = NULL;
  }
  
  lay = (Layer *)layers.begin ();
  while (lay) {
    lay-> lock ();
    layers.rem (1);
    lay-> quit = true;
    lay-> signal_feed ();
    lay-> unlock ();
    delete lay;
    lay = (Layer *)layers.begin ();
  }

  enc = (VideoEncoder *)encoders.begin();
  while(enc) {
    enc->lock();
    encoders.rem(1);
    enc->quit = true;
    enc->signal_feed();
    enc->unlock();
    delete enc;
    enc = (VideoEncoder *)encoders.begin();
  }

  ctrl = (Controller *)controllers.begin();
  while(ctrl) {
    //    ctrl->lock();
    controllers.rem(1);
    //    ctrl->quit = true;
    //    ctrl->signal_feed();
    //    ctrl->unlock();
    delete ctrl;
    ctrl = (Controller *)controllers.begin();
  }

  if (screen) {
    delete screen;
    screen = NULL;
  }

  if(audio) {
    delete audio;
    audio = NULL;
  }

  //  plugger.close();

  if(js) {
    delete js;
    js = NULL;
  }

  notice ("cu on http://freej.dyne.org");
}

bool Context::init(int wx, int hx, bool opengl) {

  notice("initializing context environment", wx, hx);

  // If selected use opengl as video output!
#ifdef WITH_OPENGL
  if (opengl)
    screen = new SdlGlScreen();
  else
#endif

    screen = new SdlScreen();
  
  if (! screen->init (wx, hx)) {
    error ("Can't initialize the viewport");
    error ("This is a fatal error");
    return (false);
  }
  
  // create javascript object
  js = new JsParser (this);

  /* create Audio Input
     audio card is opened at audio->init()
  */
  audio = new AudioInput();
  audio->init();
  /* audio->start() is called by rocknroll
     if any objects being used needs audio input
//   audio->start();
  */

#ifdef WITH_FT2
  num_fonts = 0;
  font_files = NULL;

  // parse all font directories  
  scanfonts("/usr/X11R6/lib/X11/fonts/TTF");
  scanfonts("/usr/X11R6/lib/X11/fonts/truetype");
  scanfonts("/usr/X11R6/lib/X11/fonts/TrueType");
  scanfonts("/usr/share/truetype");
  scanfonts("/usr/share/fonts");
  scanfonts("/usr/share/fonts/truetype/freefont");


  if(!num_fonts) {
    error("no truetype fonts found on your system");
    error("you should install .ttf fonts in one of the directories above.");
  } else
    act("Found %i fonts installed",num_fonts);

#endif

  
  // a fast benchmark to select the best memcpy to use
  find_best_memcpy ();
  
  if( SDL_imageFilterMMXdetect () )
    act ("using MMX accelerated blit");
  
  set_fps_interval (fps_speed);
  gettimeofday (&lst_time, NULL);

  // register SIGPIPE signal handler (stream error)
  got_sigpipe = false;
  if (signal (SIGPIPE, fsigpipe) == SIG_ERR) {
    error ("Couldn't install SIGPIPE handler"); 
    //   exit (0); lets not be so drastical...
  }
  
  return true;
}


/*
 * Main loop called fps_speed times a second
 */
void Context::cafudda(double secs) {
  VideoEncoder *enc;
  Layer *lay;
  
  if(secs) /* if secs == 0 will go out after one cycle */
    now = dtime();
  
  do {

    // Change resolution if needed 
    if (changeres) {
      handle_resize();
      changeres = false;
    } /////////////////////////////


    ///////////////////////////////
    // update the console
    if (console && interactive) 
      console->cafudda ();
    ///////////////////////////////


    ///////////////////////////////
    // start layers threads
    rocknroll ();
    ///////////////////////////////


    ///////////////////////////////
    // clear screen if needed
    if (clear_all) screen->clear();
    else if (osd.active) osd.clean();
    ///////////////////////////////


    ///////////////////////////////
    //// process controllers
    handle_controllers();
    ///////////////////////////////


    ///////////////////////////////
    /// process audio input
    audio->cafudda();
    ///////////////////////////////
    
    
    ///////////////////////////////
    /// cafudda all active LAYERS
    lay = (Layer *)layers.end ();
    ///// process each layer in the chain
    if (lay) {
      layers.lock ();
      while (lay) {
	if (!pause)
	  lay->cafudda ();
	lay = (Layer *)lay->prev;
      }
      layers.unlock ();
    }
    /////////// finish processing layers
    //////////////////////////////////////
    

    //////////////////////////////////////
    // cafudda all active ENCODERS
    enc = (VideoEncoder*)encoders.end();
    ///// process each encoder in the chain
    if(enc) {
      encoders.lock();
      while(enc) {
	if(!pause)
	  enc->cafudda();
	enc = (VideoEncoder*)enc->prev;
      }
      encoders.unlock();
    }
    /////////// finish processing encoders
    //////////////////////////////////////

    
    /////////////////////////////
    /// print on screen display 
    if (osd.active && interactive) 
      osd.print ();
    /////////////////////////////



    /////////////////////////////
    // show result on screen 
    screen->show ();
    /////////////////////////////
    

    /////////////////////////////
    // handle timing 
    if(!secs) break; // just one pass

    /////////////////////////////
    // honour quit requests
    if(quit) break; // quit was called

    riciuca = (dtime() - now < secs) ? true : false;
    
    calc_fps ();
    // synced with desired fps here
    //////////////////////////////////


  } while (riciuca);
  
}

void Context::handle_resize() {
  screen->lock ();
  if (magnification) {
    screen->set_magnification (magnification);
    magnification = 0;
  }
  if(resizing) {
    screen->resize (resize_w, resize_h);
    resizing = false;
  }
  osd.resize ();
  screen->unlock();
  
  /* crop all layers to new screen size */
  Layer *lay = (Layer *) layers.begin ();
  while (lay) {
    lay -> lock ();
    lay -> blitter.crop (screen);
    lay -> unlock ();
    lay = (Layer*) lay -> next;
  } 
}

#define SDL_KEYEVENTMASK (SDL_KEYDOWNMASK|SDL_KEYUPMASK)

void Context::handle_controllers() {
  int res;
  Controller *ctrl;

  event.type = SDL_NOEVENT;

  SDL_PumpEvents();

  // peep if there are quit or fullscreen events
  res = SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_KEYEVENTMASK|SDL_QUITMASK);

  // force quit when SDL does
  if (event.type == SDL_QUIT) {
    quit = true;
    return;
  }
  
  // fullscreen switch (ctrl-f)
  if(event.type == SDL_KEYDOWN)
    if(event.key.state == SDL_PRESSED)
      if(event.key.keysym.mod & KMOD_CTRL)
	if(event.key.keysym.sym == SDLK_f) {
	  screen->fullscreen();
	  res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYEVENTMASK|SDL_QUITMASK);  
	}
  
  ctrl = (Controller *)controllers.begin();
  if(ctrl) {
    controllers.lock();
    while(ctrl) {
      if(ctrl->active)
	ctrl->peep(this);
      ctrl = (Controller*)ctrl->next;
    }
    controllers.unlock();
  }

  // flushes all events that are leftover
  while( SDL_PeepEvents(&event,1,SDL_GETEVENT, SDL_ALLEVENTS) > 0 ) continue;
  memset(&event, 0x0, sizeof(SDL_Event));

}


bool Context::register_controller(Controller *ctrl) {
  if(!ctrl) {
    error("Context::register_controller called on a NULL object");
    return false;
  }

  if(! ctrl->initialized ) {
    error("failed registering non initialized controller %s",ctrl->name);
    return false;
  }

  ctrl->active = true;

  controllers.append(ctrl);
  
  act("registered %s controller", ctrl->name);
  return true;
}



void Context::add_layer(Layer *lay) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(lay->list) lay->rem();
  //  lay->geo.fps = fps;

  lay->env = this;
  lay->screen = screen;
  lay->blitter.screen = screen;

  // center the position
  lay->geo.x = (screen->w - lay->geo.w)/2;
  lay->geo.y = (screen->h - lay->geo.h)/2;
  lay->blitter.crop( true );
  layers.append(lay);
  layers.sel(0);
  lay->sel(true);
  func("layer %s succesfully added",lay->name);
}

void Context::add_encoder(VideoEncoder *enc) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__); 
  if(enc->list) enc->rem();
  
  enc->env = this;

  encoders.append(enc);
  encoders.sel(0);
  enc->sel(true);
  func("encoder %s succesfully added", enc->name);
}

int Context::open_script(char *filename) {
  return js->open(filename);
}

bool Context::config_check(const char *filename) {
  char tmp[512];
  
  snprintf(tmp, 512, "%s/.freej/%s", getenv("HOME"), filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/etc/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "%s/%s", DATADIR, filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/usr/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/usr/local/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/opt/video/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  return(false);
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
  fps_speed = interval;
  fps_frame_interval = interval*1000000;
  min_interval = (long)1000000/interval;
}

void Context::calc_fps() {
  struct timespec tmp_rem,*rem;
  rem = &tmp_rem;
  /* 1frame : elapsed = X frames : 1000000 */
  gettimeofday (&cur_time, NULL);
  elapsed = cur_time.tv_usec - lst_time.tv_usec;
  if (cur_time.tv_sec > lst_time.tv_sec)  // ?? should be always true? kysu
    // nope. see sys/time.h timersub macro -jrml
    elapsed += 1000000;
  
  if (track_fps) {
    framecount++;
    if (framecount==24) {
      // this is the only division
      fps = (double)1000000 / elapsed;
      framecount = 0;
    }
  }
  
  if (elapsed <= min_interval) { // If time elapsed if < 1/frame_for_second wait
    
    // the following calculus is approximated to bitwise multiplication
    // this wont really hurt our precision, anyway we care more about speed
    // hey are we sure we don't care about precision? was there some problem? -jrml
    jsleep (0, (min_interval - elapsed) << 10);
    
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
  VideoEncoder *e;
  Layer *l;
  bool need_audio = false;

  ///////////////////////////////////////////
  // Layers - start the threads to be started
  l = (Layer *)layers.begin();
  
  // Show credits when no layers are present and interactive
  if (!l) // there are no layers
    if ( interactive ) { // engine running in interactive mode
      osd.credits ( true);
      return;
    }
  
  // Iterate throught linked list of layers and start them
  layers.lock();
  while (l) {
    if (!l->running) {
      if (l->start() == 0) {
	while (!l->running) {
	  jsleep(0,500);
	  func("waiting %s layer thread to start", l->name);
	}
	l->active = start_running;
      }
      else { // problems starting thread
	func ("Context::rocknroll() : error creating thread");
      }
    }

    need_audio |= (l->running && l->use_audio);
    
    l = (Layer *)l->next;
  }
  layers.unlock();
  /////////////////////////////////////////////////////////////
  ///////////////////////////////////// end of layers rocknroll



  /////////////////////////////////////////////////////
  // Video Encoders - start the encoders to be launched
  e = (VideoEncoder*) encoders.begin();

  encoders.lock();
  while(e) {
    if(!e->running) {
      func("launching thread for %s",e->name);
      if( e->start() == 0 ) {
	act("waiting for %s thread to start...", e->name);
	while(! e->running ) jsleep(0,500);
	act("%s thread started", e->name);
	e->active = start_running;
      }
      else { // problems starting thread
	error("can't launch thread for %s", e->name);
      }
    }

    need_audio |= (e->running && e->use_audio);

    e = (VideoEncoder*)e->next;
  }
  encoders.unlock();
  //////////////////////////////////////////////////////////////
  //////////////////////////////////// end of encoders rocknroll

  // start audio capture if needed by layer/encoders
  if(need_audio) {
    //    act("starting audio capture");
    audio->start();
  } else {
    //    act("stopping audio capture");
    audio->stop();
  }

}

void fsigpipe (int Sig) {
  if (!got_sigpipe)
    warning ("SIGPIPE - Problems streaming video :-(");
  got_sigpipe = true;
}
