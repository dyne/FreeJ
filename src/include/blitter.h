/*  FreeJ - blitter layer component
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __BLITTER_H__
#define __BLITTER_H__

#include <SDL.h>
#include <freej.h>
#include <linklist.h>
#include <screen.h>

typedef void (blit_f)(void *src, void *dst, int len, void *value);

typedef void (blit_sdl_f)(void *src, SDL_Rect *src_rect,
			  SDL_Surface *dst, SDL_Rect *dst_rect,
			  ScreenGeometry *geo, void *value);

class Layer;


class Blit: public Entry {
  friend class Blitter;
 public:

  Blit();
  ~Blit();

  char desc[512];    ///< long description
  uint32_t value;    ///< parameter value
  short kernel[256]; ///< convolution kernel
  
  blit_f *fun; ///< pointer to linear blit function
  blit_sdl_f *sdl_fun; ///< pointer to sdl blit function

#define LINEAR_BLIT 1
#define SDL_BLIT 2
  int type; ///< LINEAR|SDL_BLIT type

  char *get_name() { return name; };


 private:
  // parameters for linear blit crop
  int32_t blit_x;
  int32_t blit_y;
  uint32_t blit_width;
  uint32_t blit_height;
  uint32_t blit_pitch;
  uint32_t blit_offset;
  uint32_t *blit_coords;

  // sdl blit crop rectangle
  SDL_Rect sdl_rect;

  /* small vars used in blits */
  int chan, c, cc;
  uint32_t *scr, *pscr, *off, *poff;

};



class Blitter {
 public:
  Blitter();
  ~Blitter();


  bool init(Layer *lay); ///< initialize the blitter


  /* ==== BLITS */
  void blit();
  bool set_blit(char *name); ///< set the active blit
  bool set_value(int val); ///< set the blit parameter
  bool set_kernel(short *krn); /// set the convolution kernel
  bool Blitter::set_colorkey(int x,int y);
  Linklist blitlist; ///< list of available blits

  /* ==== geometrical transformations */
  double x_scale;    ///< zoom factor on x axis
  double y_scale;    ///< zoom factor on y axis
  double rotation;   ///< rotation factor
  
  
  /* ==== CROP */
  /** @param view if NULL, default ViewPort is used */
  void crop(ViewPort *view);
  ///< crop to fit in the ViewPort
  
  Layer *layer; ///< the layer on which is applied the blitter

  Blit *current_blit; ///< currently selected blit
  
 private:
  int16_t old_x;
  int16_t old_y;
  uint16_t old_w;
  uint16_t old_h;

  SDL_Surface *sdl_dest;
};

#endif
