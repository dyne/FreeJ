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

#include <config.h>

#include <stdlib.h>

#include <jutils.h>
#include <soft_screen.h>




SoftScreen::SoftScreen()
  : ViewPort() {

  buffer = NULL;
  bpp = 32;

}

SoftScreen::~SoftScreen() {
  func("%s",__PRETTY_FUNCTION__);

}


bool SoftScreen::init(int w, int h) {

  this->w = w;
  this->h = h;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  // test
  buffer = malloc(size);

  return true;
}

void SoftScreen::set_buffer(void *buf) {
  buffer = buf;
}

void *SoftScreen::coords(int x, int y) {
//   func("method coords(%i,%i) invoked", x, y);
// if you are trying to get a cropped part of the layer
// use the .pitch parameter for a pre-calculated stride
// that is: number of bytes for one full line
  return
    ( x + (w*y) +
      (uint32_t*)get_surface() );
}

void *SoftScreen::get_surface() {
  if(!buffer) {
    error("SOFT screen output is not properly initialised via set_buffer");
    error("this will likely segfault FreeJ");
    return NULL;
  }
  return buffer;
}

