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
#include <iostream>
#include <stdlib.h>


// this header makes freej dependent from SDL
// it is here because of  SDL_PollEvent done in Context
#include <SDL/SDL.h>

#include <linklist.h>
#include <layer.h>
#include <osd.h>
#include <controller.h>
#include <plugger.h>
#include <screen.h>
#include <shouter.h>

#include <config.h>

/* maximum height & width supported by context */
#define MAX_HEIGHT 1024
#define MAX_WIDTH 768

class Console;
class JsParser;
class AudioInput;
class VideoEncoder;

class Context {
 private:
  /* ---- fps ---- */
  struct timeval cur_time;
  struct timeval lst_time;
  struct timespec slp_time;
  int fps_frame_interval;
  int framecount;
  long elapsed;
  long min_interval;
  void calc_fps();
  /* ------------- */

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


 public:

  Context();
  ~Context();

  bool init(int wx, int hx, bool opengl);
  //  void close();
  void cafudda(double secs);

  bool register_controller(Controller *ctrl);

  void add_layer(Layer *lay); ///< add a layer to the screen and engine

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

  Osd osd; ///< On Screen Display



  SDL_Event event;

  Console *console; ///< Console parser (will become a controller)

  Linklist controllers; ///< linked list of registered interactive controllers

  Linklist layers; ///< linked list of registered layers

  Linklist encoders; ///< linked list of registered encoders



  AudioInput *audio; ///< audio device recording input (PortAudio)

  Plugger plugger; ///< filter plugins host

  JsParser *js; ///< javascript parser object



  /* Set the interval (in frames) after
     the fps counter is updated */
  void set_fps_interval(int interval);

  float fps;
  bool track_fps;
  
  int fps_speed;

  bool clear_all;
  bool start_running;

#ifdef WITH_FT2
#define MAX_FONTS 1024
  int scanfonts(char *path);
  char** font_files;
  int num_fonts;
#endif

};

#endif
