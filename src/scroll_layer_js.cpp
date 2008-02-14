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
#include <scroll_layer.h>

DECLARE_CLASS_GC("VScrollLayer",vscroll_layer_class,vscroll_layer_constructor,js_layer_gc);

////////////////////////////////
// VScroll Layer methods
JSFunctionSpec vscroll_layer_methods[] = {
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "append",       vscroll_layer_append,   1},
  {     "kerning",      vscroll_layer_append,   1},
  {     "linespace",    vscroll_layer_linespace,1},
  {     "speed",        vscroll_layer_speed,    1}, 
  {0}
};

JS_CONSTRUCTOR("VScrollLayer",vscroll_layer_constructor,ScrollLayer);
JS(vscroll_layer_append) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  char *str = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!str) {
    error("JsParser :: invalid string in VScrollLayer::append");
    return JS_FALSE;
  }
  lay->append(str);

  return JS_TRUE;
}
JS(vscroll_layer_speed) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int s = JSVAL_TO_INT(argv[0]);
  lay->step = s;

  return JS_TRUE;
}
JS(vscroll_layer_linespace) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int l = JSVAL_TO_INT(argv[0]);
  lay->line_space = l;

  return JS_TRUE;
}
JS(vscroll_layer_kerning) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int k = JSVAL_TO_INT(argv[0]);
  lay->kerning = k;

  return JS_TRUE;
}

