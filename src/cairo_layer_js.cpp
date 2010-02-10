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
 *
 */

#include <jsparser.h>
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
  { "set_color",        vector_layer_color,            4 },
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
  { "fill",             vector_layer_fill,             4 },
  { "fillRect",         vector_layer_fillrect,         4 },
  { "stroke",           vector_layer_stroke,           4 },
  {0}
};


JSPropertySpec vector_layer_properties[] = {
  { "fillStyle",   0, JSPROP_ENUMERATE | JSPROP_PERMANENT,
    vector_layer_fillstyle_g, vector_layer_fillstyle_s },
  { "strokeStyle", 1, JSPROP_ENUMERATE | JSPROP_PERMANENT,
    vector_layer_strokestyle_g, vector_layer_strokestyle_s },
  { "lineCap",     2, JSPROP_ENUMERATE | JSPROP_PERMANENT,
    vector_layer_linecap_g, vector_layer_linecap_s },
  { "lineWidth",   3, JSPROP_ENUMERATE | JSPROP_PERMANENT,
    vector_layer_linewidth_g, vector_layer_linewidth_s },
  {0}
};


// properties TODO: fillStyle, strokeStyle, lineCap, lineWidth

JS_CONSTRUCTOR("VectorLayer", vector_layer_constructor, CairoLayer);



JSBool CairoLayer::set_color(JSContext *cx, uintN argc, jsval *argv, int idx) {

  jsdouble r,g,b,a;

  if(argc==1) {

    g = js_get_double(argv[idx]);
    func("%s gray [%.2f]", __FUNCTION__, g);
    js_debug_property(cx, argv[idx]);
    color->set_gray(g);
    return JS_TRUE;

  } else if(argc==2) {

    g = js_get_double(argv[idx]);
    a = js_get_double(argv[idx+1]);

    // g is a double
    // a is a signed integer
    func("%s gray [%i] alpha [%i]", __FUNCTION__, g, a);
    //    js_debug_property(cx, &argv[idx]);
    //    js_debug_property(cx, &argv[idx+1]);

    color->set_gray_alpha(g,a);
    return JS_TRUE;

  } else if(argc==3) {
    r = js_get_double(argv[idx]);
    g = js_get_double(argv[idx+1]);
    b = js_get_double(argv[idx+2]);

    func("%s r[%.2f] g[%.2f] b[%.2f]", __FUNCTION__, r, g, b);
    //    js_debug_property(cx, &argv[idx]);
    //    js_debug_property(cx, &argv[idx+1]);
    //    js_debug_property(cx, &argv[idx+2]);

    color->set_rgb(r, g, b);
    return JS_TRUE;

  } else if(argc==4) {

    r = js_get_double(argv[idx]);
    g = js_get_double(argv[idx+1]);
    b = js_get_double(argv[idx+2]);
    a = js_get_double(argv[idx+3]);

    func("%s r[%.2f] g[%.2f] b[%.2f] a[%.2f]", __FUNCTION__, r, g, b, a);
    // js_debug_property(cx, &argv[idx]);
    // js_debug_property(cx, &argv[idx+1]);
    // js_debug_property(cx, &argv[idx+2]);
    // js_debug_property(cx, &argv[idx+3]);

    color->set_rgba(r, g, b, a);
    return JS_TRUE;
    
  }

  error("usage of color with %u args not (yet?) supported",argc);

  return JS_FALSE;
}

JS(vector_layer_color) {

  GET_LAYER(CairoLayer);

  lay->set_color(cx, argc, argv, 0);
  return JS_TRUE;
}

JS(vector_layer_translate) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  jsint tx, ty;
  
  tx = js_get_int(argv[0]);
  ty = js_get_int(argv[1]);

  func("%s x[%i] y[%i]", __FUNCTION__, tx, ty);

  lay->translate(tx, ty);

  return JS_TRUE;
}

