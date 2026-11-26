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

  screen_buffer = calloc(1, geo.bytesize);
  return screen_buffer != nullptr;
}

void SoftScreen::blit(Layer *src) {
  if(src->screen != this) {
    error("%s: blit called on a layer not belonging to screen",
	  __PRETTY_FUNCTION__);
    return;
  }

  auto *b = src->current_blit;
  if (!b || !b->fun)
    return;
  if (src->need_crop)
    src->blitter->crop(src, this);
  if (!b->lay_height)
    return;

  auto *out = static_cast<uint32_t *>(get_surface()) + b->scr_offset;
  auto *in = static_cast<uint32_t *>(src->buffer) + b->lay_offset;
  for (int row = 0; row < b->lay_height; ++row)
    b->fun(in + row * b->lay_stride, out + row * b->scr_stride,
           b->lay_bytepitch, &b->parameters);

}

void SoftScreen::set_buffer(void *buf) {
  if(screen_buffer) free(screen_buffer);
  screen_buffer = buf;
}

void *SoftScreen::coords(int x, int y) {
  return static_cast<uint32_t *>(get_surface()) + y * geo.w + x;
}

void *SoftScreen::get_surface() {
  if(!screen_buffer) {
    error("SOFT screen output is not properly initialised via set_buffer");
    error("this will likely segfault FreeJ");
    return NULL;
  }
  return screen_buffer;
}

