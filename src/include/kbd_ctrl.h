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

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define SDL_REPEAT_DELAY	195
#define SDL_REPEAT_INTERVAL	30

#include <SDL/SDL.h>

class Context;
class Layer;
class Filter;
class Plugger;

class KbdListener {
 private:
  bool _layer_op(SDL_keysym *keysym);
  bool _context_op(SDL_keysym *keysym);
  Filter *_filt;
  int _sel, _lastsel;
  int plugin_bank;

  SDL_Event event;
  SDL_keysym *keysym;


 public:
  KbdListener();
  ~KbdListener();
  
  bool init(Context *context);
  void run();
  
  Context *env;
  Layer *layer;
  Filter *filter;
  Plugger *plugger;

  bool active;
};

#endif
