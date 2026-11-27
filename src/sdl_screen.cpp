/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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
 *
 * "$Id$"
 *
 */

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <memory>

#include <layer.h>
#include <blitter.h>
#include <linklist.h>
#include <sdl_screen.h>

#include <SDL_imageFilter.h>
#include <SDL_rotozoom.h>

#include <jutils.h>
#include "sdl_pixels.h"
#include "engine/frame_storage.h"

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, SdlScreen, Screen, sdl);

SdlScreen::SdlScreen()
  : ViewPort(), sdl_screen(nullptr), window(nullptr), switch_fullscreen(false) {
  set_name("SDL");
}

SdlScreen::~SdlScreen() {
  SDL_DestroySurface(sdl_screen);
  if (window) {
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
  }
}

bool SdlScreen::_init() {
  SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "0");
  if (!SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK)) {
    error("Can't initialize SDL viewport '%s' (%ux%u): %s", name, geo.w, geo.h, SDL_GetError());
    return false;
  }

  sdl_screen = SDL_CreateSurface(geo.w, geo.h, freej_pixel_format());
  if (sdl_screen)
    window = SDL_CreateWindow(PACKAGE " " VERSION, geo.w, geo.h, 0);
  if (!window) {
    error("Can't create SDL viewport '%s' (%ux%u): %s", name, geo.w, geo.h, SDL_GetError());
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    return false;
  }

  SDL_SetSurfaceBlendMode(sdl_screen, SDL_BLENDMODE_NONE);
  notice("SDL Viewport is %s %ix%i %ibpp", SDL_GetCurrentVideoDriver(),
         geo.w, geo.h, geo.bpp);
  if (SDL_imageFilterMMXdetect())
    act("SDL using MMX accelerated blit");
  return true;
}

void SdlScreen::setup_blits(Layer *lay) {

  Blitter *b = new Blitter();

  setup_linear_blits(b);

  setup_sdl_blits(b);

  lay->blitter = b;

  lay->set_blit("SDL"); // default
}

void SdlScreen::blit(Layer *layer) {
  auto *blit = layer->current_blit;
  if (!blit)
    return;

  void *pixels = layer->buffer;
  Geometry *transformed = nullptr;
  auto &cache = layer->blitter->transform_;
  auto &drawing = layer->blitter->drawing_failure_;
  const auto report = [&](const char *operation) {
    if (drawing.first()) {
      const char *detail = SDL_GetError();
      error("Layer '%s' (%ux%u, %s) on viewport '%s' (%ux%u): %s failed: %s",
            layer->name, layer->geo.w, layer->geo.h, blit->name, name, geo.w, geo.h,
            operation, *detail ? detail : "SDL provided no detail");
    }
  };
  if (layer->rotating || layer->zooming) {
    const auto frame = freej_engine::FrameAccess::layer(*layer);
    const auto generation = frame && frame->pixels.data() == pixels ? frame->generation : 0;
    const auto angle = layer->rotating ? layer->rotate : 0;
    const auto zoom_x = layer->zoom_x;
    const auto zoom_y = layer->rotating ? zoom_x : layer->zoom_y;
    if (!cache.surface || !generation || cache.generation != generation ||
        cache.angle != angle || cache.zoom_x != zoom_x || cache.zoom_y != zoom_y ||
        cache.antialias != layer->antialias) {
      // A new transform can be as large as the viewport. Release the
      // previous pixels before allocating the replacement.
      cache.surface.reset();
      std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> source(
          SDL_CreateSurfaceFrom(layer->geo.w, layer->geo.h, freej_pixel_format(),
                                pixels, layer->geo.bytewidth), SDL_DestroySurface);
      if (!source) {
        report("wrap transform source");
        return;
      }
      SDL_ClearError();
      cache.surface.reset(layer->rotating
          ? rotozoomSurface(source.get(), angle, zoom_x, layer->antialias)
          : zoomSurface(source.get(), zoom_x, zoom_y, layer->antialias));
      if (!cache.surface) {
        report(layer->rotating ? "rotate layer" : "scale layer");
        return;
      }
      cache.generation = generation;
      cache.angle = angle;
      cache.zoom_x = zoom_x;
      cache.zoom_y = zoom_y;
      cache.antialias = layer->antialias;
    }
    const auto *surface = cache.surface.get();
    transformed = &layer->geo_rotozoom;
    if (transformed->w != surface->w || transformed->h != surface->h ||
        transformed->bytewidth != surface->pitch) {
      transformed->init(surface->w, surface->h, 32);
      transformed->bytewidth = surface->pitch;
      layer->need_crop = true;
    }
    pixels = surface->pixels;
  } else {
    cache.surface.reset();
  }
  if (layer->need_crop || layer->blitter->geo != (transformed ? transformed : &layer->geo))
    layer->blitter->crop(layer, this, transformed);
  if (!blit->lay_height)
    return;

  if (blit->type == Blit::LINEAR && blit->fun) {
    auto *out = static_cast<uint32_t *>(get_surface()) + blit->scr_offset;
    auto *in = static_cast<uint32_t *>(pixels) + blit->lay_offset;
    for (int row = 0; row < blit->lay_height; ++row) {
      blit->fun(in, out, blit->lay_bytepitch, &blit->parameters);
      if (row + 1 < blit->lay_height) {
        out += blit->scr_stride;
        in += blit->lay_stride;
      }
    }
  } else if (blit->type == Blit::SDL && blit->sdl_fun) {
    SDL_Rect src = blit->sdl_rect, dst = blit->sdl_destination;
    if (const char *operation =
            blit->sdl_fun(pixels, &src, sdl_screen, &dst, layer->blitter->geo, &blit->parameters)) {
      report(operation);
      return;
    }
  }
  drawing.recovered("Drawing layer", layer->name);
}

