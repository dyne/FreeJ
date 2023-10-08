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

#ifndef __SDL_SCREEN_H__
#define __SDL_SCREEN_H__

#include <SDL2/SDL.h>
#include <screen.h>

#include <factory.h>

class SdlScreen : public ViewPort {
 public:
  SdlScreen();
  ~SdlScreen();


  void resize(int resize_w, int resize_h);
  void setup_blits(Layer *lay);
  void blit(Layer *src);


  void show();
  void clear();

  void fullscreen();
  void *get_surface();
  fourcc get_pixel_format() { return BGRA32; };

  SDL_Event event;
  SDL_Surface *sdl_screen;

  void *coords(int x, int y);

  bool lock();
  bool unlock();
 

 private:

  bool _init();

  int setres(int wx, int hx);
  SDL_Surface *emuscr;

  bool switch_fullscreen;  
  bool dbl;
  uint32_t sdl_flags;

  SDL_Surface *sdl_dest;

  SDL_Surface *pre_rotozoom;
  SDL_Surface *rotozoom; ///< pointer to blittable surface (rotated and zoomed if necessary)

  // small vars used in blits
  int chan, c, cc;
  uint32_t *scr, *off, *poff, *pastoff, *ppastoff;
  uint32_t *pscr, *play, *ppast;  // generic blit buffer pointers

  // allow to use Factory on this class
  FACTORY_ALLOWED;

};

#endif 
