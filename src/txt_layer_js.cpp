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

#ifdef WITH_FT2
#include <txt_layer.h>

DECLARE_CLASS("TextLayer",txt_layer_class,txt_layer_constructor);


////////////////////////////////
// Txt Layer methods
JSFunctionSpec txt_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //     name           native                  nargs
  {      "print",       txt_layer_print,        1},
  {      "font",        txt_layer_font,         1},
  {      "size",        txt_layer_size,         1},
  {      "advance",     txt_layer_advance,      0},
  {      "blink",       txt_layer_blink,        1},
  {      "blink_on",    txt_layer_blink_on,     1},
  {      "blink_off",   txt_layer_blink_off,    1},
  {0}
};


JS_CONSTRUCTOR("TxtLayer",txt_layer_constructor,TxtLayer);



JS(txt_layer_print) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  char *str = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!str) {
    error("JsParser :: invalid string in TxtLayer::print");
    return JS_FALSE;
  }
  lay->print(str);

  return JS_TRUE;
}
JS(txt_layer_size) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  JS_ARG_NUMBER(size,0);

  lay->set_character_size((unsigned int)size);

  return JS_TRUE;
}
JS(txt_layer_font) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  JS_ARG_NUMBER(font,0);

  lay->set_font((unsigned int)font);

  return JS_TRUE;
}
JS(txt_layer_advance) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  GET_LAYER(TxtLayer);
  
  lay->advance();

  return JS_TRUE;
}
JS(txt_layer_blink) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(TxtLayer);

  if(argc<1) {
    if(!lay->blinking) {
      lay->blinking=true;
      lay->clear_screen=true;
    } else lay->blinking=false;
  } else {
    // fetch argument and switch blinking
    lay->blinking = (bool)JSVAL_TO_INT(argv[0]);
    lay->clear_screen = true;
  }

  return JS_TRUE;
}
JS(txt_layer_blink_on) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  int b = JSVAL_TO_INT(argv[0]);
  lay->onscreen_blink = b;

  return JS_TRUE;
}
JS(txt_layer_blink_off) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  int b = JSVAL_TO_INT(argv[0]);
  lay->offscreen_blink = b;

  return JS_TRUE;
}
#endif
