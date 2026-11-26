/*  FreeJ - blitter layer component
 *
 *  (c) Copyright 2004-2009 Denis Roio aka jaromil <jaromil@dyne.org>
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


#include <algorithm>

#include <layer.h>
#include <blitter.h>
#include <context.h>
#include <iterator.h>
#include <linklist.h>


#include <sdl_screen.h>

#include <jutils.h>
#include <config.h>





Blit::Blit() :Entry() {
  sprintf(desc,"none");

  value = 0.0;

  fun = NULL;
  sdl_fun = nullptr;
  type = NONE;
  past_frame = NULL;

}

Blit::~Blit() {
  // clean up all parameters
  Parameter *par;
  par = parameters.begin();
  while(par) {
    par->rem();
    delete par;
    par = parameters.begin();
  }
}

Blitter::Blitter() : screen(nullptr), default_blit(nullptr), geo(nullptr) {}

Blitter::~Blitter() {

  Blit *b = blitlist.begin();
  while(b) {
    b->rem();
    delete(b);
    b = blitlist.begin();
  }

}



void Blitter::crop(Layer *layer, ViewPort *screen, Geometry *transformed)
{
  auto &blit = *layer->current_blit;
  geo = transformed ? transformed : &layer->geo;
  const int x = layer->geo.x - (int(geo->w) - layer->geo.w) / 2;
  const int y = layer->geo.y - (int(geo->h) - layer->geo.h) / 2;
  const int left = std::max(0, x);
  const int top = std::max(0, y);
  const int width = std::max(0, std::min(int(screen->geo.w), x + geo->w) - left);
  const int height = std::max(0, std::min(int(screen->geo.h), y + geo->h) - top);
  layer->need_crop = false;
  layer->hidden = width == 0 || height == 0;
  blit.lay_height = layer->hidden ? 0 : height;
  if (layer->hidden)
    return;

  const int src_x = left - x;
  const int src_y = top - y;
  if (blit.type == Blit::SDL) {
    blit.sdl_rect = {src_x, src_y, width, height};
    blit.sdl_destination = {left, top, width, height};
  } else {
    const int source_stride = geo->bytewidth / sizeof(uint32_t);
    blit.lay_bytepitch = width * sizeof(uint32_t);
    blit.lay_stride = source_stride;
    blit.scr_stride = screen->geo.w;
    blit.lay_offset = src_y * source_stride + src_x;
    blit.scr_offset = top * screen->geo.w + left;
  }
}
