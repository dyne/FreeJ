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
#include <SDL_syswm.h>

#include <layer.h>
#include <blitter.h>
#include <linklist.h>
#include <sdl_screen.h>

#include <jutils.h>

#include <SDL_rotozoom.h>


SdlScreen::SdlScreen()
  : ViewPort() {
  
  sdl_screen = NULL;
  emuscr = NULL;

  rotozoom = NULL;
  pre_rotozoom = NULL;

  bpp = 32;
  dbl = false;
  sdl_flags = (SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWACCEL );
	  //| SDL_DOUBLEBUF | SDL_HWACCEL | SDL_RESIZABLE);
  // add above | SDL_FULLSCREEN to go fullscreen from the start

  magnification = 0;
  switch_fullscreen = false;

  //  set_name("SDL");
}

SdlScreen::~SdlScreen() {
  SDL_Quit();
}

bool SdlScreen::init(int width, int height) {
  char temp[120];
 
  /* initialize SDL */
  
  setenv("SDL_VIDEO_HWACCEL", "1", 1);  

  if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0 ) {
    error("Can't initialize SDL: %s",SDL_GetError());
    return(false);
  }

  setres(width,height);
  sdl_screen = SDL_GetVideoSurface();

  w = width;
  h = height;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);
  SDL_VideoDriverName(temp,120);

  notice("SDL Viewport is %s %ix%i %ibpp",
	 temp,w,h,sdl_screen->format->BytesPerPixel<<3);



  /* be nice with the window manager */
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);

  /* hide mouse cursor */
  SDL_ShowCursor(SDL_DISABLE);

  return(true);
}

void SdlScreen::blit(Layer *src) {
  register int16_t c;
  void *offset;
  Blit *b;

  if(src->rotating | src->zooming) {


    src->rotate += src->spin_rotation;
    src->rotate = // cycle rotation
      (src->rotate>360) ? (src->rotate-360):
      (src->rotate<0)   ? (360+src->rotate):
      src->rotate;
    
    src->spin_zoom = // cycle zoom
      (src->zoom_x >= 1.7) ? -src->spin_zoom :
      (src->zoom_x < 0.1)  ? -src->spin_zoom :
      src->spin_zoom;
    // zoom_y = zoom_x += spin_zoom;

    // if we have to rotate or scale,
    // create a sdl surface from current pixel buffer
    pre_rotozoom = SDL_CreateRGBSurfaceFrom
      (src->buffer,
       src->geo.w, src->geo.h, src->geo.bpp,
       src->geo.pitch, red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask);
    
    if(src->rotating) {
      
      rotozoom =
	rotozoomSurface(pre_rotozoom, src->rotate, src->zoom_x, (int)src->antialias);

    } else if(src->zooming) {
      
      rotozoom =
	zoomSurface(pre_rotozoom, src->zoom_x, src->zoom_y, (int)src->antialias);
      
    }
    
    offset = rotozoom->pixels;
    // free the temporary surface (needed again in sdl blits)
    src->geo_rotozoom.w = rotozoom->w;
    src->geo_rotozoom.h = rotozoom->h;


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
	 (void*)&b->parameters);
      
      // strides down to the next line
      pscr += b->scr_stride + b->lay_pitch;
      play += b->lay_stride + b->lay_pitch;
    }
    
    // executes SDL blit
  } else if (b->type == Blit::SDL) {

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
    SDL_FreeSurface(pre_rotozoom);
    pre_rotozoom = NULL;
    SDL_FreeSurface(rotozoom);
    rotozoom = NULL;
  }
  
}

void SdlScreen::resize(int resize_w, int resize_h) {
  act("resizing viewport to %u x %u",resize_w, resize_h);
  sdl_screen = SDL_SetVideoMode(resize_w,resize_h,32,sdl_flags);
  w = resize_w;
  h = resize_h;
  size = resize_w * resize_h * (bpp>>3);
  pitch = resize_w * (bpp>>3);
}

void *SdlScreen::coords(int x, int y) {
  return 
    ( x + (w*y) +
      (uint32_t*)sdl_screen->pixels );
}

void SdlScreen::show() {

  if(magnification==1) {
    lock();
    scale2x
	((uint32_t*)sdl_screen->pixels,
	 (uint32_t*)SDL_GetVideoSurface()->pixels);
    unlock();
  } else if(magnification==2) {
    lock();
    scale3x
	((uint32_t*)sdl_screen->pixels,
	 (uint32_t*)SDL_GetVideoSurface()->pixels);
    unlock();
  }

  if(switch_fullscreen) { 
#ifdef HAVE_DARWIN
#ifndef WITH_COCOA
    if((sdl_flags&SDL_FULLSCREEN) == SDL_FULLSCREEN)
        sdl_flags &= ~SDL_FULLSCREEN;
    else
        sdl_flags |= SDL_FULLSCREEN;
    screen = SDL_SetVideoMode(w, h, bpp, sdl_flags);
#else
    SDL_WM_ToggleFullScreen(sdl_screen);
#endif
#endif
    switch_fullscreen = false;
  }

  lock();
  SDL_Flip(sdl_screen);
  unlock();

}

void *SdlScreen::get_surface() {
  return sdl_screen->pixels;
}

void SdlScreen::clear() {
  SDL_FillRect(sdl_screen,NULL,0x0);
}
void SdlScreen::fullscreen() {
  switch_fullscreen = true;
}

bool SdlScreen::lock() {
  if (!SDL_MUSTLOCK(sdl_screen)) return true;
  if (SDL_LockSurface(sdl_screen) < 0) {
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

int SdlScreen::setres(int wx, int hx) {
  /* check and set available videomode */
  int res;
  act("setting resolution to %u x %u", wx, hx);
  res = SDL_VideoModeOK(wx, hx, bpp, sdl_flags);
  
  
  sdl_screen = SDL_SetVideoMode(wx, hx, bpp, sdl_flags);
  //  screen = SDL_SetVideoMode(wx, hx, 0, sdl_flags);
  if( sdl_screen == NULL ) {
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
    if(magnification) SDL_FreeSurface(sdl_screen);
    sdl_screen = SDL_GetVideoSurface();

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
    sdl_screen = SDL_CreateRGBSurface
      (sdl_flags,w,h,bpp,red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask);
      //(sdl_flags,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
      //      (SDL_HWSURFACE,w,h,bpp,blue_bitmask,green_bitmask,red_bitmask,alpha_bitmask);
  }

  magnification = algo;
  
}
    
