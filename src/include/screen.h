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

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <inttypes.h>
#include <config.h>
#include <SDL.h>

#include <closure.h>
#include <linklist.h>
#include <ringbuffer.h>

#include <layer.h>
#include <blitter.h>


template <class T> class Linklist;

///////////////////////
// GLOBAL COLOR MASKING

#ifdef WITH_COCOA
#define red_bitmask   (uint32_t)0x0000ff00
#define rchan         2
#define green_bitmask (uint32_t)0x00ff0000
#define gchan         1
#define blue_bitmask  (uint32_t)0xff000000
#define bchan         0
#define alpha_bitmask (uint32_t)0x000000ff
#define achan         3
#else
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define red_bitmask   (uint32_t)0x00ff0000
#define rchan         2
#define green_bitmask (uint32_t)0x0000ff00
#define gchan         1
#define blue_bitmask  (uint32_t)0x000000ff
#define bchan         0
#define alpha_bitmask (uint32_t)0xff000000
#define achan         3
#else
// I got much better performance with same bitmasks on my ppc
// maybe runtime detect these like SDL testvidinfo.c?
#define red_bitmask   (uint32_t)0x00ff0000
#define rchan         1
#define green_bitmask (uint32_t)0x0000ff00
#define gchan         2
#define blue_bitmask  (uint32_t)0x000000ff
#define bchan         3
#define alpha_bitmask (uint32_t)0xff000000
#define achan         0
#endif
#endif

class JackClient;
class Layer;
class Context;
class VideoEncoder;

class ViewPort : public Entry {
  friend class Layer;
 public:
  ViewPort(int w, int h);
  virtual ~ViewPort();


  enum fourcc { RGBA32, BGRA32, ARGB32 }; ///< pixel formats understood
  virtual fourcc get_pixel_format() =0; ///< return the pixel format

  virtual void *get_surface() =0; ///< returns direct pointer to video memory

  virtual void *coords(int x, int y) =0;
  ///< returns pointer to pixel (slow! use it once and then move around)

  virtual void blit(Layer *src) =0; ///< operate the blit

  virtual void setup_blits(Layer *lay) =0; ///< setup available blits on added layer

  Context *env;

  void blit_layers();

  virtual bool add_layer(Layer *lay); ///< add a new layer to the screen
  virtual bool add_audio(JackClient *jcl); ///< connect layer to audio output
  virtual void rem_layer(Layer *lay); ///< remove a layer from the screen
    
  Linklist<Layer> layers; ///< linked list of registered layers

  bool add_encoder(VideoEncoder *enc); ///< add a new encoder for the screen

  Linklist<VideoEncoder> encoders; ///< linked list of registered encoders

  void handle_resize();

  virtual void set_magnification(int algo) { };
  virtual void resize(int resize_w, int resize_h) { };
  virtual void show() { };
  virtual void clear() { };

  virtual void fullscreen() { };
  virtual bool lock() { return(true); };
  virtual bool unlock() { return(true); };

  ringbuffer_t *audio; ///< FIFO ringbuffer for audio

  void scale2x(uint32_t *osrc, uint32_t *odst);
  void scale3x(uint32_t *osrc, uint32_t *odst);
  int w, h;
  int bpp;
  int size, pitch;
  int magnification;

  bool changeres;
  bool resizing;
  int resize_w;
  int resize_h;
  

  // opengl special blit
  bool opengl;

 private:


  //  uint32_t red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask;  

};

#endif
