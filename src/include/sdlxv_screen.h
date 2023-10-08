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
 *
 * "$Id$"
 *
 */

#ifndef __SDLXV_SCREEN_H__
#define __SDLXV_SCREEN_H__

#include <SDL2/SDL.h>
#include <screen.h>

class SdlXvScreen : public ViewPort {
 public:

  SdlXvScreen();
  ~SdlXvScreen();

  bool init(int width, int height);
  void blit(Layer *layer);
  void crop(Layer *layer);
  void show();
  void clear();
  void fullscreen() {};
  void *get_surface();
  SDL_Surface *scr;

  // deprecated
  void *coords(int x, int y);

 private:

  bool sdl_lock();
  bool sdl_unlock();
  bool yuv_lock();
  bool yuv_unlock();

  SDL_Surface *blitter;
  //  uint32_t rmask,gmask,bmask,amask;
  SDL_Rect rect;
  SDL_Overlay *yuv_overlay;

  
  void *anal;

};


#endif
