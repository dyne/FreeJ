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
#include <linklist.h>
#include <screen.h>

typedef void (blit_f)(void *src, void *dst, int len, void *value);

class Layer;


class Blit: public Entry {
  friend class Blitter;
 public:

  Blit();
  ~Blit();

  char desc[512];    ///< long description
  uint32_t value;    ///< parameter value
  short kernel[256]; ///< convolution kernel
  
  blit_f *fun; ///< pointer to blit function

#define LINEAR_BLIT 1
#define SDL_BLIT 2
#define PLANAR_BLIT 3
  int type; ///< LINEAR|PLANAR|SDL_BLIT type

 private:
  uint32_t blit_x;
  uint32_t blit_y;
  uint32_t blit_width;
  uint32_t blit_height;
  uint32_t blit_pitch;
  uint32_t blit_offset;
  uint32_t *blit_coords;
  SDL_Rect sdl_rect;
  SDL_Surface *sdl_surface;
  /* small vars used in blits? */
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
  
};

#endif
