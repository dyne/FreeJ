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
  
  screen = NULL;
  emuscr = NULL;
  bpp = 32;
  dbl = false;
  sdl_flags = (SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL);

}

SdlScreen::~SdlScreen() {
  SDL_Quit();
}

bool SdlScreen::init(int width, int height) {
  /* initialize SDL */
  setenv("SDL_VIDEO_HWACCEL", "1", 1);  

  if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0 ) {
    error("Can't initialize SDL: %s",SDL_GetError());
    return(false);
  }

  setres(width,height);

  w = width;
  h = height;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  notice("SDL Viewport is %ix%i %ibpp",w,h,screen->format->BytesPerPixel<<3);

  /* be nice with the window manager */
  char temp[120];
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);

  /* hide mouse cursor */
  SDL_ShowCursor(SDL_DISABLE);

  return(true);
}

void *SdlScreen::coords(int x, int y) {
  return 
    ( x + (w*y) +
      (uint32_t*)SDL_GetVideoSurface()->pixels );
}

void SdlScreen::show() {
  SDL_Flip(screen);
}

void *SdlScreen::get_surface() {
  return SDL_GetVideoSurface()->pixels;
}

void SdlScreen::clear() {
  memset(SDL_GetVideoSurface()->pixels,0x0,size);
}
void SdlScreen::fullscreen() {
  SDL_WM_ToggleFullScreen(screen);
}

bool SdlScreen::lock() {
  if (!SDL_MUSTLOCK(screen)) return true;
  if (SDL_LockSurface(screen) < 0) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}

bool SdlScreen::unlock() {
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
  return true;
}

int SdlScreen::setres(int wx, int hx) {
  /* check and set available videomode */
  int res;
  res = SDL_VideoModeOK(wx, hx, bpp, sdl_flags);
  
  
  screen = SDL_SetVideoMode(wx, hx, bpp, sdl_flags);
  //  screen = SDL_SetVideoMode(wx, hx, 0, sdl_flags);
  if( screen == NULL ) {
    error("can't set video mode: %s\n", SDL_GetError());
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
