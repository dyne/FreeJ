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
 *
 */

#include <config.h>

#include <color.h>

Color::Color() {
  // defaults to black
  r = 0;
  g = 0;
  b = 0;
  a = 0;
  //  set();
}

Color::~Color() { }


void Color::set_rgb(double nr, double ng, double nb) {
  r = nr;
  g = ng;
  b = nb;
  set();
}

void Color::set_rgba(double nr, double ng, double nb, double na) {
  r = nr;
  g = ng;
  b = nb;
  a = na;
  set();
}

void Color::set_gray(double g) { set_rgb(g, g, g); }
void Color::set_gray_alpha(double g, double a) { set_rgba(g, g, g, a); }
