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

#include <sdl_screen.h>
#include <jutils.h>
#include <config.h>

SdlScreen::SdlScreen()
  : ViewPort() {
  
  scr = NULL;
  emuscr = NULL;
  bpp = 32;
  dbl = false;
  sdl_flags = (SDL_HWSURFACE| SDL_ASYNCBLIT | SDL_DOUBLEBUF | SDL_HWACCEL);
  setenv("SDL_VIDEO_YUV_DIRECT", "1", 1);
  setenv("SDL_VIDEO_HWACCEL", "1", 1);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0x00000000;
#endif
  
}

SdlScreen::~SdlScreen() {
  SDL_Quit();
}

bool SdlScreen::init(int width, int height) {
  /* initialize SDL */
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    error("Can't initialize SDL: %s",SDL_GetError());
  }

  setres(width,height);

  w = width;
  h = height;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  rect.x = 0; rect.y = 0;
  rect.w = w; rect.h = h;

  /* be nice with the window manager */
  char temp[120];
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);

  /* hide mouse cursor */
  SDL_ShowCursor(SDL_DISABLE);

  //  SDL_UpdateRect(scr,0,0,w,h);
  surface = SDL_GetVideoSurface()->pixels;
  
  return(true);
}

void SdlScreen::show() {
  SDL_Flip(scr);
}

void *SdlScreen::get_surface() {
  return surface;
}

bool SdlScreen::update(void *buf) {

  blit = SDL_CreateRGBSurfaceFrom
    (buf, w, h, bpp, pitch, rmask, gmask, bmask, amask);
  if(!blit) {
    error("SdlScreen::update : %s",SDL_GetError());
    return false;
  }

  SDL_BlitSurface(blit, NULL, SDL_GetVideoSurface(), NULL);

  SDL_FreeSurface(blit);
  return true;
}

void SdlScreen::clear() {
  memset(surface,0x0,size);
}
void SdlScreen::fullscreen() {
  SDL_WM_ToggleFullScreen(scr);
}

bool SdlScreen::sdl_lock() {
  if (!SDL_MUSTLOCK(scr)) return true;
  if (SDL_LockSurface(scr) < 0) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}

bool SdlScreen::sdl_unlock() {
  if (SDL_MUSTLOCK(scr)) {
    SDL_UnlockSurface(scr);
  }
  return true;
}

int SdlScreen::setres(int wx, int hx) {
  /* check and set available videomode */
  int res = SDL_VideoModeOK(wx, hx, bpp, sdl_flags);

  act("SDL viewport is %ux%u %ubpp",wx,hx,res);

  scr = SDL_SetVideoMode(wx, hx, bpp, sdl_flags);
  if( scr == NULL ) {
    error("can't set video mode to %ux%u %ubpp: %s\n",
	  wx, hx, bpp, SDL_GetError());
    return(false);
  }

  if(res!=bpp) {
    act("your screen does'nt support %ubpp",bpp);
    act("doing video surface software conversion");
    
    emuscr = SDL_GetVideoSurface();
    act("emulated surface geometry %ux%u %ubpp",
	emuscr->w,emuscr->h,emuscr->format->BitsPerPixel);
  }  
  return res;
}
