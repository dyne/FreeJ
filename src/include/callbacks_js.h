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
 * "$Id: jsparser.h 652 2005-08-17 19:52:40Z jaromil $"
 *
 */

#ifndef __CALLBACKS_JS_H__
#define __CALLBACKS_JS_H__


#include <config.h>
#include <context.h>
#include <jsapi.h> // spidermonkey header

/*
 * Tune this to avoid wasting space for shallow stacks, while saving on
 * malloc overhead/fragmentation for deep or highly-variable stacks. */
#define STACK_CHUNK_SIZE    8192

#define JS(fun) \
JSBool fun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

#define JS_CHECK_ARGC(num) \
  if(argc<num) { \
    error("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__); \
    error("not enough arguments: minimum %u needed",num); \
    return(JS_FALSE); \
  }

// cast a numerical value in a double variable
#define JS_ARG_NUMBER(variable,argnum) \
  double variable; \
  if(JSVAL_IS_DOUBLE(argv[argnum])) { \
    variable = *JSVAL_TO_DOUBLE(argv[argnum]); \
  } else if(JSVAL_IS_INT(argv[argnum])) { \
    variable = (double)JSVAL_TO_INT(argv[argnum]); \
  } else if(JSVAL_IS_BOOLEAN(argv[argnum])) { \
    variable = (double)JSVAL_TO_BOOLEAN(argv[argnum]); \
  } else { \
    JS_ReportError(cx,"%s: argument %u is not a number",__FUNCTION__,argnum); \
    env->quit = true; \
    return JS_FALSE; \
  }

#define JS_ARG_STRING(variable,argnum) \
  if(JSVAL_IS_STRING(argv[argnum])) \
    variable = JS_GetStringBytes \
      ( JS_ValueToString(cx, argv[argnum]) ); \
  else { \
    JS_ReportError(cx,"%s: argument %u is not a string",__FUNCTION__,argnum); \
    env->quit = true; \
    return JS_FALSE; \
  }

#define DECLARE_CLASS(class_name, class_struct, class_constructor) \
JSClass class_struct = { \
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
  int width, height; \
  layer = new constructor_class(); \
  if(!layer) { \
    error("JS::%s : cannot create constructor_class", constructor_name); \
    return JS_FALSE; \
  } \
  if(argc>1) { \
    width = JSVAL_TO_INT(argv[0]); \
    height = JSVAL_TO_INT(argv[1]); \
  } else { \
    width = env->screen->w; \
    height = env->screen->h; \
  } \
  if(argc > 2) {\
    filename = JS_GetStringBytes(JS_ValueToString(cx,argv[2])); \
    if(!layer->open(filename)) { \
      error("JS::%s : can't open file %s",constructor_name, filename); \
      delete layer; return JS_TRUE; \
    } \
  } \
  layer->init(width, height); \
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


//  error("%u:%s:%s : %s",__LINE__,__FILE__,__FUNCTION__,str);


#define JS_ERROR(str) { \
  JS_ReportError(cx,"%s: %s",__FUNCTION__,str); \
  env->quit = true; \
  return JS_FALSE; \
}



 
extern Context *env;
extern bool stop_script;

void js_sigint_handler(int sig);
JSBool js_static_branch_callback(JSContext* Context, JSScript* Script);
void js_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report);

#endif
