/*  FreeJ
 *  (c) Copyright 2009 Denis Roio <jaromil@dyne.org>
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
 *
 */

#include <config.h>

#ifdef WITH_CAIRO
#include <jutils.h>
#include <context.h>


#include <cairo_layer.h>

#include <jsparser_data.h>
#include <factory.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, CairoLayer, VectorLayer, cairo);

CairoColor::CairoColor(cairo_t *cai) :Color() { cairo = cai; }
CairoColor::~CairoColor() { }
void CairoColor::set() { cairo_set_source_rgba(cairo, r, g, b, a); };

CairoLayer::CairoLayer()
  :Layer() {


  surf = NULL;
  cairo = NULL;
  color = NULL;
  pixels = NULL;

  set_name("VEC");
  set_filename("/vector layer");
  jsclass = &vector_layer_class;
}

CairoLayer::~CairoLayer() {
  
  if(cairo)  cairo_destroy(cairo);
  if(surf)   cairo_surface_destroy(surf);
  if(pixels) free(pixels);
  if(color) free(color);
}

bool CairoLayer::_init() {
  // create the surface
  stride = cairo_format_stride_for_width
    (CAIRO_FORMAT_ARGB32, geo.w);
  pixels = malloc (stride * geo.h);
  surf = cairo_image_surface_create_for_data
    ((unsigned char*)pixels, CAIRO_FORMAT_ARGB32, geo.w, geo.h, stride);
  // create the drawing context
  cairo = cairo_create(surf);
  // This  function references  target,  so you  can immediately  call
  // cairo_surface_destroy()  on it if  you don't  need to  maintain a
  // separate reference to it.

  color = new CairoColor( cairo );

  // test
  cairo_set_line_width(cairo, 0.1 );   
  cairo_set_source_rgb(cairo, 1.0, 1.0, 0.0); 

  opened = true;
  return(true);

}

void *CairoLayer::feed() {
  return(pixels);
}

bool CairoLayer::open(const char *file) {
  /* we don't need this */
  return true;
}

void CairoLayer::close() {
  /* neither this */
  return;
}

///////////////////////////////////////////////
// public methods exported to language bindings

// Cairo API
void CairoLayer::save() { cairo_save(cairo); }
void CairoLayer::restore() { cairo_restore(cairo); }
void CairoLayer::new_path() { cairo_new_path(cairo); }
void CairoLayer::close_path() { cairo_close_path(cairo); }
void CairoLayer::scale(double xx, double yy) { cairo_scale(cairo, xx, yy); }
void CairoLayer::rotate(double angle) { cairo_rotate(cairo, angle); }
void CairoLayer::translate(int xx, int yy) { cairo_translate(cairo, xx, yy); }
void CairoLayer::move_to(double xx, double yy) { cairo_move_to(cairo, xx, yy); }
void CairoLayer::line_to(double xx, double yy) { cairo_line_to(cairo, xx, yy); }
void CairoLayer::curve_to(int x1, int y1, int x2, int y2, int x3, int y3) {
    cairo_curve_to(cairo, x1, y1, x2, y2, x3, y3); }
void CairoLayer::arc(double xc, double yc, double radius, double angle1, double angle2) {
  cairo_arc(cairo, xc, yc, radius, angle1, angle2); }
void CairoLayer::fill() { cairo_fill(cairo); }
void CairoLayer::stroke() { cairo_stroke(cairo); }
void CairoLayer::set_line_width(double wid) { cairo_set_line_width(cairo, wid); }
int CairoLayer::get_line_width() { return cairo_get_line_width(cairo); }

// Mozilla's GFX compatibility API
void CairoLayer::quad_curve_to(double x1, double y1, double x2, double y2) {
  double xc, yc;
  cairo_get_current_point(cairo, &xc, &yc);
  cairo_curve_to(cairo,
		 (xc + x1 * 2.0) / 3.0,
		 (yc + y1 * 2.0) / 3.0,
		 (x1 * 2.0 + x2) / 3.0,
		 (y1 * 2.0 + y2) / 3.0,
		 x2, y2);
}

void CairoLayer::fill_rect(double x1, double y1, double x2, double y2) {
  cairo_save(cairo);
  cairo_rectangle(cairo, x1, y1, x2, y2);
  cairo_fill(cairo);
  cairo_restore(cairo);
}


#endif
