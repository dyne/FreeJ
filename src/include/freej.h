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

/**
   @file freej.h
   @brief FreeJ public header
*/

#ifndef __FREEJ_H__
#define __FREEJ_H__

#include <inttypes.h>

/**
   This  class is  made to  hold  informations about  the geometry  of
   Layers in FreeJ, describing their format and image bounds.
   
   @brief geometrical specifications of layers
*/
class Geometry {

 public:

  Geometry() { x = y = 0; initialized = false; }
  ~Geometry() { };

  void init(int nw, int nh, int nbpp) {
    w = nw; h = nh; bpp = nbpp;
    pixelsize = w * h;
    bytesize  = w * h * (bpp / 8);
    bytewidth = w * (bpp / 8);
    initialized = true;
  }

  int16_t x; ///< x axis position coordinate
  int16_t y; ///< y axis position coordinate
  uint16_t w; ///< width of frame in pixels
  uint16_t h; ///< height of frame in pixels
  uint8_t bpp; ///< bits per pixel
  uint32_t pixelsize; ///< size of the whole frame in pixels
  uint32_t bytesize; ///< size of the whole frame in bytes
  uint16_t bytewidth; ///< width of frame in bytes (also called pitch or stride)

  bool initialized;
};

#endif