JS(vector_layer_scale) {
  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  jsdouble sx, sy;
  sx = js_get_double(argv[0]);
  sy = js_get_double(argv[1]);

  func("%s x[%.2f] y[%.2f]", __FUNCTION__, sx, sy );

  lay->scale(sx, sy);

  return JS_TRUE;

}

JS(vector_layer_rotate) {
  JS_CHECK_ARGC(1);

  GET_LAYER(CairoLayer);

  jsdouble angle = js_get_double(argv[0]);

  func("%s angle[%.2f]", __FUNCTION__, angle);

  lay->rotate(angle);

  return JS_TRUE;

}

JS(vector_layer_save) {
  GET_LAYER(CairoLayer);
  lay->save();
  return JS_TRUE;
}
JS(vector_layer_restore) {
  GET_LAYER(CairoLayer);
  lay->restore();
  return JS_TRUE;
}

JS(vector_layer_lineto) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  jsint x, y;
  x = js_get_int(argv[0]);
  y = js_get_int(argv[1]);

  func("%s x[%i] y[%i]", __FUNCTION__ , x, y );

  lay->line_to(x, y);

  return JS_TRUE;
}

JS(vector_layer_beginpath) {
  GET_LAYER(CairoLayer);  
  lay->new_path();
  return JS_TRUE;
}
JS(vector_layer_closepath) {
  GET_LAYER(CairoLayer);  
  lay->close_path();
  return JS_TRUE;
}

JS(vector_layer_moveto) {

  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  jsint x, y;
  x = js_get_int(argv[0]);
  y = js_get_int(argv[1]);

  func("%s x[%i] y[%i]", __FUNCTION__, x, y  );

  lay->move_to(x, y);

  return JS_TRUE;
}

JS(vector_layer_quadcurveto) {

  JS_CHECK_ARGC(4);
  GET_LAYER(CairoLayer);

  jsint x1, y1;
  x1 = js_get_int(argv[0]);
  y1 = js_get_int(argv[1]);

  jsint x2, y2;
  x2 = js_get_int(argv[2]);
  y2 = js_get_int(argv[3]);

  
  lay->quad_curve_to( x1, y1, x2, y2) ;

  return JS_TRUE;
}

JS(vector_layer_beziercurveto) {

  JS_CHECK_ARGC(6);

  GET_LAYER(CairoLayer);

  jsint x1, y1;
  x1 = js_get_int(argv[0]);
  y1 = js_get_int(argv[1]);

  jsint x2, y2;
  x2 = js_get_int(argv[2]);
  y2 = js_get_int(argv[3]);

  jsint x3, y3;
  x3 = js_get_int(argv[4]);
  y3 = js_get_int(argv[5]);
  
  func("Vector bezier curve :: x1[%.2f] y1[%.2f] x2[%.2f] y2[%.2f] x3[%.2f] y3[%.2f]",
       x1, y1, x2, y2, x3, y3 );
  
  lay->curve_to(x1, y1, x2, y2, x3, y3);

  return JS_TRUE;
}

JS(vector_layer_arc) {

  JS_CHECK_ARGC(5);

  GET_LAYER(CairoLayer);

  jsint xc, yc;
  xc = js_get_int(argv[0]);
  yc = js_get_int(argv[1]);

  jsdouble radius, angle1, angle2;
  radius = js_get_double(argv[2]);
  angle1 = js_get_double(argv[3]);
  angle2 = js_get_double(argv[4]);

  func("Vector arc :: x[%i] y[%i] rad[%.2f] angle1[%.2f] angle2[%.2f]",
       xc, yc, radius, angle1, angle2 );
  
  lay->arc(xc, yc, radius, angle1, angle2);

  return JS_TRUE;
}

JS(vector_layer_fillrect) {

  JS_CHECK_ARGC(4);

  GET_LAYER(CairoLayer);  

  jsint x1, y1;
  x1 = js_get_int(argv[0]);
  y1 = js_get_int(argv[1]);

  jsint x2, y2;
  x2 = js_get_int(argv[2]);
  y2 = js_get_int(argv[3]);

  func("Vector fill_rect :: x1[%i] y1[%i] x2[%i] y2[%i]",
       x1, y1, x2, y2);

  lay->fill_rect(x1, y1, x2, y2);

  return JS_TRUE;
}
  
