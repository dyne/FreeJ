/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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
 */

#ifndef __SOFT_SCREEN_H__
#define __SOFT_SCREEN_H__

#include <screen.h>

#include <factory.h>

class SoftScreen : public ViewPort {

 public:
  SoftScreen();
  ~SoftScreen();

  bool _init(int w, int h);


  fourcc get_pixel_format() { return RGBA32; };

  void *get_surface();

  void setup_blits(Layer *);

  void blit(Layer *src);

  void set_buffer(void *buf);
  void *coords(int x, int y);

  void *buffer;

  uint32_t *pscr, *play;  // generic blit buffer pointers

  // allow to use Factory on this class
  FACTORY_ALLOWED


};
 
#endif
