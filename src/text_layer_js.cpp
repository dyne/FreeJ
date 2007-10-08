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
#include <text_layer.h>

DECLARE_CLASS("TextLayer",txt_layer_class,txt_layer_constructor);


////////////////////////////////
// Txt Layer methods
JSFunctionSpec txt_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //   name            native                  nargs
  {    "print",        txt_layer_print,        1},
  {    "color",        txt_layer_color,        3},
  {    "font",         txt_layer_font,         1},
  {    "size",         txt_layer_size,         1},
  {    "calculate_size", txt_layer_calculate_size, 1},
  {0}
};


JS_CONSTRUCTOR("TextLayer",txt_layer_constructor,TTFLayer);

JS(txt_layer_color) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  JS_CHECK_ARGC(1);

  GET_LAYER(TTFLayer);

  // color accepts arguments in many ways
  // R,G,B,A or R,G,B or the whole 32bit value
  //  uint32_t r,g,b,a;
  
  if(JSVAL_IS_DOUBLE(argv[0])) {
    
    double *hex;
    hex = JSVAL_TO_DOUBLE(argv[0]);
    
    warning("TODO: assign colors to text layer in hex form");
    //    lay->color = (uint32_t)*hex;
    
  } else {
    
    lay->fgcolor.r = JSVAL_TO_INT(argv[0]);
    lay->fgcolor.g = JSVAL_TO_INT(argv[1]);
    lay->fgcolor.b = JSVAL_TO_INT(argv[2]);
    
    //    lay->color = 0x0|(r<<8)|(g<<16)|(b<<24);
  }
  
  return JS_TRUE;
}

JS(txt_layer_print) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  JS_CHECK_ARGC(1);

  GET_LAYER(TTFLayer);

  char *str;
  JS_ARG_STRING(str, 0);

  lay->print(str);

  return JS_TRUE;
}
JS(txt_layer_size) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TTFLayer);

  JS_ARG_NUMBER(size,0);

  lay->size = (int)size;

  return JS_TRUE;
}
JS(txt_layer_font) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TTFLayer);

  char *font;
  JS_ARG_STRING(font,0);

  // try full path to .ttf file
  lay->font = TTF_OpenFont(font, lay->size);
  if (!lay->font) {
    error("Couldn't load font %s: %s\n", font, SDL_GetError());
    *rval = JSVAL_FALSE;
  } else
    *rval = JSVAL_TRUE;

  return JS_TRUE;
}

JS(txt_layer_calculate_size) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TTFLayer);

  int w, h;
  char *text;
  JSObject *arr;
  jsdouble num;
  jsval val;

  JS_ARG_STRING(text,0);
  
  lay->calculate_string_size(text, &w, &h);

  arr = JS_NewArrayObject(cx, 0, NULL); // create a void array
  if(!arr) { error("error generating array"); return JS_FALSE; }
  
  // fill the array with [0]w [1]h
  num = JS_NewNumberValue(cx, (double)w, &val);
  JS_SetElement(cx, arr, 0, &val);
  num = JS_NewNumberValue(cx, (double)h, &val);
  JS_SetElement(cx, arr, 1, &val);
    
  *rval = OBJECT_TO_JSVAL( arr);
  return JS_TRUE;
}

#endif
