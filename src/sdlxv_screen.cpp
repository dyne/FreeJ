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

#include <stdlib.h>
#include <string.h>
#include <SDL_syswm.h>

#include <sdlxv_screen.h>
#include <ccvt.h>
#include <jutils.h>
#include <config.h>

SdlXvScreen::SdlXvScreen()
  : ViewPort() {
  yuv_overlay = NULL;
  w = h = 0;
  size = pitch = 0;
  bpp = 32;
  anal = NULL;
}

SdlXvScreen::~SdlXvScreen() {
  if(anal) free(anal);
  SDL_FreeYUVOverlay(yuv_overlay);
  SDL_Quit();
}

bool SdlXvScreen::init(int width, int height) {
  setenv("SDL_VIDEO_YUV_DIRECT", "1", 1);
  setenv("SDL_VIDEO_HWACCEL", "1", 1);
  
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
    return false;
  
  scr = SDL_SetVideoMode
    ( width, height, 0,
      SDL_HWSURFACE |                // SDL_ASYNCBLIT |
      SDL_DOUBLEBUF | SDL_HWACCEL );
      //SDL_RESIZABLE );
  if(!scr) {
    error("%s",SDL_GetError());
    return(false);
  }

  w = width;
  h = height;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);
  
  yuv_overlay = SDL_CreateYUVOverlay(w, h, SDL_YV12_OVERLAY, scr);
  if(!yuv_overlay)
    yuv_overlay = SDL_CreateYUVOverlay(w, h, SDL_YUY2_OVERLAY, scr);
  if(!yuv_overlay)
    yuv_overlay = SDL_CreateYUVOverlay(w, h, SDL_UYVY_OVERLAY, scr);
  if(!yuv_overlay) {
    error("can't create Xv YUV overlay : %s",SDL_GetError());
    return false;
  }

  /*
  if (yuv_overlay->pitches[0] != yuv_overlay->pitches[1] * 2
      || yuv_overlay->pitches[0] != yuv_overlay->pitches[2] * 2) {
    error("SDL returned non YUV 420 overlay");
    return false;
  }
  */

  anal = malloc(w*h*4);
  
  rect.x = 0; rect.y = 0;
  rect.w = w; rect.h = h;

  notice("SDL XV Viewport is %ix%i %ibpp",w,h,scr->format->BytesPerPixel<<3);
  act("YUV overlay surface Y%i:U%i:V%i",
      yuv_overlay->pitches[0], yuv_overlay->pitches[1], yuv_overlay->pitches[2]);
  act("Overlay hardware acceleration is %s",
      (yuv_overlay->hw_overlay) ? "ON" : "OFF");

  //  if(!sdl_lock()) return(false);

  /* TEST: draw a gradient
     uint8_t *sbuffer = (uint8_t *) scr->pixels;
     for (int i = 0; i < scr->h; ++i) {
     memset(sbuffer, (i * 255) / scr->h,
     scr->w * scr->format->BytesPerPixel);
     sbuffer += scr->pitch;
     } */

  //  if(!sdl_unlock()) return (false);

  /* be nice with the window manager */
  char temp[120];
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);

  /* hide mouse cursor */
  SDL_ShowCursor(SDL_DISABLE);

  SDL_UpdateRect(scr,0,0,w,h);

  return(true);
}  
    
void SdlXvScreen::show() {
  //  SDL_DisplayYUVOverlay(yuv_overlay, &rect);
  SDL_Flip(scr);
}

bool SdlXvScreen::sdl_lock() {
  if (!SDL_MUSTLOCK(scr)) return true;
  if (SDL_LockSurface(scr) < 0) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}
  
bool SdlXvScreen::yuv_lock() {
  if (SDL_LockYUVOverlay(yuv_overlay) < 0) {
    error("%s", SDL_GetError());
    return false;
  }
  return true;
}

bool SdlXvScreen::sdl_unlock() {
  if (SDL_MUSTLOCK(scr)) {
    SDL_UnlockSurface(scr);
  }
  return true;
}

bool SdlXvScreen::yuv_unlock() {
  SDL_UnlockYUVOverlay(yuv_overlay);
  return true;
}

void *SdlXvScreen::get_surface() {
  return anal;
}

void *SdlXvScreen::coords(int x, int y) {
  return anal;
}

/*
bool SdlXvScreen::update(void *buf) {
  if(!yuv_lock()) return false;  

  ccvt_rgb32_bgr24(w, h, buf, anal);
  ccvt_bgr24_420p(w, h, anal, 
		  yuv_overlay->pixels[0], // y
		  yuv_overlay->pixels[2], // v
		  yuv_overlay->pixels[1]);// u
  yuv_unlock();
  SDL_DisplayYUVOverlay(yuv_overlay, &rect);
  //memcpy(scr->pixels,buf,size);
  return(true);
}
*/

void SdlXvScreen::clear() {
  /* TODO */
}
