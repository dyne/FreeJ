/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: freej.cpp 654 2005-08-18 16:52:47Z jaromil $"
 *
 */

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <jsnum.h>
#include <config.h>
#include <geo_layer.h>

DECLARE_CLASS("GeometryLayer",geometry_layer_class,geometry_layer_constructor);

////////////////////////////////
// Geometry Layer methods

JSFunctionSpec geometry_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  { "clear",          geometry_layer_clear,          0 },
  { "color",          geometry_layer_color,          4 },
  { "pixel",          geometry_layer_pixel,          2 },
  { "hline",          geometry_layer_hline,          3 },
  { "vline",          geometry_layer_vline,          3 },
  { "rectangle",      geometry_layer_rectangle,      4 },
  { "rectangle_fill", geometry_layer_rectangle_fill, 4 },
  { "line",           geometry_layer_line,           4 },
  { "aaline",         geometry_layer_aaline,         4 },
  { "circle",         geometry_layer_circle,         3 },
  { "aacircle",       geometry_layer_aacircle,       3 },
  { "circle_fill",    geometry_layer_circle_fill,    3 },
  { "ellipse",        geometry_layer_ellipse,        4 },
  { "aaellipse",      geometry_layer_aaellipse,      4 },
  { "ellipse_fill",   geometry_layer_ellipse_fill,   4 },
  { "pie",            geometry_layer_pie,            5 },
  { "pie_fill",       geometry_layer_pie_fill,       5 },
  { "trigon",         geometry_layer_trigon,         6 },
  { "aatrigon",       geometry_layer_aatrigon,       6 },
  { "trigon_fill",    geometry_layer_trigon_fill,    6 },
  //  { "polygon", geometry_layer_polygon, 4},
  //  { "aapolygon", geometry_layer_aapolygon, 4},
  //  { "polygon_fill", geometry_layer_polygon_fill, 4},
  //  { "bezier", geometry_layer_bezier, 5},
  {0}
};

// JSBool fun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
JS_CONSTRUCTOR("GeometryLayer",geometry_layer_constructor,GeoLayer);  

/// color handling, a macro and some overloading tricks
#define OPTIONAL_COLOR_ARG(num) \
  uint32_t color; \
  if( argc>=(num+1) ) { \
    if(JSVAL_IS_DOUBLE(argv[num])) \
      color = (uint32_t) *(JSVAL_TO_DOUBLE(argv[num])); \
    else \
      color = (uint32_t) (JSVAL_TO_INT(argv[num])); \
  } else \
    color = lay->color;

JS(geometry_layer_clear) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  GET_LAYER(GeoLayer);
  lay->clear();

  return JS_TRUE;
}



JS(geometry_layer_color) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(1);
  
  GET_LAYER(GeoLayer);

  // color accepts arguments in many ways
  // R,G,B,A or R,G,B or the whole 32bit value
  uint16_t r,g,b,a;

  if(JSVAL_IS_DOUBLE(argv[0])) {

    double *hex;
    hex = JSVAL_TO_DOUBLE(argv[0]);
    lay->color = (uint32_t)*hex;
    
  } else {
      js_ValueToUint16(cx, argv[0], &r);
      js_ValueToUint16(cx, argv[1], &g);
      js_ValueToUint16(cx, argv[2], &b);
      if (argc == 4) 
          js_ValueToUint16(cx, argv[3], &a);
      else
        a = 0xff;
    
      lay->color = a|(r<<8)|(g<<16)|(b<<24);
  }
  return JS_TRUE;
}

JS(geometry_layer_pixel) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(2);

  GET_LAYER(GeoLayer);
  
  uint16_t x,y;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);

  OPTIONAL_COLOR_ARG(2);

  lay->pixel(x, y, color);

  return JS_TRUE;
}
JS(geometry_layer_hline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(3);

  GET_LAYER(GeoLayer);

  uint16_t x1,x2,y;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &x2);
  js_ValueToUint16(cx, argv[2], &y);

  OPTIONAL_COLOR_ARG(3);

  lay->hline(x1, x2, y, color);

  return JS_TRUE;
}
JS(geometry_layer_vline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(3);

  GET_LAYER(GeoLayer);

  uint16_t x,y1,y2;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y1);
  js_ValueToUint16(cx, argv[2], &y2);

  OPTIONAL_COLOR_ARG(3);

  lay->vline(x, y1, y2, color);

  return JS_TRUE;
}

JS(geometry_layer_rectangle) {
  //  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x1,x2,y1,y2;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);

  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);

//uint32_t col;
  OPTIONAL_COLOR_ARG(4);
  
  lay->rectangle(x1, y1, x2, y2, color);

  return JS_TRUE;
}
JS(geometry_layer_rectangle_fill) {
  //  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x1,x2,y1,y2;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);

  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);

