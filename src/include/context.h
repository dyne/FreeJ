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
 */

#ifndef __context_h__
#define __context_h__

#include <inttypes.h>
#include <iostream>
#include <stdlib.h>

#include <linklist.h>
#include <layer.h>
#include <osd.h>
#include <kbd_ctrl.h>
//#include <console.h>
#include <plugger.h>
#include <screen.h>

/* maximum height & width supported by context */
#define MAX_HEIGHT 1024
#define MAX_WIDTH 768

class JsParser;
class Console;

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


 public:

  Context();
  ~Context();

  bool init(int wx, int hx);
  void close();
  void cafudda(unsigned int secs);

  /* this returns the address of selected coords to video memory */
  void *coords(int x, int y) { return screen->coords(x,y); };

  void rocknroll();

  void magnify(int algo);
  int magnification;
  bool changeres;

  bool quit;
  
  bool pause;

  /* linked list of registered layers */
  Linklist layers;

  /* Video Screen */
  ViewPort *screen;

  /* On Screen Display */
  Osd osd;

  /* Keyboard controller */
  KbdListener kbd;

  /* Console parser */
  Console *console;

  /* Filter plugins plugger */
  Plugger plugger;

  /* javascript parser object */
  JsParser *js;

  /* Set the interval (in frames) after
     the fps counter is updated */
  void set_fps_interval(int interval);
  float fps;
  bool track_fps;
  
  int fps_speed;

  bool clear_all;
  bool start_running;

};

#endif
