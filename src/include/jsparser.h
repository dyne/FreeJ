/*  FreeJ
 *
 *  Copyright (C) 2004
 *  Silvano Galliani aka kysucix <kysucix@dyne.org>
 *  Denis Rojo aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#ifndef __JSPARSER_H__
#define __JSPARSER_H__

#include <config.h>
#ifdef WITH_JAVASCRIPT

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks. */
#define STACK_CHUNK_SIZE    8192

#include <context.h>
#include <jsapi.h> // spidermonkey header
#include <layer.h>

#define JS(fun) \
JSBool fun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define DECLARE_CLASS(class_name, class_struct, class_constructor) \
static JSClass class_struct = { \
  class_name, JSCLASS_HAS_PRIVATE, \
  JS_PropertyStub,  JS_PropertyStub, \
  JS_PropertyStub,  JS_PropertyStub, \
  JS_EnumerateStub, JS_ResolveStub, \
  JS_ConvertStub,   JS_FinalizeStub, \
  NULL,   NULL, \
  class_constructor \
}

#define REGISTER_CLASS(class_name, class_struct, class_constructor, class_methods) \
    layer_object = JS_InitClass(js_context, global_object, NULL, \
				&class_struct, class_constructor, \
				0, 0, 0, 0, 0); \
    if(!layer_object) { \
        error("JsParser::init() can't instantiate %s class",class_name); \
    } else { \
      ret = JS_DefineFunctions(js_context, layer_object, class_methods); \
      if(ret != JS_TRUE) { \
	error("JsParser::init() can't register %s methods", class_name); \
      } \
    }

#define JS_CONSTRUCTOR(constructor_name, constructor_func, constructor_class) \
JS(constructor_func) { \
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__); \
  constructor_class *layer; \
  char *filename; \
  layer = new constructor_class(); \
  if(!layer) \
    return JS_FALSE; \
  if(argc > 0) {\
    filename = JS_GetStringBytes(JS_ValueToString(cx,argv[0])); \
    if(!layer->open(filename)) { \
      error("JS::%s : can't open file %s",constructor_name, filename); \
      delete layer; return JS_FALSE; \
    } \
  } \
  if(!JS_SetPrivate(cx,obj,(void*)layer)) { \
    error("JS::%s : can't set the private value"); \
    delete layer; return JS_FALSE; \
  } \
  *rval = OBJECT_TO_JSVAL(obj); \
  return JS_TRUE; \
}

// Gets a pointer to the layer from the private object of javascript
// it can be then referenced as *lay
#define GET_LAYER(layer_class) \
layer_class *lay = (layer_class *) JS_GetPrivate(cx,obj); \
if(!lay) { \
  error("%u:%s:%s :: Layer core data is NULL", \
	__LINE__,__FILE__,__FUNCTION__); \
  return JS_FALSE; \
}

class JsParser {
    public:
	JsParser(Context *_env);
	~JsParser();
	int open(const char* script_file);
	int parse(const char *command);
    private:
	JSRuntime *js_runtime;
	JSContext *js_context;
	JSObject *global_object;
	JSObject *layer_object;
	void init();

	JSPropertySpec layer_properties[3];

	int parse_count;
	//	JSFunctionSpec shell_functions[3];

};
#endif

#endif
