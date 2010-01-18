/*  FreeJ
 *  (c) Copyright 2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __FREEJ_COLOR_H__
#define __FREEJ_COLOR_H__

#include <config.h>

#include <inttypes.h>


class Color {

 public:

  Color();
  virtual ~Color();

  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;

  void set_rgb(int nr, int ng, int nb);
  void set_rgba(int nr, int ng, int nb, int na);

  void set_gray(double g);
  void set_gray_alpha(double g, int a);

  /* void set_hsb(double h, double s, double b); */

  /* void set_hex(uint32_t h); */
  /* void set_hex_alpha(uint32_t h, uint8_t a); */

  virtual void set() =0;

};

#endif
  
