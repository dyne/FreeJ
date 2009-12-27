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

CairoLayer::CairoLayer()
  :Layer() {


  surf = NULL;
  cairo = NULL;
  pixels = NULL;

  set_name("VEC");
  set_filename("/vector layer");
  jsclass = &vector_layer_class;
}

CairoLayer::~CairoLayer() {
  
  if(cairo)  cairo_destroy(cairo);
  if(surf)   cairo_surface_destroy(surf);
  if(pixels) free(pixels);

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

  cairo_set_line_width(cairo, 3.0 );   
  cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0); 

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

#endif
