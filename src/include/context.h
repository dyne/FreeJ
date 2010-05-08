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

//#include <linklist.h>
#include <layer.h>
// #include <osd.h>
//#include <controller.h>
#include <plugger.h>
#include <screen.h>
#include <shouter.h>

#include <factory.h>
#include <config.h>
#include <string>

class Controller;

class JsParser;
class AudioCollector;
class VideoEncoder;


class FreejDaemon;

template <class T> class Linklist;
    
/* maximum height & width supported by context */
#define MAX_HEIGHT 1024
#define MAX_WIDTH 768

class Context {
 private:
  static bool factory_initialized;
  /* doublesize calculation */
  uint64_t **doubletab;
  Uint8 *doublebuf;
  int dcy, cy, cx;
  uint64_t eax;
  /* ---------------------- */

  // parts of the cafudda process
  void handle_resize();
  void handle_controllers();

  pthread_t cafudda_thread;
  bool running;

  // Factories 
  //static Factory<Layer> layer_factory; // Layer Factory
  // Default layer types
  /*
  inline void default_layertypes()
  {
    default_layertypes_map.insert(FIdPair("GeometryLayer", "basic"));
  }

  //static Factory<Controller> controller_factory; // Controller Factory
  // Default controller types
  inline void default_controllertypes()
  {
    default_controllertypes_map.insert(FIdPair("KeyboardController", "sdl"));
  }
  */
    
 public:

  Context();
  ~Context();

  bool init(); ///< initialise the engine

  //  void close();
  void cafudda(double secs); ///< run the engine for seconds or one single frame pass

  void start(); ///< start the engine and loop until quit is false
  void start_threaded(); ///< start the engine in a thread, looping until quit is false

  bool register_controller(Controller *ctrl);
  bool rem_controller(Controller *ctrl);

  bool add_layer(Layer *lay); ///< add a layer to the screen and engine
  void rem_layer(Layer *lay);

  bool add_encoder(VideoEncoder *enc); ///< add an encoder to the engine

  void *coords(int x, int y); ///< returns an offset to currently selected screen

  int parse_js_cmd(const char *cmd);

  int open_script(char *filename);

  int reset(); ///< clear the engine and deletes all registered objects

  bool config_check(const char *filename);

  void resize(int w, int h);

  bool quit;
  
  bool pause;

  bool save_to_file;

  bool interactive;

  //  Osd osd; ///< On Screen Display

  SDL_Event event;
  bool poll_events;

  bool add_screen(ViewPort *scr); ///< add a new screen
  Linklist<ViewPort> screens; ///< linked list of registered screens
  ViewPort *screen; ///< pointer to the first screen on top of the list (auxiliary)

  Linklist<Controller> controllers; ///< linked list of registered interactive controllers

  Linklist<Filter> filters; ///< linked list of registered filters

  Linklist<Filter> generators; ///< linked list of registered generators

  //AudioCollector *audio; ///< audio device recording input (PortAudio)

  Plugger plugger; ///< filter plugins host

  JsParser *js; ///< javascript parser object

  char main_javascript[512]; ///< if started with a javascript, save the filename here (used by reset)

  /* Set the interval (in frames) after
     the fps counter is updated */
  FPS fps;
  int fps_speed;

  bool clear_all;
  bool start_running;

  char *layers_description; ///< string describing available layer types
  char *screens_description; ///< string describing available screen types

  Layer *open(char *file, int w = 0, int h = 0); ///< creates a layer from a filename, detecting its type
 
};

#endif
