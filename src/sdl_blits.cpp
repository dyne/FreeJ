/*  FreeJ - blitter layer component
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

#include <config.h>
#include <stdlib.h>

#include <jutils.h>
#include <blitter.h>
#include <memory>
#include "sdl_pixels.h"

///////////////////////////////////////////////////////////////////
// SDL BLITS
// TODO: more SDL blits playing with color masks

namespace {
const char *blit_pixels(void *pixels, SDL_Rect *src, SDL_Surface *dst, SDL_Rect *dest,
                        const Geometry &geometry, bool source_alpha, bool blend, Uint8 alpha) {
  std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
      SDL_CreateSurfaceFrom(geometry.w, geometry.h, freej_pixel_format(source_alpha),
                            pixels, geometry.bytewidth), SDL_DestroySurface);
  if (!surface)
    return "wrap layer pixels";
  if (!SDL_SetSurfaceBlendMode(surface.get(), blend ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE))
    return "set blend mode";
  if (!SDL_SetSurfaceAlphaMod(surface.get(), alpha))
    return "set opacity";
  return SDL_BlitSurface(surface.get(), src, dst, dest) ? nullptr : "blit layer pixels";
}
}

static const char *sdl_rgb(void *src, SDL_Rect *src_rect, SDL_Surface *dst,
                           SDL_Rect *dst_rect, Geometry *geo, Linklist<Parameter> *) {
  return blit_pixels(src, src_rect, dst, dst_rect, *geo, false, false, 255);
}

static const char *sdl_alpha(void *src, SDL_Rect *src_rect, SDL_Surface *dst,
                             SDL_Rect *dst_rect, Geometry *geo, Linklist<Parameter> *params) {
  const auto alpha = static_cast<Uint8>(*static_cast<float *>(params->begin()->value));
  return blit_pixels(src, src_rect, dst, dst_rect, *geo, false, true, alpha);
}

static const char *sdl_srcalpha(void *src, SDL_Rect *src_rect, SDL_Surface *dst,
                                SDL_Rect *dst_rect, Geometry *geo, Linklist<Parameter> *) {
  // Per-pixel alpha takes precedence over the surface alpha in the show API.
  return blit_pixels(src, src_rect, dst, dst_rect, *geo, true, true, 255);
}

void setup_sdl_blits(Blitter *blitter) {
  Parameter *p;
  Blit *b;

  // SDL blits
  b = new Blit(); b->set_name("SDL");
  sprintf(b->desc,"RGB blit (SDL)");
  b->type = Blit::SDL;
  b->sdl_fun = sdl_rgb;
  blitter->blitlist.prepend(b);
  
  blitter->default_blit = b; // SDL is default

  /////////////

  b = new Blit(); b->set_name("ALPHA");
  sprintf(b->desc,"alpha blit (SDL)");
  b->type = Blit::SDL;
  b->sdl_fun = sdl_alpha;
  blitter->blitlist.prepend(b);
  
  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "alpha");
  strcpy(p->description, "level of transparency of alpha channel (0.0 - 1.0)");
  p->multiplier = 255.0;
  b->parameters.append(p);

  /////////////

  b = new Blit(); b->set_name("SRCALPHA");
  sprintf(b->desc,"source alpha blit (SDL)");
  b->type = Blit::SDL;
  b->sdl_fun = sdl_srcalpha;
  blitter->blitlist.prepend(b);

  p = new Parameter(Parameter::NUMBER);
  strcpy(p->name, "alpha");
  strcpy(p->description, "level of transparency of alpha channel (0.0 - 1.0)");
  p->multiplier = 255.0;
  b->parameters.append(p);
  
}
