/*  FreeJ
 *  (c) Copyright 2009-2010 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __FREEJ_CAIRO_H__
#define __FREEJ_CAIRO_H__

#include <config.h>
#ifdef WITH_CAIRO

#include <color.h>

#include <cairo.h>

class CairoColor: public Color {

 public:
  CairoColor(cairo_t *cai);
  ~CairoColor();

 protected:
  void set();

 private:
  cairo_t *cairo;

};

class CairoLayer: public Layer {

 public:
  CairoLayer();
  ~CairoLayer();

  bool open(const char *file);
  void *feed();
  void close();

  cairo_t *cairo;

  Color *color;
  JSBool set_color(JSContext *cx, uintN argc, jsval *argv, int idx);

  ///////////////////////////////////////////////
  // public methods exported to language bindings

  // Cairo API
  void save();
  void restore();
  void new_path();
  void close_path();
  void fill();
  void stroke();
  void scale(double xx, double yy);
  void rotate(double angle);
  void translate(int xx, int yy);
  void line_to(double xx, double yy);
  void move_to(double xx, double yy);
  void curve_to(int x1, int y1, int x2, int y2, int x3, int y3);
  void arc(double xc, double yc, double radius, double angle1, double angle2);
  void set_line_width(double wid);
  int get_line_width();

  // Mozilla's GFX compatibility API
  void quad_curve_to(double x1, double y1, double x2, double y2);
  void fill_rect(double x1, double y1, double x2, double y2);
  

 protected:
  bool _init();

 private:
  cairo_surface_t *surf;
  void *pixels;

  int stride;
  
   // allow to use Factory on this class
  FACTORY_ALLOWED
};

#endif
#endif
