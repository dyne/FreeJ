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
  // add above | SDL_FULLSCREEN to go fullscreen from the start

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    
    magnification = 0;
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
  surface = SDL_GetVideoSurface();

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
      (uint32_t*)surface->pixels );
}

void SdlScreen::show() {
  if(magnification==1) {
    lock();
    scale2x
	((uint32_t*)surface->pixels,
	 (uint32_t*)SDL_GetVideoSurface()->pixels);
    unlock();
  } else if(magnification==2) {
    lock();
    scale3x
	((uint32_t*)surface->pixels,
	 (uint32_t*)SDL_GetVideoSurface()->pixels);
    unlock();
  }
  SDL_Flip(screen);

}

void *SdlScreen::get_surface() {
  return surface->pixels;
}

void SdlScreen::clear() {
  SDL_FillRect(surface,NULL,0x0);
}
void SdlScreen::fullscreen() {
  SDL_WM_ToggleFullScreen(screen);
}

bool SdlScreen::lock() {
  if (!SDL_MUSTLOCK(surface)) return true;
  if (SDL_LockSurface(surface) < 0) {
    error("%s", SDL_GetError());
    return false;
  }
  return(true);
}

bool SdlScreen::unlock() {
  if (SDL_MUSTLOCK(surface)) {
    SDL_UnlockSurface(surface);
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

void SdlScreen::set_magnification(int algo) {

  if(magnification == algo) return;


  if(algo==0) {
    notice("screen magnification off");
    setres(w,h);
    if(magnification) SDL_FreeSurface(surface);
    surface = SDL_GetVideoSurface();

  } else if(algo==1) {

    notice("screen magnification scale2x");
    setres(w*2,h*2);

  } else if(algo==2) {

    notice("screen magnification scale3x");
    setres(w*3,h*3);

  } else {

    error("magnification algorithm %i not supported",algo);
    algo = magnification;

  }


  if(!magnification && algo) {
    func("create surface for magnification");
    surface = SDL_CreateRGBSurface
      (sdl_flags,w,h,bpp,bmask,gmask,rmask,amask);
      //      (SDL_HWSURFACE,w,h,bpp,bmask,gmask,rmask,amask);
  }

  magnification = algo;
  
}
    
