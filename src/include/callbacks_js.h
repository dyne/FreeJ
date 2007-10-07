/*  FreeJ javascript application protocol interface
 *
 *  Copyright (C) 2004-2006
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
#include <jsnum.h>
#include <jscntxt.h>

// stuff for exception handling "try{} catch(e) {}"
#define MAX_ERR_MSG 1024
typedef enum JSExnType {
    JSEXN_NONE = -1,
        JSEXN_ERR,
        JSEXN_INTERNALERR,
        JSEXN_EVALERR,
        JSEXN_RANGEERR,
        JSEXN_REFERENCEERR,
        JSEXN_SYNTAXERR,
        JSEXN_TYPEERR,
        JSEXN_URIERR,
        JSEXN_LIMIT
} JSExnType;
// FIXME:
// I don't know how to override this ... we don't get our <EXCEPTION_NAME> from jsfreej.msg
static JSExnType errorToExceptionNum[] = {
#define MSG_DEF(name, number, count, exception, format) \
    exception,
#include "jsfreej.msg"
#undef MSG_DEF
};

typedef enum JSFreejErrNum {
#define MSG_DEF(name, number, count, exception, format) \
    name = number,
#include "jsfreej.msg"
#undef MSG_DEF
    JSFreejErr_Limit
#undef MSGDEF
} JSFreejErrNum;

const JSErrorFormatString * JSFreej_GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber);
// exception stuff end

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
    error("%s: argument %u is not a number",__FUNCTION__,argnum); \
    return JS_FALSE; \
  }

#define JS_ARG_STRING(variable,argnum) \
  if(JSVAL_IS_STRING(argv[argnum])) \
    variable = JS_GetStringBytes \
      ( JS_ValueToString(cx, argv[argnum]) ); \
  else { \
    JS_ReportError(cx,"%s: argument %u is not a string",__FUNCTION__,argnum); \
    error("%s: argument %u is not a string",__FUNCTION__,argnum); \
    return JS_FALSE; \
  }

// set the return value as a string
#define JS_RETURN_STRING(cb_msg) \
  JSString *cb_str = JS_NewStringCopyZ(cx, cb_msg); \
  *rval = STRING_TO_JSVAL(cb_str)

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
JS(constructor_func) {                                                        \
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);                            \
  constructor_class *layer;                                                   \
  char *filename;                                                             \
  char excp_msg[MAX_ERR_MSG + 1];                                             \
  uint16_t width  = env->screen->w;                                           \
  uint16_t height = env->screen->h;                                           \
                                                                              \
  layer = new constructor_class();                                            \
  if(!layer) {                                                                \
    sprintf(excp_msg, "cannot create constructor_class");                     \
    goto error;                                                               \
  }                                                                           \
  if(argc==0) {                                                               \
    if(!layer->init(env)) {                                                   \
      sprintf(excp_msg, "failed init(env)");                                  \
      goto error;                                                             \
    }                                                                         \
  } else if(argc==1) {                                                        \
    JS_ARG_STRING(filename,0);                                                \
    if(!layer->init(env)) {                                                   \
      sprintf(excp_msg, "failed init(env)");                                  \
      goto error;                                                             \
    }                                                                         \
    if(!layer->open(filename)) {                                              \
      snprintf(excp_msg, MAX_ERR_MSG, "failed open(%s): %s", filename, strerror(errno));    \
      goto error;                                                             \
    }                                                                         \
  } else if(argc==2) {                                                        \
    js_ValueToUint16(cx, argv[0], &width);                                    \
    js_ValueToUint16(cx, argv[1], &height);                                   \
    if(!layer->init(env, width, height)) {                                    \
      snprintf(excp_msg, MAX_ERR_MSG, "failed init(env, %u, %u)", width, height); \
      goto error;                                                             \
    }                                                                         \
  } else if(argc==3) {                                                        \
    js_ValueToUint16(cx, argv[0], &width);                                    \
    js_ValueToUint16(cx, argv[1], &height);                                   \
    JS_ARG_STRING(filename,2);                                                \
    if(!layer->init(env, width, height)) {                                    \
      snprintf(excp_msg, MAX_ERR_MSG, "failed init(env, %u, %u)", width, height); \
      goto error;                                                             \
    }                                                                         \
    if(!layer->open(filename)) {                                              \
      snprintf(excp_msg, MAX_ERR_MSG, "failed open(%s): %s", filename, strerror(errno)); \
      goto error;                                                             \
    }                                                                         \
  } else {                                                                    \
    sprintf(excp_msg, "Wrong numbers of arguments\n use (\"filename\") or (width, height, \"filename\") or ()");       \
    goto error;                                                               \
  }                                                                           \
  if(!JS_SetPrivate(cx,obj,(void*)layer)) {                                   \
    sprintf(excp_msg, "%s", "JS_SetPrivate failed");                          \
    goto error;                                                               \
  }                                                                           \
  *rval = OBJECT_TO_JSVAL(obj);                                               \
  return JS_TRUE;                                                             \
                                                                              \
error:                                                                        \
    func("a");if(layer) delete layer;                                         \
    func("b");JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,         \
              JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);                     \
    func("c");cx->newborn[GCX_OBJECT] = NULL;                                 \
    func("d");return JS_FALSE;                                                \
}                                                                             \


// Gets a pointer to the layer from the private object of javascript
// it can be then referenced as *lay
#define GET_LAYER(layer_class) \
layer_class *lay = (layer_class *) JS_GetPrivate(cx,obj); \
if(!lay) { \
  error("%u:%s:%s :: Layer core data is NULL", \
	__LINE__,__FILE__,__FUNCTION__); \
  return JS_FALSE; \
}

#define JS_ERROR(str) { \
  JS_ReportError(cx,"%s: %s",__FUNCTION__,str); \
  env->quit = true; \
  return JS_FALSE; \
}


// check if a function of name exists for the current js class
#define CHECKSYM(key,name) \
  if(keysym->sym == key) { \
    strcat(funcname,name); \
    func("keyboard controller calling method %s()",funcname); \
    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret); \
    return 1; \
  }


 
extern Context *env;
extern bool stop_script;

void js_sigint_handler(int sig);
JSBool js_static_branch_callback(JSContext* Context, JSScript* Script);
void js_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report);

#endif
