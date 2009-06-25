/*  FreeJ
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

/**
   @file freej.h
   @brief FreeJ public header
*/

#ifndef __FREEJ_H__
#define __FREEJ_H__

#include <inttypes.h>

/**
   This data structure is made to hold informations about the geometry
   of Layers in FreeJ, describing their format and image bounds.
   
   @brief geometrical specifications of layers
*/
typedef struct {
  int16_t x; ///< x axis position coordinate
  int16_t y; ///< y axis position coordinate
  uint16_t w; ///< width of frame in pixels
  uint16_t h; ///< height of frame in pixels
  uint8_t bpp; ///< bits per pixel
  uint16_t pitch; ///< width of frame in bytes
  uint32_t size; ///< size of the whole frame in bytes
} ScreenGeometry;

#endif
