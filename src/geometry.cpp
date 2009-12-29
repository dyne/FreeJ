/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#include <geometry.h>

Geometry::Geometry() {
  x = 0;
  y = 0;
  w = 0;
  h = 0;
  bpp = 0;
  pixelsize = 0;
  bytesize = 0;
  bytewidth = 0;
}

Geometry::~Geometry() {
}

void Geometry::init(int nw, int nh, int nbpp) {
  w = nw; h = nh; bpp = nbpp;
  pixelsize = w * h;
  bytesize  = w * h * (bpp / 8);
  bytewidth = w * (bpp / 8);
}

