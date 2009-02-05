/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 */


/**
   @file context.h FreeJ main engine context environment

   @desc this is the main FreeJ engine, containing the main loop
   cafudda() and referencing the tree of instantiated objects
*/

#ifndef __context_h__
#define __context_h__

#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>


// this header makes freej dependent from SDL
// it is here because of  SDL_PollEvent done in Context
#include <SDL/SDL.h>

#include <linklist.h>
#include <layer.h>
// #include <osd.h>
#include <controller.h>
#include <plugger.h>
#include <screen.h>
#include <shouter.h>
#include <gen_f0r_layer.h>

#include <config.h>

class Console;
class JsParser;
class AudioCollector;
class VideoEncoder;

class FreejDaemon;

/* maximum height & width supported by context */
#define MAX_HEIGHT 1024
#define MAX_WIDTH 768

class Context {
	public:
		enum VideoMode {
			SDL,
			SDLGL,
			GL_HEADLESS,
			HEADLESS
		};
 private:

  /* doublesize calculation */
  uint64_t **doubletab;
  Uint8 *doublebuf;
  int dcy, cy, cx;
  uint64_t eax;
  /* ---------------------- */

  /* timing and other amenities */
  double now;
  bool riciuca;

  // parts of the cafudda process
  void handle_resize();
  void handle_controllers();

  pthread_t cafudda_thread;
  bool running;

 public:

  Context();
  ~Context();

  bool init(int wx, int hx, VideoMode videomode, int audiomode); ///< initialise the engine and screen

  //  void close();
  void cafudda(double secs); ///< run the engine for seconds or one single frame pass

  void start(); ///< start the engine and loop until quit is false
  void start_threaded(); ///< start the engine in a thread, looping until quit is false

  bool register_controller(Controller *ctrl);
  bool rem_controller(Controller *ctrl);

  void add_layer(Layer *lay); ///< add a layer to the screen and engine
  void rem_layer(Layer *lay);

  void add_encoder(VideoEncoder *enc); ///< add an encoder to the engine

  /* this returns the address of selected coords to video memory */
  void *coords(int x, int y) { return screen->coords(x,y); };

  int open_script(char *filename);

  bool config_check(const char *filename);

  void rocknroll();

  void magnify(int algo);
  int magnification;

  void resize(int w, int h);
  bool resizing;
  int resize_w;
  int resize_h;

  bool changeres;

  bool quit;
  
  bool pause;

  bool save_to_file;

  bool interactive;

  ViewPort *screen; ///< Video Screen

  //  Osd osd; ///< On Screen Display

  SDL_Event event;
  bool poll_events;


  Console *console; ///< Console parser (will become a controller)

  Linklist<Controller> controllers; ///< linked list of registered interactive controllers

  Linklist<Layer> layers; ///< linked list of registered layers

  Linklist<Filter> filters; ///< linked list of registered filters

  Linklist<Filter> generators; ///< linked list of registered generators

  Linklist<VideoEncoder> encoders; ///< linked list of registered encoders



  AudioCollector *audio; ///< audio device recording input (PortAudio)

  Plugger plugger; ///< filter plugins host

  JsParser *js; ///< javascript parser object


  /* Set the interval (in frames) after
     the fps counter is updated */
  void set_fps(int fps);
  int fps_speed;

  bool clear_all;
  bool start_running;

#ifdef WITH_FT2
#define MAX_FONTS 1024
  int scanfonts(const char *path, int depth);
  char** font_files;
  int num_fonts;
#endif
  
  char *layers_description; ///< string with a list of available layers compiled in
  Layer *open(char *file); ///< creates a layer from a filename, detecting its type

};

#endif
