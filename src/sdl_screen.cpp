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

#include <layer.h>
#include <blitter.h>
#include <linklist.h>
#include <sdl_screen.h>

#include <SDL_imageFilter.h>
#include <SDL_rotozoom.h>

#include <jutils.h>
#include "sdl_pixels.h"

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, SdlScreen, Screen, sdl);

SdlScreen::SdlScreen()
  : ViewPort(), sdl_screen(nullptr), window(nullptr), switch_fullscreen(false),
    pre_rotozoom(nullptr), rotozoom(nullptr) {
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
    error("Can't initialize SDL: %s", SDL_GetError());
    return false;
  }

  sdl_screen = SDL_CreateSurface(geo.w, geo.h, freej_pixel_format());
  if (sdl_screen)
    window = SDL_CreateWindow(PACKAGE " " VERSION, geo.w, geo.h, 0);
  if (!window) {
    error("Can't create SDL viewport: %s", SDL_GetError());
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

void SdlScreen::blit(Layer *src) {
  register int16_t c;
  void *offset;
  Blit *b;

  if(src->rotating | src->zooming) {

    // if we have to rotate or scale,
    // create a sdl surface from current pixel buffer
    pre_rotozoom = SDL_CreateSurfaceFrom(src->geo.w, src->geo.h,
        freej_pixel_format(), src->buffer, src->geo.bytewidth);
    if (!pre_rotozoom)
      return;
    if (src->rotating)
      rotozoom = rotozoomSurface(pre_rotozoom, src->rotate, src->zoom_x, (int)src->antialias);
    else
      rotozoom = zoomSurface(pre_rotozoom, src->zoom_x, src->zoom_y, (int)src->antialias);
    SDL_DestroySurface(pre_rotozoom);
    pre_rotozoom = nullptr;
    if (!rotozoom)
      return;
    offset = rotozoom->pixels;
    // free the temporary surface (needed again in sdl blits)
    src->geo_rotozoom.init( rotozoom->w, rotozoom->h, src->geo.bpp);


  } else offset = src->buffer;



  if(src->need_crop)
    src->blitter->crop( src, this );

  b = src->current_blit;

//   if(!b) {
//     error("%s: blit is NULL",__PRETTY_FUNCTION__);
//     return;
//   }

  // executes LINEAR blit
  if( b->type == Blit::LINEAR ) {
    
    pscr = (uint32_t*) get_surface() + b->scr_offset;
    play = (uint32_t*) offset        + b->lay_offset;
    
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
    
    // executes SDL blit
  } else if (b->type == Blit::SDL) {

    if (src->blitter->geo)
        (*b->sdl_fun)
          (offset, &b->sdl_rect, sdl_screen,
           NULL, src->blitter->geo, &b->parameters);
    
  }

//  else if (b->type == Blit::PAST) {
//     // this is a linear blit which operates
//     // line by line on the previous frame

//     // setup crop variables
//     pscr  = (uint32_t*)get_surface() + b->scr_offset;
//     play  = (uint32_t*)offset        + b->lay_offset;
//     ppast = (uint32_t*)b->past_frame + b->lay_offset;

//     // iterates the blit on each horizontal line
//     for(c = b->lay_height; c>0; c--) {
      
//       (*b->past_fun)
// 	((void*)play, (void*)ppast, (void*)pscr,
// 	 b->lay_bytepitch);
      
//       // copy the present to the past
//       jmemcpy(ppast, play, geo->pitch);
      
//       // strides down to the next line
//       pscr  += b->scr_stride + b->lay_pitch;
//       play  += b->lay_stride + b->lay_pitch;
//       ppast += b->lay_stride + b->lay_pitch;

//     }
//   }

  // free rotozooming temporary surface
  if(rotozoom) {
    SDL_DestroySurface(rotozoom);
    rotozoom = NULL;
  }
  
}

void SdlScreen::resize(int resize_w, int resize_h) {
  auto *surface = SDL_CreateSurface(resize_w, resize_h, freej_pixel_format());
  if (!surface || !SDL_SetWindowSize(window, resize_w, resize_h)) {
    error("unable to resize SDL viewport: %s", SDL_GetError());
    SDL_DestroySurface(surface);
    return;
  }
  SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
  SDL_DestroySurface(sdl_screen);
  sdl_screen = surface;
  geo.init(resize_w, resize_h, 32);
}

void *SdlScreen::coords(int x, int y) {
  return 
    ( x + geo.pixelsize*y +
      (uint32_t*)sdl_screen->pixels );
}

void SdlScreen::show() {
  if (switch_fullscreen) {
    const bool full = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
    if (!SDL_SetWindowFullscreen(window, !full))
      error("unable to switch fullscreen: %s", SDL_GetError());
    switch_fullscreen = false;
  }

  auto *target = SDL_GetWindowSurface(window);
  if (!target || !SDL_BlitSurfaceScaled(sdl_screen, nullptr, target, nullptr, SDL_SCALEMODE_NEAREST)
      || !SDL_UpdateWindowSurface(window))
    error("unable to present SDL viewport: %s", SDL_GetError());
}

void *SdlScreen::get_surface() {
  return sdl_screen->pixels;
}

void SdlScreen::clear() {
  SDL_FillSurfaceRect(sdl_screen,NULL,0x0);
}
void SdlScreen::fullscreen() {
  switch_fullscreen = true;
}

bool SdlScreen::lock() {
  if (!SDL_MUSTLOCK(sdl_screen)) return true;
  if (!SDL_LockSurface(sdl_screen)) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}

bool SdlScreen::unlock() {
  if (SDL_MUSTLOCK(sdl_screen)) {
    SDL_UnlockSurface(sdl_screen);
  }
  return true;
}

