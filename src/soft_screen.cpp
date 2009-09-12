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

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, SoftScreen, Screen, soft);




SoftScreen::SoftScreen()
  : ViewPort() {

  screen_buffer = NULL;
  set_name("SOFT");
}

SoftScreen::~SoftScreen() {
  func("%s",__PRETTY_FUNCTION__);
  if(screen_buffer) free(screen_buffer);
}

void SoftScreen::setup_blits(Layer *lay) {

  Blitter *b = new Blitter();

  setup_linear_blits(b);

  lay->blitter = b;

  lay->set_blit("RGB"); // default

}

bool SoftScreen::_init() {

  // test
  screen_buffer = malloc(geo.bytesize);
  func("SoftScreen buffer initialized at %p (%u bytes)",
       screen_buffer, geo.bytesize);
  return(true);
}

void SoftScreen::blit(Layer *src) {
  int16_t c;
  Blit *b;
  void *offset;
  
  if(src->screen != this) {
    error("%s: blit called on a layer not belonging to screen",
	  __PRETTY_FUNCTION__);
    return;
  }

  if(src->need_crop)
    src->blitter->crop( src, this );

  b = src->current_blit;

  pscr = (uint32_t*) get_surface() + b->scr_offset;
  play = (uint32_t*) src->buffer   + b->lay_offset;

  // iterates the blit on each horizontal line
  for( c = b->lay_height ; c > 0 ; c-- ) {

    (*b->fun)
      ((void*)play, (void*)pscr,
       b->lay_bytepitch,// * src->geo.bpp>>3,
       &b->parameters);

    // strides down to the next line
    pscr += b->scr_stride + b->lay_pitch;
    play += b->lay_stride + b->lay_pitch;
    
  }

}

void SoftScreen::set_buffer(void *buf) {
  if(screen_buffer) free(screen_buffer);
  screen_buffer = buf;
}

void *SoftScreen::coords(int x, int y) {
//   func("method coords(%i,%i) invoked", x, y);
// if you are trying to get a cropped part of the layer
// use the .pixelsize geometric property for a pre-calculated stride
// that is: number of bytes for one full line
  return
    ( x + geo.pixelsize +
      (uint32_t*)get_surface() );
}

void *SoftScreen::get_surface() {
  if(!screen_buffer) {
    error("SOFT screen output is not properly initialised via set_buffer");
    error("this will likely segfault FreeJ");
    return NULL;
  }
  return screen_buffer;
}

