/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#include <sdl_screen.h>
#include <layer.h>

void SdlScreen::crop(Layer *layer) {

  /* crop for the SDL blit */
  layer->rect.x = -(layer->geo.x);
  layer->rect.y = -(layer->geo.y);
  layer->rect.w = w;
  layer->rect.h = h;

  /* crop for our own blit */
  uint32_t blit_xoff=0;
  uint32_t blit_yoff=0;
  
  blit_x = layer->geo.x;
  blit_y = layer->geo.y;
  blit_width = layer->geo.w;
  blit_height = layer->geo.h;
  blit_xoff = 0;
  blit_yoff = 0;

  func("CROP x[%i] y[%i] w[%i] h[%i] on screen w[%i] h[%i]",
       layer->geo.x,layer->geo.y,layer->geo.w,layer->geo.h,w,h);

  /* left bound 
     affects x-offset and width */
  if(layer->geo.x<0) {
    blit_xoff = (-layer->geo.x);
    blit_x = 1;

    if(blit_xoff>layer->geo.w) {
      func("layer out of screen to the left");
      layer->hidden = true; /* out of the screen */
      layer->geo.x = -(layer->geo.w+1); /* don't let it go far */      
    } else {
      layer->hidden = false;
      blit_width -= blit_xoff;
    }
  }

  /* right bound
     affects width */
  if((layer->geo.x+layer->geo.w)>w) {
    if(layer->geo.x>w) { /* out of screen */
      func("layer out of screen to the right");
      layer->hidden = true; 
      layer->geo.x = w+1; /* don't let it go far */
    } else {
      layer->hidden = false;
      blit_width -= (layer->geo.w - (w - layer->geo.x));
    }
  }

  /* upper bound
     affects y-offset and height */
  if(layer->geo.y<0) {
    blit_yoff = (-layer->geo.y);
    blit_y = 1;

    if(blit_yoff>layer->geo.h) {
      func("layer out of screen up");
      layer->hidden = true; /* out of the screen */
      layer->geo.y = -(layer->geo.h+1); /* don't let it go far */      
    } else {
      layer->hidden = false;
      blit_height -= blit_yoff;
    }
  }

  /* lower bound
     affects height */
  if((layer->geo.y+layer->geo.h)>h) {
    if(layer->geo.y>h) { /* out of screen */
      func("layer out of screen down");
      layer->hidden = true; 
      layer->geo.y = h+1; /* don't let it go far */
    } else {
      layer->hidden = false;
      blit_height -= (layer->geo.h - (h - layer->geo.y));
    }
  }
  
  blit_coords = (uint32_t*)coords(blit_x,blit_y);

  blit_offset = (blit_xoff<<2) + (blit_yoff*layer->geo.pitch);


  func("CROP x[%i] y[%i] w[%i] h[%i] xoff[%i] yoff[%i]",
       blit_x, blit_y, blit_width, blit_height, blit_xoff, blit_yoff);
}


#define BLIT(op) \
    { \
      scr = pscr = (uint32_t*) blit_coords; \
      off = poff = (uint32_t*) ((uint8_t*)layer->offset+blit_offset); \
      for(c=blit_height;c>0;c--) { \
	for(cc=blit_width;cc>0;cc--) { \
	  *scr op *off; \
	  scr++; off++; \
	} \
	off = poff = poff + layer->geo.w; \
	scr = pscr = pscr + w; \
      } \
    }


#define SDL_BLIT(b,g,r,a,opacity) \
  blitter = SDL_CreateRGBSurfaceFrom \
    (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp, \
     layer->geo.pitch, b, g, r, a); \
  if(!blitter) { \
    error("SDL_CreateRGBSurfaceFrom : %s",SDL_GetError()); \
    return; \
  } \
  SDL_SetAlpha(blitter,SDL_SRCALPHA,opacity); \
  SDL_BlitSurface(blitter,&layer->rect,SDL_GetVideoSurface(),NULL); \
  SDL_FreeSurface(blitter);


void SdlScreen::blit(Layer *layer) {
  /* transparence aware blits:
     if alpha channel is 0x00 then pixel is not blitted
     works with 32bpp */
  
  if(layer->hidden) return;

  /* SOLID COLOR BLIT (NO ALPHA TRANSPARENCE) */
  switch(layer->blit) {
    
  case 1:
    //BLIT(=);
    blitter = SDL_CreateRGBSurfaceFrom
      (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, layer->bmask, layer->gmask, layer->rmask, 0x0);
    if(!blitter) {
      error("SDL_CreateRGBSurfaceFrom : %s",SDL_GetError());
      return;
    }
    SDL_BlitSurface(blitter,&layer->rect,SDL_GetVideoSurface(),NULL);
    SDL_FreeSurface(blitter);
    return;
    
  case 2:
  case 3:
  case 4:
    {
      chan = layer->blit-2;
      char *scr, *pscr, *off, *poff;
      scr = pscr = (char *) blit_coords;
      off = poff = (char *) ((Uint8*)layer->offset+blit_offset);
      for(c=blit_height;c>0;c--) {
	for(cc=blit_width;cc>0;cc--) {
	  *(scr+chan) = *(off+chan);
	  scr+=4; off+=4;
	}
	off = poff = poff + layer->geo.pitch;
	scr = pscr = pscr + pitch;
      }
    }
    return;
    
  case 5:
    BLIT(+=);
    return;
    
  case 6:
    BLIT(-=);
    return;
    
  case 7:
    BLIT(&=);
    return;
    
  case 8:
    BLIT(|=);
    return;

  case 9:
    blitter = SDL_CreateRGBSurfaceFrom
      (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, layer->bmask, layer->gmask, layer->rmask, 0x0);
    if(!blitter) {
      error("SDL_CreateRGBSurfaceFrom : %s",SDL_GetError());
      return;
    }
    SDL_SetAlpha(blitter,SDL_SRCALPHA,layer->alpha);
    SDL_BlitSurface(blitter,&layer->rect,SDL_GetVideoSurface(),NULL);
    SDL_FreeSurface(blitter);
    break;

  default:
    error("invalid algo blit");
    return;
  }
  
}


/* SIMPLE C CODE
   (where all this blit trip started)
    
  char *scr, *pscr;
  scr = pscr = (char *) coords(geo.x,geo.y);
  char *off, *poff;
  off = poff = (char *)video;
  int c,cc;
  for(c=geo.h;c>0;c--) {
    off = poff = poff + geo.pitch;
    scr = pscr = pscr + pitch;
    for(cc=geo.pitch;cc>0;cc--) *scr++ = *off++;
  }
  */
