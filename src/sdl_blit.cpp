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
#include <imagefilter.h>
#include <layer.h>


void SdlScreen::blit(Layer *layer) {


  for(
  switch(layer->blit) {
    
  case 1: // plain RGB blit, we use SDL for this
    blitter = SDL_CreateRGBSurfaceFrom
      (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, bmask, gmask, rmask, 0x0);
    SDL_BlitSurface(blitter,&layer->rect,surface,NULL);
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
    
  case 5: // ADD
    SDL_imageFilterAdd(layer->offset,blit_coords,blit_coords,layer->geo.size);
    return;
  case 6: // SUB
    SDL_imageFilterSub(layer->offset,blit_coords,blit_coords,layer->geo.size);
    return;
    
  case 7: // AND
    SDL_imageFilterBitAnd(layer->offset,blit_coords,blit_coords,layer->geo.size);    
    return;
    
  case 8: // OR
    SDL_imageFilterBitOr(layer->offset,blit_coords,blit_coords,layer->geo.size);    
    return;

  case 9: // ALPHA
    blitter = SDL_CreateRGBSurfaceFrom
      (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, bmask, gmask, rmask, 0x0);
    SDL_SetAlpha(blitter,SDL_SRCALPHA,layer->alpha);
    SDL_BlitSurface(blitter,&layer->rect,surface,NULL);
    SDL_FreeSurface(blitter);
    break;
    
  case 10:
    SDL_imageFilterBitNegation(layer->offset,blit_coords,layer->geo.size);    
    break;

  }
}

#if 0

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


void SdlScreen::blit(Layer *layer) {
  /* works in 32bpp, if screen is another dept
     conversion is automatically done by SDL 
     (JAH bless Sam Lantinga!) */
  
  if(layer->hidden) return;

  switch(layer->blit) {
    
  case 1: // plain RGB blit, we use SDL for this
    blitter = SDL_CreateRGBSurfaceFrom
      (layer->offset, layer->geo.w, layer->geo.h, layer->geo.bpp,
       layer->geo.pitch, bmask, gmask, rmask, 0x0);
    SDL_BlitSurface(blitter,&layer->rect,surface,NULL);
    SDL_FreeSurface(blitter);
    //    BLIT(=);
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
       layer->geo.pitch, bmask, gmask, rmask, 0x0);
    SDL_SetAlpha(blitter,SDL_SRCALPHA,layer->alpha);
    SDL_BlitSurface(blitter,&layer->rect,surface,NULL);
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
#endif
