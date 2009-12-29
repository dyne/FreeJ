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
#include <geometry.h>

#include <screen.h>
#include <linklist.h>
#include <parameter.h>

/////////////////////////////////// blit functions prototypes
#define BLIT static inline void

typedef void (blit_f)(void *src, void *dst, int len, Linklist<Parameter> *params);

typedef void (blit_sdl_f)(void *src, SDL_Rect *src_rect,
			  SDL_Surface *dst, SDL_Rect *dst_rect,
			  Geometry *geo, Linklist<Parameter> *params);

typedef void (blit_past_f)(void *src, void *past, void *dst, int len);

///////////////////////////////////////////////////////////////////////

class Layer;


template <class T> class Linklist;

class Blitter;
class ViewPort;

////// blit setup functions to be used by the screen
void setup_sdl_blits(Blitter *blitter);
void setup_linear_blits(Blitter *blitter);


class Blit: public Entry {
  friend class Blitter;
  friend class ViewPort;

 public:

  Blit();
  ~Blit();

  char desc[512];    ///< long description
  float value;    ///< parameter value

  Linklist<Parameter> parameters; ///< linklist of blit parameters

  blit_f *fun; ///< pointer to linear blit function
  blit_sdl_f *sdl_fun; ///< pointer to sdl blit function
  blit_past_f *past_fun; ///< pointer to past blit function

  enum BlitType {
	  NONE = 0,
	  LINEAR = 1,
	  SDL = 2,
	  PAST = 3
  };

//#define LINEAR_BLIT 1
//#define SDL_BLIT 2
//#define PAST_BLIT 3
  BlitType type; ///< LINEAR|SDL|PAST type

  //  char *get_name();


  int32_t scr_stride_dx;
  int32_t scr_stride_sx;
  int32_t scr_stride_up;
  int32_t scr_stride;
  uint32_t scr_offset;

  int32_t lay_pitch;
  int32_t lay_bytepitch;
  int32_t lay_stride;
  int32_t lay_stride_sx;
  int32_t lay_stride_dx;
  int32_t lay_stride_up;
  int32_t lay_height;
  uint32_t lay_offset;

  SDL_Rect sdl_rect; // sdl crop rectangle

 private:
  // parameters for linear crop
  




  // past blit buffer
  void *past_frame;

/*   /\* small vars used in blits *\/ */
/*   int chan, c, cc; */
/*   uint32_t *scr, *pscr, *off, *poff, *pastoff, *ppastoff; */

};



class Blitter {
 public:
  Blitter();
  ~Blitter();


  Linklist<Blit> blitlist; ///< list of available blits

  /* ==== CROP */
  /** @param force crop even if nothing changed */
  void crop(Layer *lay, ViewPort *scr);
  ///< crop to fit in the ViewPort
  
  ViewPort *screen; ///< the layer on which is applied the blitter

  Blit *default_blit;

  Geometry *geo;
  
 private:
  int16_t old_lay_x;
  int16_t old_lay_y;
  uint16_t old_lay_w;
  uint16_t old_lay_h;

  int16_t old_scr_x;
  int16_t old_scr_y;
  uint16_t old_scr_w;
  uint16_t old_scr_h;


};

#endif
