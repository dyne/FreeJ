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

#ifndef __LAYER_H__
#define __LAYER_H__

#include <assert.h>
#include <SDL.h>

#include <filter.h>
#include <jsync.h>

class Context;

class Layer: public Entry, public JSyncThread {

 private:
  int _w, _h, _pitch;
  long _size;

 public:

  Layer() { active = false; };
  virtual ~Layer() { _delete(); };

  void run();
  void _init(Context *screen, int wdt, int hgt, int bpp=0);
  void _delete();
  virtual void _close() { };

  /* these has to be defined into layer instances
     (pure virtual functions) */
  virtual bool feed() = 0; /* feeds in the image source */
  virtual void *get_buffer() = 0; /* returns a pointer to the image source */

  bool add_filter(Filter *newfilt);
  bool del_filter(int sel);
  void clear_filters();
  bool moveup_filter(int sel);
  bool movedown_filter(int sel);
  Filter *active_filter(int sel);
  Filter *listen_filter(int sel);
  virtual bool keypress(SDL_keysym *keysym) { return(false); };

  bool cafudda();

  Linklist filters;

  Context *screen;

  ScreenGeometry geo;

  //bool feeded;
  bool active;
  bool quit;
};

#endif
