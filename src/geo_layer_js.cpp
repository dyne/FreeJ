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
#include <config.h>
#include <geo_layer.h>

DECLARE_CLASS("GeometryLayer",geometry_layer_class,geometry_layer_constructor);

////////////////////////////////
// Geometry Layer meth
JSFunctionSpec geometry_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {     "clear",        geometry_layer_clear,   1},
  {     "pixel",        geometry_layer_pixel,   3},
  {     "hline",        geometry_layer_hline,   4},
  {     "vline",        geometry_layer_vline,   4},
  {     "rectangle",         geometry_layer_rectangle, 5},
  {     "rectangle_fill",    geometry_layer_rectangle_fill, 5},
  {     "line", geometry_layer_line, 5},
  { "aaline", geometry_layer_aaline, 5},
  { "circle", geometry_layer_circle, 4},
  { "aacircle", geometry_layer_aacircle, 4},
  { "circle_fill", geometry_layer_circle_fill, 4},
  { "ellipse", geometry_layer_ellipse, 5},
  { "aaellipse", geometry_layer_aaellipse, 5},
  { "ellipse_fill", geometry_layer_ellipse_fill, 5},  
  { "pie", geometry_layer_pie, 6},
  { "pie_fill", geometry_layer_pie_fill, 6},
  { "trigon", geometry_layer_trigon, 7},
  { "aatrigon", geometry_layer_aatrigon, 7},
  { "trigon_fill", geometry_layer_trigon_fill, 7},
  //  { "polygon", geometry_layer_polygon, 4},
  //  { "aapolygon", geometry_layer_aapolygon, 4},
  //  { "polygon_fill", geometry_layer_polygon_fill, 4},
  //  { "bezier", geometry_layer_bezier, 5},
  {0}
};

JS_CONSTRUCTOR("GeometryLayer",geometry_layer_constructor,GeoLayer);  
JS(geometry_layer_clear) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  int color = 0x0;
  if(argc>0) color = JSVAL_TO_INT(argv[0]);
  GET_LAYER(GeoLayer);
  lay->clear(color);
  return JS_TRUE;
}
JS(geometry_layer_pixel) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  
  if(argc<3) return JS_FALSE;

  GET_LAYER(GeoLayer);
  
  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int col = JSVAL_TO_INT(argv[2]);

  lay->pixel(x,y,col);

  return JS_TRUE;
}
JS(geometry_layer_hline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int x2 = JSVAL_TO_INT(argv[1]);
  int y = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);
  
  lay->hline(x1, x2, y, col);

  return JS_TRUE;
}
JS(geometry_layer_vline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int y2 = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->vline(x, y1, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_rectangle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->rectangle(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_rectangle_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->rectangle_fill(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_line) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->line(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_aaline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);
  
  lay->aaline(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_circle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->circle(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_aacircle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->aacircle(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_circle_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->circle_fill(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_ellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->ellipse(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_aaellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->aaellipse(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_ellipse_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->ellipse_fill(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_pie) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<6) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rad = JSVAL_TO_INT(argv[2]);
  int start = JSVAL_TO_INT(argv[3]);
  int end = JSVAL_TO_INT(argv[4]);
  int col = JSVAL_TO_INT(argv[5]);

  lay->pie(x, y, rad, start, end, col);

  return JS_TRUE;
}
JS(geometry_layer_pie_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rad = JSVAL_TO_INT(argv[2]);
  int start = JSVAL_TO_INT(argv[3]);
  int end = JSVAL_TO_INT(argv[4]);
  int col = JSVAL_TO_INT(argv[5]);

  lay->pie_fill(x, y, rad, start, end, col);

  return JS_TRUE;
}
JS(geometry_layer_trigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->trigon(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
JS(geometry_layer_aatrigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->aatrigon(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
JS(geometry_layer_trigon_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->trigon_fill(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
/// TODO: polygon and bezier

