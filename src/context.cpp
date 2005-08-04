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
#include <sdlgl_screen.h>
#include <imagefilter.h>
#include <jsparser.h>
#include <video_encoder.h>
#include <impl_video_encoders.h>
#include <pipe.h>
#include <shouter.h>
#include <signal.h>
#include <errno.h>

#include <jutils.h>
#include <fastmemcpy.h>
#include <config.h>

void fsigpipe (int Sig);
int got_sigpipe;

Context::Context() {

	notice("starting %s %s engine",PACKAGE,VERSION);

	screen          = NULL;
	console         = NULL;

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

	fps_speed=25;

}

Context::~Context() {
	close ();
	notice ("cu on http://freej.dyne.org");
}

bool Context::init(int wx, int hx, bool opengl) {

	/*
	 * If selected use opengl as video output!
	 */
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

#ifdef WITH_JAVASCRIPT
	// create javascript object
	js = new JsParser (this);
#endif

	// create object here to avoid performance issues at run time
#ifdef WITH_AVCODEC
	video_encoder = get_encoder ("freej.ogg");

	// register SIGPIPE signal handler (stream error)
	got_sigpipe = false;
	if (signal (SIGPIPE, fsigpipe) == SIG_ERR) {
		error ("Couldn't install SIGPIPE handler"); 
		exit (0);
	}

	// create shouter object to stream to an icecast server
	shouter = new Shouter ();
#endif

	// a fast benchmark to select the best memcpy to use
	find_best_memcpy ();

	if( SDL_imageFilterMMXdetect () )
		act ("using MMX accelerated blit");

	set_fps_interval (fps_speed);
	gettimeofday (&lst_time, NULL);

	return true;
}

void Context::close() {
	Layer *lay;

#ifdef WITH_AVCODEC
	delete video_encoder;
#endif

	if (console)
		console->close ();

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

	if (screen) 
		delete screen;

	plugger.close ();

#ifdef WITH_JAVASCRIPT
	delete js;
#endif

}

/*
 * Main loop called fps_speed times a second
 */
void Context::cafudda(double secs) {
	Layer *lay;

	if(secs) /* if secs == 0 will go out after one cycle */
		now = dtime();

	do {
		/** Fetch keyboard events */
		if (interactive) 
			kbd.run ();

		/* 
		 * Change resolution if needed 
		 */
		if (changeres) {
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
			changeres = false;
		}

		if (console && interactive) 
			console->cafudda ();

		/** start layers thread */
		rocknroll ();

		// clear screen before each iteration
		if (clear_all)
			screen->clear();
		else if (osd.active)
			osd.clean();

		/** process each layer in the chain */
		lay = (Layer *)layers.end ();
		if (lay) {
			layers.lock ();
			while (lay) {
				if (!pause)
					lay->cafudda ();
				lay = (Layer *)lay->prev;
			}
			layers.unlock ();
		}


#ifdef WITH_AVCODEC
		/*
		 if (save_to_file)
		 video_encoder -> write_frame();

		 if (stream)
		 video_encoder -> stream_it(true);
		 
		 



		 */
		// show results on file if requested encoder in a thread ?? not now. kysu.
		//	    if(! video_encoder->isStarted())
		//		    video_encoder->start();
		//	    video_encoder->has_finished_frame();
		//	    video_encoder->signal();
		//
		if(save_to_file) {
			//	    for ( int i = 0; i < 10 ;i ++) {
			if (video_encoder -> is_stream() && !shouter -> start())
				video_encoder -> stream_it (false);
			//			    break;
			//		    else if (i == 9)
			//			    save_to_file = false;
			//	    }

			if (save_to_file) {
				if (! (video_encoder->init (this, screen) )) {
					error ("Can't save to file. retry!");
					save_to_file = false;
				}
				else {
					if (!video_encoder -> is_audio_inited ()) {
					    notice ("Starting audio system");
					    video_encoder -> start_audio_stream ();
					}
					video_encoder -> write_frame ();
				}
			}
		}
#endif

		/* 
		 * print on screen display 
		 */
		if (osd.active && interactive) 
			osd.print ();

		/** 
		 * show result on screen 
		 */
		screen->show ();


		/* 
		 * Handle timing 
		 */
		if(!secs) 
			break; /* just one pass */

		riciuca = (dtime() - now < secs) ? true : false;

		calc_fps ();

	} while (riciuca);

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
	rem = &tmp_rem;
	/* 1frame : elapsed = X frames : 1000000 */
	gettimeofday (&cur_time, NULL);
	elapsed = cur_time.tv_usec - lst_time.tv_usec;
	if (cur_time.tv_sec > lst_time.tv_sec)  // ?? should be always true? kysu
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
	Layer *l = (Layer *)layers.begin();

	/*
	 * Show credits when user doesn't specified layers
	 */
	if (!l) // there are no layers
		if ( interactive) { // engine running in interactive mode
			osd.credits ( true);
			return;
		}

	/*
	 * Iterate throught linked list of layers and start them
	 */
	layers.lock();
	while (l) {
		if (!l->running) {
			if (l->start() == 0) {
				while (!l->running) 
					jsleep(0,500);
				l->active = start_running;
			}
			else { // problems starting thread
				func ("Context::rocknroll() : error creating thread");
			}
		}
		l = (Layer *)l->next;
	}
	layers.unlock();

}

    void fsigpipe (int Sig) {
	if (!got_sigpipe)
	    warning ("Problems streaming video :-(");
	got_sigpipe = true;
    }
