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

#include <SDL.h>
#include <iostream.h>
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
  double cur_time;
  double lst_time;
  int fps_frame_interval;

  /* --------------------- */

  void *prec_y[MAX_HEIGHT];

 public:

  Context(int w, int h, int bppx, Uint32 flags);
  ~Context() { close(); };

  void close();
  bool flip();

  /* this returns a pointer to the video memory */
  void *get_surface();

  /* this returns the address of selected coords to video memory */
  void *coords(int x, int y) { return( ((Uint8 *)prec_y[y] + 
					(x<<(bpp>>4)) )); }

  SDL_Surface *surf;
  SDL_VideoInfo *Info;

  int w, h;
  int size;
  int bpp;
  int pitch;
  int id;

  /* linked list of registered layers */
  Linklist layers;
  /* this function is untested */
  bool add_layer(Layer *newlayer);

  /* On Screen Display */
  Osd *osd;

  /* Keyboard Listener */
  KbdListener *kbd;

  /* ---- fps ---- */
  bool calc_fps();
  /* Set the interval (in frames) after which the fps counter is updated */
  void set_fps_interval(int interval);
  void print_fps();
  float fps;
  double trascorso;
  int framecount;

  bool quit;
};

#endif
