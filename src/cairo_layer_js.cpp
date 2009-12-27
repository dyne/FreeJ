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

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <jsnum.h>
#include <config.h>

#ifdef WITH_CAIRO
#include <cairo_layer.h>


DECLARE_CLASS_GC("VectorLayer", vector_layer_class, vector_layer_constructor, js_layer_gc);

////////////////////////////////
// Vector Layer methods
// basically exposing Cairo's API

JSFunctionSpec vector_layer_methods[] = {
  ENTRY_METHODS,
  { "translate",        vector_layer_translate,        2 },
  { "scale",            vector_layer_scale,            2 },
  { "rotate",           vector_layer_rotate,           1 },
  { "save",             vector_layer_save,             0 },
  { "restore",          vector_layer_restore,          0 },
  { "lineTo",           vector_layer_lineto,           2 },
  { "beginPath",        vector_layer_beginpath,        0 },
  { "moveTo",           vector_layer_moveto,           2 },
  { "quadraticCurveTo", vector_layer_quadcurveto,      4 },
  { "bezierCurveTo",    vector_layer_beziercurveto,    6 },
  { "arc",              vector_layer_arc,              5 },
  { "closePath",        vector_layer_closepath,        0 },
  { "fill",             vector_layer_fill,             0 },
  { "stroke",           vector_layer_stroke,           0 },
  {0}
};

// properties TODO: fillStyle, strokeStyle, lineCap, lineWidth

JS_CONSTRUCTOR("VectorLayer", vector_layer_constructor, CairoLayer);


JS(vector_layer_translate) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(tx, 0);
  JS_ARG_NUMBER(ty, 1);

  func("%s x[%.2f] y[%.2f]", __PRETTY_FUNCTION__, tx, ty);

  cairo_translate(lay->cairo, tx, ty);

  return JS_TRUE;
}

JS(vector_layer_scale) {
  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(sx, 0);
  JS_ARG_NUMBER(sy, 1);

  func("%s x[%.2f] y[%.2f]", __PRETTY_FUNCTION__, sx, sy );

  cairo_scale(lay->cairo, sx, sy);

  return JS_TRUE;

}

JS(vector_layer_rotate) {
  JS_CHECK_ARGC(1);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(angle, 0);

  func("%s angle[%.2f]", __PRETTY_FUNCTION__, angle);

  cairo_rotate(lay->cairo, angle);

  return JS_TRUE;

}

JS(vector_layer_save) {
  GET_LAYER(CairoLayer);
  cairo_save(lay->cairo);
  return JS_TRUE;
}
JS(vector_layer_restore) {
  GET_LAYER(CairoLayer);
  cairo_restore(lay->cairo);
  return JS_TRUE;
}

JS(vector_layer_lineto) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(x, 0);
  JS_ARG_NUMBER(y, 1);

  func("%s x[%.2f] y[%.2f]", __PRETTY_FUNCTION__ , x, y );

  cairo_line_to(lay->cairo, x, y);

  return JS_TRUE;
}

JS(vector_layer_beginpath) {
  GET_LAYER(CairoLayer);  
  cairo_new_path(lay->cairo);
  return JS_TRUE;
}
JS(vector_layer_closepath) {
  GET_LAYER(CairoLayer);  
  cairo_close_path(lay->cairo);
  return JS_TRUE;
}

JS(vector_layer_moveto) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(x, 0);
  JS_ARG_NUMBER(y, 1);

  func("%s x[%.2f] y[%.2f]", __PRETTY_FUNCTION__, x, y  );

  cairo_move_to(lay->cairo, x, y);

  return JS_TRUE;
}

JS(vector_layer_quadcurveto) {

  JS_CHECK_ARGC(4);
  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(x1, 0);
  JS_ARG_NUMBER(y1, 1);
  
  JS_ARG_NUMBER(x2, 2);
  JS_ARG_NUMBER(y2, 3);

  double xc, yc;

  cairo_get_current_point(lay->cairo, &xc, &yc);
  
  cairo_curve_to(lay->cairo,
		 (xc + x1 * 2.0) / 3.0,
		 (yc + y1 * 2.0) / 3.0,
		 (x1 * 2.0 + x2) / 3.0,
		 (y1 * 2.0 + y2) / 3.0,
		 x2, y2);

  return JS_TRUE;
}

JS(vector_layer_beziercurveto) {

  JS_CHECK_ARGC(6);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(x1, 0);
  JS_ARG_NUMBER(y1, 1);

  JS_ARG_NUMBER(x2, 2);
  JS_ARG_NUMBER(y2, 3);

  JS_ARG_NUMBER(x3, 4);
  JS_ARG_NUMBER(y3, 5);
  
  func("Vector bezier curve :: x1[%.2f] y1[%.2f] x2[%.2f] y2[%.2f] x3[%.2f] y3[%.2f]",
       x1, y1, x2, y2, x3, y3 );
  
  cairo_curve_to(lay->cairo,
		 x1, y1, x2, y2, x3, y3);

  return JS_TRUE;
}

JS(vector_layer_arc) {

  JS_CHECK_ARGC(5);

  GET_LAYER(CairoLayer);

  JS_ARG_NUMBER(xc, 0);
  JS_ARG_NUMBER(yc, 1);

  JS_ARG_NUMBER(radius, 2);

  JS_ARG_NUMBER(angle1, 3);
  JS_ARG_NUMBER(angle2, 4);

  func("Vector arc :: x[%.2f] y[%.2f] rad[%.2f] angle1[%.2f] angle2[%.2f]",
       xc, yc, radius, angle1, angle2 );
  
  cairo_arc(lay->cairo,
	    xc, yc, radius, angle1, angle2);

  return JS_TRUE;
}

JS(vector_layer_fill) {
  GET_LAYER(CairoLayer);  
  cairo_fill(lay->cairo);
  return JS_TRUE;
}
JS(vector_layer_stroke) {
  GET_LAYER(CairoLayer);  
  cairo_stroke(lay->cairo);
  return JS_TRUE;
}

/*
  { "quadraticCurveTo", vector_layer_quadcurveto,      4 },
  { "bezierCurveTo".    vector_layer_beziercurveto,    6 },
  { "arc",              vector_layer_arc,              5 },
  { "closePath",        vector_layer_closepath,        0 },
  { "fill",             vector_layer_fill,             0 },
  { "stroke",           vector_layer_stroke,           0 },
*/

#endif
