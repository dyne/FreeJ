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
#include <layer.h>
#include <config.h>

DECLARE_CLASS("CamLayer",cam_layer_class,cam_layer_constructor);

////////////////////////////////
// CamLayer methods
JSFunctionSpec cam_layer_methods[] = {
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "open",         cam_layer_open,            1},


  //  {     "chan",         v4l_layer_chan,         1},
  //  {     "band",         v4l_layer_band,         1},
  //  {     "freq",         v4l_layer_freq,         1},
  {0}
};

JSPropertySpec cam_layer_properties[] = {
  {0}
};


JS(cam_layer_constructor) {
  func("%s",__PRETTY_FUNCTION__);
  Layer *cam = NULL;
  char *type = NULL;

  if(argc >= 1) {  
    // a specific screen type has been requested
    char *type = js_get_string(argv[0]);
    cam = Factory<Layer>::new_instance( "CamLayer", type );
  } else {
    // no screen type has been specified, return the default one
    cam = Factory<Layer>::new_instance( "CamLayer" );
  }

  JS_BeginRequest(cx);

  if(!cam) {
    error("%s: cannot open CamLayer",__FUNCTION__);
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
			 JSSMSG_FJ_CANT_CREATE, type,
			 strerror(errno));
    JS_EndRequest(cx);
    return JS_FALSE;
  }

  if (!JS_SetPrivate(cx, obj, (void *) cam))
      JS_ERROR("internal error setting private value");

  *rval = OBJECT_TO_JSVAL(obj);
  JS_EndRequest(cx);

  return JS_TRUE;
}




JS(cam_layer_open) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;
  char *type = js_get_string(argv[0]);

  GET_LAYER(Layer);

  char *file = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!file) {
    error("JsParser :: invalid string in CamLayer::open");
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
			 JSSMSG_FJ_CANT_CREATE, type,
			 strerror(errno));
    JS_EndRequest(cx);

    return JS_FALSE;
  }
  lay->open(file);

  return JS_TRUE;
}
