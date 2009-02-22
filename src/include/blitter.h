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

#include <screen.h>


typedef void (blit_f)(void *src, void *dst, int len, void *value);

typedef void (blit_sdl_f)(void *src, SDL_Rect *src_rect,
			  SDL_Surface *dst, SDL_Rect *dst_rect,
			  ScreenGeometry *geo, void *value);

typedef void (blit_past_f)(void *src, void *past, void *dst, int len);

class Layer;

class Entry;
template <class T> class Linklist;


class Blit: public Entry {
  friend class Blitter;
 public:

  Blit();
  ~Blit();

  char desc[512];    ///< long description
  float value;    ///< parameter value
  short kernel[256]; ///< convolution kernel
  
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

  bool has_value;

  //  char *get_name();

 private:
  // parameters for linear crop
  
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




  // sdl crop rectangle
  SDL_Rect sdl_rect;

  // past blit buffer
  void *past_frame;

  /* small vars used in blits */
  int chan, c, cc;
  uint32_t *scr, *pscr, *off, *poff, *pastoff, *ppastoff;

};



class Blitter {
 public:
  Blitter();
  ~Blitter();

  bool init(Layer *lay); ///< initialize the blitter


  /* ==== BLITS */
  void blit(Layer *src);
  bool set_blit(const char *name); ///< set the active blit
  void set_value(float val); ///< set the blit value
  bool fade_value(float step, float val); ///< fade to a new blit value
  bool pulse_value(float step, float val); ///< pulse it to a value and come back
  bool set_kernel(short *krn); /// set the convolution kernel
  bool set_colorkey(int x,int y);

  bool set_zoom(double x, double y);
  bool set_rotate(double angle);
  bool set_spin(double rot, double z);

  Linklist<Entry> blitlist; ///< list of available blits

  /* ==== CROP */
  /** @param force crop even if nothing changed */
  void crop(bool force);
  ///< crop to fit in the ViewPort
  
  Layer *layer; ///< the layer on which is applied the blitter

  Blit *current_blit; ///< currently selected blit

  bool antialias;
  bool zooming;
  bool rotating;
  double zoom_x;
  double zoom_y;
  double rotate;
  double spin_rotation;
  double spin_zoom;

  ViewPort *screen;
  
 private:
  int16_t old_x;
  int16_t old_y;
  uint16_t old_w;
  uint16_t old_h;

  // generic blit buffer pointers
  uint32_t *pscr, *play, *ppast;



  SDL_Surface *sdl_dest;

  SDL_Surface *pre_rotozoom;
  SDL_Surface *rotozoom; ///< pointer to blittable surface (rotated and zoomed if necessary)

  ScreenGeometry geo_rotozoom; ///< geometrical information about the rotozoomed Layer
  ScreenGeometry *geo;

};

#endif