JS(vector_layer_fill) {
  func("%s",__FUNCTION__);
  GET_LAYER(CairoLayer);

  if(argc>0)
    lay->set_color(cx, argc, argv, 0);

  lay->fill();
  return JS_TRUE;
}
JS(vector_layer_stroke) {
  func("%s",__FUNCTION__);
  GET_LAYER(CairoLayer);  

  if(argc>0) {
    lay->set_color(cx, argc, argv, 0);
  }

  lay->stroke();
  return JS_TRUE;
}


JSP(vector_layer_fillstyle_g) {
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  //  GET_LAYER(CairoLayer);

  return JS_TRUE;
}
JSP(vector_layer_fillstyle_s) {
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  GET_LAYER(CairoLayer);

  // check if this makes sense
  cairo_set_fill_rule(lay->cairo, (cairo_fill_rule_t)*vp);

  // if( JSVAL_IS_NUM(*vp) )
  //   func("FILLSTYLE set is NUMBER");

  // if( JSVAL_IS_OBJECT(*vp) )
  //   func("FILLSTYLE set is OBJECT");

  // if( JS_IsArrayObject(cx, JSVAL_TO_OBJECT(*vp) ) )
  //   func("FILLSTYLE set is ARRAY");

  return JS_TRUE;
}

JSP(vector_layer_strokestyle_g) { 
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  return JS_TRUE;
}
JSP(vector_layer_strokestyle_s) { 
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  //  GET_LAYER(CairoLayer);  

  //  JS_PROP_NUMBER(num, vp);
  //  func("num is %f", num);

  // if( JSVAL_IS_OBJECT(*vp) )
  //   func("FILLSTYLE set is OBJECT");
  
  // if( JS_IsArrayObject(cx, JSVAL_TO_OBJECT(*vp) ) )
  //   func("FILLSTYLE set is ARRAY");
  
  return JS_TRUE;
}

JSP(vector_layer_linecap_g)     {
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  return JS_TRUE;
}
JSP(vector_layer_linecap_s)     {
  //  js_debug_property(cx, vp);

  GET_LAYER(CairoLayer);  

  char *cap = NULL;
  if(JSVAL_IS_STRING(*vp))
    cap = JS_GetStringBytes( JS_ValueToString(cx, *vp) );
  else {
    JS_ReportError(cx,"%s: property value is not a string",__FUNCTION__);
    ::error("%s: property value is not a string",__FUNCTION__);
  }

  func("Vector linecap set :: %s",cap);
  if(!cap) return JS_TRUE; // we don't stop the flow

  switch(cap[0]) { // we parse fast, using only first letter
    // [b]utt, [r]ound, [s]quare
  case 'b':
    cairo_set_line_cap(lay->cairo, CAIRO_LINE_CAP_BUTT);
    break;
  case 'r':
    cairo_set_line_cap(lay->cairo, CAIRO_LINE_CAP_ROUND);
    break;
  case 's':
    cairo_set_line_cap(lay->cairo, CAIRO_LINE_CAP_SQUARE);
    break;
  default:
    error("VectorLayer line cap not supported: %s", cap);
    error("use: butt, round or square");
    break;
  }
  return JS_TRUE;
}

JSP(vector_layer_linewidth_g)   { 
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  GET_LAYER(CairoLayer);  

  func("vp is %p : %f",vp, *vp);

  return JS_TRUE; }

JSP(vector_layer_linewidth_s)   {
  func("%s",__FUNCTION__);
  //  js_debug_property(cx, vp);

  GET_LAYER(CairoLayer);  

  JS_PROP_DOUBLE(wid, *vp);

  lay->set_line_width(wid);

  return JS_TRUE;
}

#endif
