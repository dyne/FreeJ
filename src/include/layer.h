/*  FreeJ
 *  (c) Copyright 2001-2010 Denis Roio <jaromil@dyne.org>
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

/**
   @file layer.h
   @brief FreeJ generic Layer interface
*/

#ifndef __LAYER_H__
#define __LAYER_H__

#include <inttypes.h>
#include <geometry.h>
#include <filter.h>
#include <screen.h>
#include <jsync.h>


class Context;
class AudioCollector;
class Iterator;
class Blitter;
class Blit;
class ViewPort;


class JSContext;


/**
   This class describes methods and properties common to all Layers in
   FreeJ: it is the main interface for functionalities like blit
   changes, filter management and position changes.

   The public methods hereby described are matching the javascript API
   which is made available by the internal parser.

   Methods implemented to create and destroy a Layer:
   - Layer::init
   - Layer::open
   - Layer::close

   Miscellaneus operations made available for the Layer:
   - Layer::set_position
   - Layer::set_zoom
   - Layer::set_rotate

   Blitter interface of the Layer
   - Layer::blitter
   - Layer::current_blit
   - Layer::set_blit
   - Layer::get_blit

   LinkList of Filter instances added to the Layer:
   - Layer::filters

   LinkList of Parameters available for the Layer:
   - Layer::parameters

   Geometrical informations about the layer:
   - Layer::geo

   @brief Provides input to the Context
*/
class Layer: public Entry, public JSyncThread {
  friend class Blitter;
  friend class Context;
  friend class JSyncThread;
  friend class ViewPort;

 public:

  enum Type {
    UNKNOWN,
    TEXT,
    GENERATOR,
#if defined HAVE_DARWIN && defined WITH_COCOA
    GL_COCOA
#endif
  };

  Layer(); ///< Layer constructor
  virtual ~Layer(); ///< Layer destructor

  /* wrap JSyncThread::start() so we don't export JSyncThread on SWIG */

  /**
     Start the layer thread
  */
  virtual int start() { return JSyncThread::start(); }

  Type type; ///< type of the layer

  /* these must be defined in layer implementations */

  /**
     Open a file or resource for the layer
     @param file string describing the path to the file, can be also an url
  */
  virtual bool open(const char *file) =0; ///< open the file (first called)

  
  virtual bool init(int w = 0, int h = 0, int bpp = 0);
  ///< initializes the layer (this will call also the layer implementation's init)

  virtual void close() =0; ///< close the layer (ready to open a new one)

  virtual bool set_parameter(int idx); ///< activate the setting on parameter pointed by idx index number

  char *get_name() { return name; };
  char *get_filename() { return filename; };
  ///< Get Layer's filename

  /**
     Move the layer to absolute position,
     coordinates refer to the upper left corner
     @param x horizontal coordinate
     @param y vertical coordinate
  */
  virtual void set_position(int x, int y);
  ///< Set Layer's position on screen

  /**
     Set the zoom rate (magnification) for a layer
     the coordinates are floats, original size is 1.0
     @param x horizontal zoom float coefficient (default 1.0)
     @param y vertical zoom float coefficient (default 1.0)
  */
  virtual void set_zoom(double x, double y); ///< Zoom (resize) a Layer
  /**
     Degrees of rotation
     @param angle from 0 to 360 degrees rotation
   */
  virtual void set_rotate(double angle); ///< Rotate a Layer

  bool antialias;
  bool zooming;
  bool rotating;
  double zoom_x;
  double zoom_y;
  double rotate;

  virtual void fit(bool maintain_aspect_ratio = true);


  Linklist<Parameter> *parameters;
  ///< Parameter list for the layer

  Linklist<FilterInstance> filters;
  ///< Filter list of effects applied on the Layer
  virtual void *do_filters(void *tmp_buf); ///< process all filters on a buffer

  Geometry geo;
  ///< Geometrical information about the Layer
  Geometry geo_rotozoom;
  ///< Geometrical information about the Rotozoom

  Linklist<Iterator> iterators;
  ///< Iterator list of value modifiers
  int do_iterators(); ///< process all registered iterators


  bool active; // is active? (read-only)
  bool hidden; // is hidden (read-only by the blit)
  bool fade; // layer is deactivated at the end of current iterations (read-write internal)
  bool use_audio; // layer makes use of audio input
  bool opened; // set by the layer (ex: image file has been opened)
  bool need_crop; // tell the screen that the layer need a crop (r/w internal)
  int bgcolor; // matte background color
  int null_feeds; // counter of how many sequencial feed() returned null
  int max_null_feeds; // maximum null feeds tolerated

  //////////////////////// BLIT operations
  Blitter *blitter; ///< Blitter interface for this Layer
  Blit *current_blit; ///< currently set Blit on this Layer
  virtual char *get_blit(); ///< return the name of the currently seleted Blit
  virtual bool set_blit(const char *bname); ///< select a Blit by name

  virtual void blit(); // operates the current blit
    
  void *get_data(); // returns private data associated to this layer
  void set_data(void *data);

  ViewPort *screen;  ///< ViewPort on which the Layer is blitted


  AudioCollector *audio; //< registered audio collector

  /** physical buffers */
  void *buffer; ///< RGBA pixel buffer returned by the layer


  void *js_constructor(Context *env, JSContext *cx,
                       JSObject *obj, int argc, void *aargv, char *err_msg);
  ///< javascript layer constructor
  //  void layer_gc(JSContext *cx, JSObject *obj);

  unsigned int textureID; ///< opengl texture id

  int frame_rate; ///< value set by implemented layer type

 protected:

  /**
     Initialize the layer implementation
     @param freej freej context where the layer will be used
   */
  virtual bool _init(); ///< implementation specific _init() to be present in Layer subclassess (if needed)

  void set_filename(const char *f);
  char filename[256];


  bool is_native_sdl_surface;
  void *priv_data; // pointer to private data eventually associated to this layer

 private:

  void _set_position(int x, int y);
  void _set_zoom(double x, double y);
  void _set_rotate(double angle);
  void _fit(bool maintain_aspect_ratio);

  char alphastr[5];

  void thread_setup();
  void thread_loop();
  void thread_teardown();

  virtual void *feed() = 0; ///< feeds in the image source

  bool cafudda(); ///< cafudda is called by the Context

  // working variables
  int res;
  Iterator *iter;
  Iterator *itertmp;
  // colorkey point
  uint8_t colorkey_r;
  uint8_t colorkey_g;
  uint8_t colorkey_b;

};


#endif