//uint32_t col;
  OPTIONAL_COLOR_ARG(4);

  lay->rectangle_fill(x1, y1, x2, y2, color);

  return JS_TRUE;
}
JS(geometry_layer_line) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x1,x2,y1,y2;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);

  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);

  OPTIONAL_COLOR_ARG(4);

  lay->line(x1, y1, x2, y2, color);

  return JS_TRUE;
}
JS(geometry_layer_aaline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x1,x2,y1,y2;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);

  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);

  OPTIONAL_COLOR_ARG(4);
  
  lay->aaline(x1, y1, x2, y2, color);

  return JS_TRUE;
}
JS(geometry_layer_circle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(3); 

  GET_LAYER(GeoLayer);

  uint16_t x,y,r;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &r);

  OPTIONAL_COLOR_ARG(3);

  lay->circle(x, y, r, color);

  return JS_TRUE;
}
JS(geometry_layer_aacircle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(3);

  GET_LAYER(GeoLayer);

  uint16_t x,y,r;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &r);

  OPTIONAL_COLOR_ARG(3);

  lay->aacircle(x, y, r, color);

  return JS_TRUE;
}
JS(geometry_layer_circle_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(3);

  GET_LAYER(GeoLayer);

  uint16_t x,y,r;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &r);

  OPTIONAL_COLOR_ARG(3);

  lay->circle_fill(x, y, r, color);

  return JS_TRUE;
}
JS(geometry_layer_ellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x,y,rx,ry;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &rx);
  js_ValueToUint16(cx, argv[2], &ry);

  OPTIONAL_COLOR_ARG(4);

  lay->ellipse(x, y, rx, ry, color);

  return JS_TRUE;
}
JS(geometry_layer_aaellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x,y,rx,ry;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &rx);
  js_ValueToUint16(cx, argv[2], &ry);

  OPTIONAL_COLOR_ARG(4);

  lay->aaellipse(x, y, rx, ry, color);

  return JS_TRUE;
}
JS(geometry_layer_ellipse_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(4);

  GET_LAYER(GeoLayer);

  uint16_t x,y,rx,ry;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &rx);
  js_ValueToUint16(cx, argv[2], &ry);

  OPTIONAL_COLOR_ARG(4);

  lay->ellipse_fill(x, y, rx, ry, color);

  return JS_TRUE;
}
JS(geometry_layer_pie) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(5);

  GET_LAYER(GeoLayer);

  uint16_t x,y,rad,start,end;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &rad);
  js_ValueToUint16(cx, argv[3], &start);
  js_ValueToUint16(cx, argv[4], &end);

  OPTIONAL_COLOR_ARG(5);

  lay->pie(x, y, rad, start, end, color);

  return JS_TRUE;
}
JS(geometry_layer_pie_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(5);

  GET_LAYER(GeoLayer);

  uint16_t x,y,rad,start,end;
  js_ValueToUint16(cx, argv[0], &x);
  js_ValueToUint16(cx, argv[1], &y);
  js_ValueToUint16(cx, argv[2], &rad);
  js_ValueToUint16(cx, argv[3], &start);
  js_ValueToUint16(cx, argv[4], &end);

  OPTIONAL_COLOR_ARG(5);

  lay->pie_fill(x, y, rad, start, end, color);

  return JS_TRUE;
}
JS(geometry_layer_trigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(6);

  GET_LAYER(GeoLayer);

  uint16_t x1,y1,x2,y2,x3,y3;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);
  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);
  js_ValueToUint16(cx, argv[4], &x3);
  js_ValueToUint16(cx, argv[5], &y3);

  OPTIONAL_COLOR_ARG(6);

  lay->trigon(x1, y1, x2, y2, x3, y3, color);

  return JS_TRUE;
}
JS(geometry_layer_aatrigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(6);

  GET_LAYER(GeoLayer);

  uint16_t x1,y1,x2,y2,x3,y3;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);
  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);
  js_ValueToUint16(cx, argv[4], &x3);
  js_ValueToUint16(cx, argv[5], &y3);

  OPTIONAL_COLOR_ARG(6);

  lay->aatrigon(x1, y1, x2, y2, x3, y3, color);

  return JS_TRUE;
}
JS(geometry_layer_trigon_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  JS_CHECK_ARGC(6);

  GET_LAYER(GeoLayer);

  uint16_t x1,y1,x2,y2,x3,y3;
  js_ValueToUint16(cx, argv[0], &x1);
  js_ValueToUint16(cx, argv[1], &y1);
  js_ValueToUint16(cx, argv[2], &x2);
  js_ValueToUint16(cx, argv[3], &y2);
  js_ValueToUint16(cx, argv[4], &x3);
  js_ValueToUint16(cx, argv[5], &y3);

  OPTIONAL_COLOR_ARG(6);

  lay->trigon_fill(x1, y1, x2, y2, x3, y3, color);

  return JS_TRUE;
}
/// TODO: polygon and bezier

