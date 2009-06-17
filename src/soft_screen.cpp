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

#include <layer.h>
#include <blitter.h>

#include <jutils.h>
#include <soft_screen.h>




SoftScreen::SoftScreen(int w, int h)
  : ViewPort(w, h) {

  buffer = NULL;
  bpp = 32;

  init(w, h);
  //  set_name("SOFT");
}

SoftScreen::~SoftScreen() {
  func("%s",__PRETTY_FUNCTION__);

}


bool SoftScreen::init(int w, int h) {

  // test
  buffer = malloc(size);

  return(true);
}

void SoftScreen::blit(Layer *src) {
  register int16_t c;
  Blit *b;
  void *offset;
  
  if(src->screen != this) {
    error("%s: blit called on a layer not belonging to screen",__PRETTY_FUNCTION__);
    return;
  }

  if(src->need_crop)
    src->blitter->crop( src, this );

  b = src->current_blit;

  pscr = (uint32_t*) get_surface() + b->scr_offset;
  play = (uint32_t*) offset        + b->lay_offset;
  
  // iterates the blit on each horizontal line
  for( c = b->lay_height ; c > 0 ; c-- ) {
    
    (*b->fun)
      ((void*)play, (void*)pscr,
       b->lay_bytepitch,// * src->geo.bpp>>3,
       (void*)&b->value);
    
    // strides down to the next line
    pscr += b->scr_stride + b->lay_pitch;
    play += b->lay_stride + b->lay_pitch;
    
  }
  
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

