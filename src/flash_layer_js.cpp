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
#include <flash_layer.h>

DECLARE_CLASS_GC("FlashLayer", flash_layer_class, flash_layer_constructor, js_layer_gc);

/////////////////////////////////
// Flash Layer methods
JSFunctionSpec flash_layer_methods[] = {
  ENTRY_METHODS ,
  {     "open",          flash_layer_open,             1},
  {0}
};

JS_CONSTRUCTOR("FlashLayer", flash_layer_constructor, FlashLayer);

JS(flash_layer_open) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  if(argc<1) return JS_FALSE;
  
  GET_LAYER(FlashLayer);
    
  char *file = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!file) {
    error("JsParser :: invalid string in FlashLayer::open");
    return JS_FALSE;
  }
  
  if(! lay->open(file) ) {
    error("can't open %s in layer %s",file, lay->name);
    return JS_FALSE;
  }

  return JS_TRUE;
}
