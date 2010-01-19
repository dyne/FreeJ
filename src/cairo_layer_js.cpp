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


static inline void js_debug_property(JSContext *cx, jsval *vp) {
  func(" vp mem address %p", vp);
  int tag = JSVAL_TAG(*vp);
  func(" type tag is %i: %s",tag,
       (tag==0x0)?"object":
       (tag==0x1)?"integer":
       (tag==0x2)?"double":
       (tag==0x4)?"string":
       (tag==0x6)?"boolean":
       "unknown");

  switch(tag) {
  case 0x0:
    {
      JSObject *obj = JSVAL_TO_OBJECT(*vp);
      jsval val;
      if( JS_IsArrayObject(cx, obj) ) {
	jsuint len; JS_GetArrayLength(cx, obj, &len);
	func(" object is an array of %u elements", len);
	for(jsuint c = 0; c<len; c++) {
	  func(" dumping element %u:",c);
	  JS_GetElement(cx, obj, c, &val);
	  if(val == JSVAL_VOID)
	    func(" content is VOID");
	  else
	    js_debug_property(cx, &val);
	}
      } else {
	func(" object type is unknown to us (not an array?)");
      }
    }
    break;
  case 0x1:
    {
      JS_PROP_INT(num, vp);
      func("  Sint[ %i ] Uint[ %u ]",
	   num, num);
    }
    break;

  case 0x2:
    {
      JS_PROP_DOUBLE(num, vp);
      func("  double is %.4f",num);
    }
    break;
    
  case 0x4:
    {
      char *cap = NULL;
      JS_PROP_STRING(cap);
      func("  string is \"%s\"",cap);
    }
    break;

  case 0x6:
    {
      bool b = false;
      b = JSVAL_TO_BOOLEAN(*vp);
      func("  boolean is %i",b);
    }
    break;

  default:
    func(" tag %u is unhandled, probably double");
    JS_PROP_DOUBLE(num, vp);
    func("  Double [ %.4f ] - Sint[ %i ] - Uint[ %u ]",
	 num, num, num);
  }
}

JSBool CairoLayer::set_color(JSContext *cx, uintN argc, jsval *argv, int idx) {

  if(argc==1) {

    JS_ARG_DOUBLE(g, idx);
    func("%s gray [%.2f]", __FUNCTION__, g);
    js_debug_property(cx, &argv[idx]);
    color->set_gray(g);
    return JS_TRUE;

  } else if(argc==2) {

    JS_ARG_DOUBLE(g, idx);
    JS_ARG_DOUBLE(a, idx+1);

    // g is a double
    // a is a signed integer
    func("%s gray [%i] alpha [%i]", __FUNCTION__, g, a);
    //    js_debug_property(cx, &argv[idx]);
    //    js_debug_property(cx, &argv[idx+1]);

    color->set_gray_alpha(g,a);
    return JS_TRUE;

  } else if(argc==3) {
    JS_ARG_DOUBLE(r, idx);
    JS_ARG_DOUBLE(g, idx+1);
    JS_ARG_DOUBLE(b, idx+2);

    func("%s r[%i] g[%i] b[%i]", __FUNCTION__, r, g, b);
    //    js_debug_property(cx, &argv[idx]);
    //    js_debug_property(cx, &argv[idx+1]);
    //    js_debug_property(cx, &argv[idx+2]);

    color->set_rgb(r, g, b);
    return JS_TRUE;

  } else if(argc==4) {

    JS_ARG_DOUBLE(r, idx);
    JS_ARG_DOUBLE(g, idx+1);
    JS_ARG_DOUBLE(b, idx+2);
    JS_ARG_DOUBLE(a, idx+3);

    func("%s r[%i] g[%i] b[%i] a[%i]", __FUNCTION__, r, g, b, a);
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

  JS_ARG_INT(tx, 0);
  JS_ARG_INT(ty, 1);

  func("%s x[%i] y[%i]", __FUNCTION__, tx, ty);

  lay->translate(tx, ty);

  return JS_TRUE;
}

JS(vector_layer_scale) {
  JS_CHECK_ARGC(2);

  GET_LAYER(CairoLayer);

  JS_ARG_DOUBLE(sx, 0);
  JS_ARG_DOUBLE(sy, 1);

  func("%s x[%.2f] y[%.2f]", __FUNCTION__, sx, sy );

  lay->scale(sx, sy);

  return JS_TRUE;

}

JS(vector_layer_rotate) {
  JS_CHECK_ARGC(1);

  GET_LAYER(CairoLayer);

  JS_ARG_DOUBLE(angle, 0);

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

  JS_ARG_INT(x, 0);
  JS_ARG_INT(y, 1);

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

  JS_ARG_INT(x, 0);
  JS_ARG_INT(y, 1);

  func("%s x[%i] y[%i]", __FUNCTION__, x, y  );

  lay->move_to(x, y);

  return JS_TRUE;
}

JS(vector_layer_quadcurveto) {

  JS_CHECK_ARGC(4);
  GET_LAYER(CairoLayer);

  JS_ARG_INT(x1, 0);
  JS_ARG_INT(y1, 1);
  
  JS_ARG_INT(x2, 2);
  JS_ARG_INT(y2, 3);
  
  lay->quad_curve_to( x1, y1, x2, y2) ;

  return JS_TRUE;
}

JS(vector_layer_beziercurveto) {

  JS_CHECK_ARGC(6);

  GET_LAYER(CairoLayer);

  JS_ARG_DOUBLE(x1, 0);
  JS_ARG_DOUBLE(y1, 1);

  JS_ARG_DOUBLE(x2, 2);
  JS_ARG_DOUBLE(y2, 3);

  JS_ARG_DOUBLE(x3, 4);
  JS_ARG_DOUBLE(y3, 5);
  
  func("Vector bezier curve :: x1[%.2f] y1[%.2f] x2[%.2f] y2[%.2f] x3[%.2f] y3[%.2f]",
       x1, y1, x2, y2, x3, y3 );
  
  lay->curve_to(x1, y1, x2, y2, x3, y3);

  return JS_TRUE;
}

JS(vector_layer_arc) {

  JS_CHECK_ARGC(5);

  GET_LAYER(CairoLayer);

  JS_ARG_INT(xc, 0);
  JS_ARG_INT(yc, 1);

  JS_ARG_DOUBLE(radius, 2);

  JS_ARG_DOUBLE(angle1, 3);
  JS_ARG_DOUBLE(angle2, 4);

  func("Vector arc :: x[%i] y[%i] rad[%.2f] angle1[%.2f] angle2[%.2f]",
       xc, yc, radius, angle1, angle2 );
  
  lay->arc(xc, yc, radius, angle1, angle2);

  return JS_TRUE;
}

JS(vector_layer_fillrect) {

  JS_CHECK_ARGC(4);

  GET_LAYER(CairoLayer);  

  JS_ARG_INT(x1, 0);
  JS_ARG_INT(y1, 1);

  JS_ARG_INT(x2, 2);
  JS_ARG_INT(y2, 3);

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
  JS_PROP_STRING(cap);

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

  JS_PROP_DOUBLE(wid, vp);

  lay->set_line_width(wid);

  return JS_TRUE;
}

#endif
