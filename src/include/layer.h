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

/**
   @file layer.h
   @brief FreeJ generic Layer interface
*/

#ifndef __LAYER_H__
#define __LAYER_H__

#include <inttypes.h>
#include <iterator.h>
#include <blitter.h>
#include <filter.h>
#include <jsync.h>

class Context;



/**
   This class describes methods and properties common to all Layers in
   FreeJ: it is the main interface for functionalities like blit
   changes, filter management and position changes.

   The public methods hereby described are matching the javascript API
   which is made available by the internal parser.

   Methods implemented to create and destroy a layer:
   - Layer::open
   - Layer::init
   - Layer::close
   
   Miscellaneus operations made available for the layer:
   - Layer::set_position
   - Layer::set_blit
   - Layer::get_blit
   - Layer::set_alpha
   - Layer::get_name

   LinkList of filters used on the layer:
   - Layer::filters

   Pointer to the initialized Context where the layer is used:
   - Layer::freej

   Geometrical informations about the layer:
   - Layer::geo
   
   @brief Layer parent abstract class
*/
class Layer: public Entry, public JSyncThread {
  friend class Blitter;
  friend class Context;
  friend class JSyncThread;

 public:

  Layer(); ///< Layer constructor
  ~Layer(); ///< Layer destructor
  
  /* these must be defined in layer implementations */
  virtual bool open(char *file) =0; ///< open the file (first called)
  virtual bool init(Context *scr) =0; ///< initialize the layer (second called)
  virtual void close() =0; ///< close the layer (ready to open a new one)
  virtual bool keypress(char key) =0; ///< pass to the Layer a key pressed

  char *get_name() { return name; };
  char *get_filename() { return filename; };
  ///< Get Layer's filename

  void set_position(int x, int y);
  ///< Set Layer's position on screen


  Linklist filters;
  ///< Filter list of effects applied on the Layer

  ScreenGeometry geo;
  ///< Geometrical information about the Layer

  Linklist iterators;
  ///< Iterator list of value modifiers

  bool active; ///< is active? (read-only)
  bool quit; ///< should it quit? (read-write)
  bool running; ///< is running? (read-only)
  bool hidden; ///< is hidden (read-only by the blit)
  int bgcolor; ///< matte background color

  Blitter blitter; ///< blitter class

  /** physical buffers */
  void *offset; ///< pointer to pixel plane

 protected:

  void _init(Context *freej, int wdt, int hgt, int bpp=0);
  ///< Layer abstract initialization

  Context *freej;

  void set_filename(char *f);
  char filename[256];

  void *buffer; ///< feed buffer returned by layer implementation


 private:

  char alphastr[5];

  void run(); ///< Main Layer thread loop

  virtual void *feed() = 0; ///< feeds in the image source

  bool cafudda(); ///< cafudda is called by the Context

  void *bgmatte;

  // working variables
  int res;
  Filter *filt;
  Iterator *iter;
  Iterator *itertmp;
  // colorkey point
  uint8_t colorkey_r;
  uint8_t colorkey_g;
  uint8_t colorkey_b;

  bool has_colorkey;

};

/* function for type detection of implemented layers */
extern const char *layers_description;
extern Layer *create_layer(char *file);
///< create the propriate Layer type from a file



#endif
