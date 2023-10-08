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

///////////////////////////////////////////////////////////////////
// SDL BLITS
// TODO: more SDL blits playing with color masks

// temporary surface used in SDL blits
static SDL_Surface *sdl_surf;

BLIT sdl_rgb(void *src, SDL_Rect *src_rect,
	     SDL_Surface *dst, SDL_Rect *dst_rect,
	     Geometry *geo, Linklist<Parameter> *param) {
  
  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->bytewidth, red_bitmask, green_bitmask, blue_bitmask, 0x0);
  
  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  //SDL_UpdateRects(sdl_surf, 1, dst_rect);
  
  SDL_FreeSurface( sdl_surf );
};

BLIT sdl_alpha(void *src, SDL_Rect *src_rect,
	       SDL_Surface *dst, SDL_Rect *dst_rect,
	       Geometry *geo, Linklist<Parameter> *params) {

  float alpha = *(float*)(params->begin()->value); // only one value
  unsigned int int_alpha = (unsigned int) alpha;

  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->bytewidth, red_bitmask, green_bitmask, blue_bitmask, 0x0);

  SDL_SetSurfaceAlphaMod( sdl_surf, int_alpha );

  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );
  
};

BLIT sdl_srcalpha(void *src, SDL_Rect *src_rect,
		  SDL_Surface *dst, SDL_Rect *dst_rect,
		  Geometry *geo, Linklist<Parameter> *params) {

  float alpha = *(float*)(params->begin()->value); // only one value
  unsigned int int_alpha = (unsigned int) alpha;

  sdl_surf = SDL_CreateRGBSurfaceFrom
    (src, geo->w, geo->h, geo->bpp,
     geo->bytewidth, red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask);
  
  SDL_SetSurfaceAlphaMod( sdl_surf, int_alpha );

  SDL_BlitSurface( sdl_surf, src_rect, dst, dst_rect );
  
  SDL_FreeSurface( sdl_surf );
  
};

// BLIT sdl_chromakey(void *src, SDL_Rect *src_rect,
// 		   SDL_Surface *dst, SDL_Rect *dst_rect,
// 		   Geometry *geo, Linklist<Parameter> *params) {


//   // TODO color

//   sdl_surf = SDL_CreateRGBSurfaceFrom
//     (src, geo->w, geo->h, geo->bpp,
//      geo->bytewidth, red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask);

//   // TODO
//   SDL_SetColorKey( sdl_surf, SDL_FALSE, NULL);

//   //  SDL_SetAlpha(sdl_surf, SDL_RLEACCEL, 0);

//   SDL_Surface *colorkey_surf = SDL_ConvertSurfaceFormat(sdl_surf);

//   SDL_BlitSurface( colorkey_surf, src_rect, dst, dst_rect );

//   SDL_FreeSurface( sdl_surf );
//   SDL_FreeSurface( colorkey_surf );

// }

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
  
//   b = new Blit(); b->set_name("CHROMAKEY");
//   sprintf(b->desc,"chromakey blit (SDL)");
//   b->type = Blit::SDL; b->has_value = true;
//   b->sdl_fun = sdl_chromakey;
//   blitter->blitlist.prepend(b);

}
