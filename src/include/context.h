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
#include <SDL.h>
#include <SDL_syswm.h>
#include <iostream>
#include <stdlib.h>

#include <linklist.h>
#include <layer.h>
#include <osd.h>
#include <keyboard.h>

/* maximum height & width supported by context */
#define MAX_HEIGHT 800
#define MAX_WIDTH 600

class Context {
 private:
  /* fps calculation stuff */
  struct timeval cur_time;
  struct timeval lst_time;
  int fps_frame_interval;
  int framecount;
  long elapsed;
  long min_interval;
  /* --------------------- */

  void *prec_y[MAX_HEIGHT];

  /* doublesize calculation */
  uint64_t **doubletab;
  Uint8 *doublebuf;
  int dcy, cy, cx;
  uint64_t eax;
  /* --- */

 public:

  Context(int w, int h, int bppx, Uint32 flags);
  ~Context() { close(); };

  bool init(int wx, int hx, int bppx, Uint32 flagsx);

  int setres(int w, int h);
  
  void close();
  bool flip();
  void clear();

  /* this returns the address of selected coords to video memory */
  void *coords(int x, int y) { return( ((Uint8 *)prec_y[y] + 
					(x<<(bpp>>4)) )); }
  /* this returns a pointer to the video memory */  
  void *get_surface() { return surface; }; //  { return coords(0,0); }

  bool doublesize(bool val);
  bool dbl;


  void rocknroll(bool state);

  SDL_Surface *surf;
  SDL_SysWMinfo sys;

  int w, h;
  int size;
  int bpp;
  int pitch;
  int id;
  Uint32 flags;

  Uint8* surface;

  /* linked list of registered layers */
  Linklist layers;
  int add_layer(Layer *newlayer);
  int del_layer(int sel);
  int clear_layers();
  int moveup_layer(int sel);
  int movedown_layer(int sel);
  Layer *active_layer(int sel);

  /* On Screen Display */
  Osd *osd;

  Plugger *plugger;

  /* Keyboard Listener */
  KbdListener *kbd;

  /* ---- fps ---- */
  void calc_fps();
  /* Set the interval (in frames) after which the fps counter is updated */
  void set_fps_interval(int interval);
  void print_fps();
  float fps;
  bool track_fps;

  bool clear_all;
  bool quit;
};

#endif
