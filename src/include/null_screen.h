/*  FreeJ
 *  (c) Copyright 2008 Denis Roio <jaromil@dyne.org>
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

#include <config.h>

#ifndef __NULL_SCREEN_H__
#define __NULL_SCREEN_H__


#include <screen.h>


class NullScreen : public ViewPort {
 public:
  NullScreen();
  ~NullScreen();

  bool init(int width, int height);
  void set_magnification(int algo);
  void resize(int resize_w, int resize_h);
  void show();
  void clear();
  void *get_surface();
  fourcc get_pixel_format() { return RGBA32 };
  void fullscreen();
  bool lock();
  bool unlock();

  void *coords(int x, int y);

};

#endif