void SdlScreen::resize(int resize_w, int resize_h) {
  auto *surface = SDL_CreateSurface(resize_w, resize_h, freej_pixel_format());
  if (!surface || !SDL_SetWindowSize(window, resize_w, resize_h)) {
    error("Unable to resize viewport '%s' from %ux%u to %dx%d: %s",
          name, geo.w, geo.h, resize_w, resize_h, SDL_GetError());
    SDL_DestroySurface(surface);
    return;
  }
  SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
  SDL_DestroySurface(sdl_screen);
  sdl_screen = surface;
  geo.init(resize_w, resize_h, 32);
}

void *SdlScreen::coords(int x, int y) {
  if (!sdl_screen || x < 0 || y < 0 || x >= sdl_screen->w || y >= sdl_screen->h)
    return nullptr;
  return static_cast<uint8_t *>(sdl_screen->pixels) + y * sdl_screen->pitch
      + x * sizeof(uint32_t);
}

void SdlScreen::show() {
  if (switch_fullscreen) {
    const bool full = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
    if (!SDL_SetWindowFullscreen(window, !full))
      error("unable to switch fullscreen: %s", SDL_GetError());
    switch_fullscreen = false;
  }

  auto *target = SDL_GetWindowSurface(window);
  if (!target)
    viewport_error(present_failure_, "get window surface");
  else if (!SDL_BlitSurfaceScaled(sdl_screen, nullptr, target, nullptr, SDL_SCALEMODE_NEAREST))
    viewport_error(present_failure_, "copy frame to window");
  else if (!SDL_UpdateWindowSurface(window))
    viewport_error(present_failure_, "update window surface");
  else
    present_failure_.recovered("Presenting viewport", name);
}

void SdlScreen::viewport_error(FailureEpisode &episode, const char *operation) {
  if (episode.first())
    error("Viewport '%s' (%ux%u): %s failed: %s", name, geo.w, geo.h,
          operation, SDL_GetError());
}

void *SdlScreen::get_surface() {
  return sdl_screen ? sdl_screen->pixels : nullptr;
}

void SdlScreen::clear() {
  if (!SDL_FillSurfaceRect(sdl_screen, nullptr, 0))
    viewport_error(clear_failure_, "clear frame");
  else
    clear_failure_.recovered("Clearing viewport", name);
}
void SdlScreen::fullscreen() {
  switch_fullscreen = true;
}

bool SdlScreen::lock() {
  if (SDL_MUSTLOCK(sdl_screen) && !SDL_LockSurface(sdl_screen)) {
    viewport_error(lock_failure_, "lock frame");
    return false;
  }
  lock_failure_.recovered("Locking viewport", name);
  return true;
}

bool SdlScreen::unlock() {
  if (SDL_MUSTLOCK(sdl_screen)) {
    SDL_UnlockSurface(sdl_screen);
  }
  return true;
}

