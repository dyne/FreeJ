/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
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
#include <config.h>
#include <generator_layer.h>


DECLARE_CLASS_GC("GeneratorLayer",
		 generator_layer_class,
		 generator_layer_constructor,
		 js_layer_gc);


JSFunctionSpec generator_layer_methods[] = {
  ENTRY_METHODS,
  {0}
};


JS(generator_layer_constructor) {
  func("%s",__PRETTY_FUNCTION__);
  GeneratorLayer *layer = NULL;
  char excp_msg[MAX_ERR_MSG + 1];                                           
  layer = new GeneratorLayer();
  if(!layer) {
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
                         JSSMSG_FJ_CANT_CREATE, __func__,
			 "cannot create GeneratorLayer");
    return JS_FALSE;
  }
  layer->register_generators(&global_environment->generators);
  rval = (jsval*)layer->js_constructor(global_environment,
				       cx, obj, argc, argv, excp_msg);
  if(!rval) {
    delete layer;
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
                         JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
    return JS_FALSE;
  }
  layer->data = (void*)rval;
  return JS_TRUE;							     
}
  

