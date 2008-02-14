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

/* // stuff for exception handling "try{} catch(e) {}" */
/* typedef enum JSExnType { */
/*     JSEXN_NONE = -1, */
/*         JSEXN_ERR, */
/*         JSEXN_INTERNALERR, */
/*         JSEXN_EVALERR, */
/*         JSEXN_RANGEERR, */
/*         JSEXN_REFERENCEERR, */
/*         JSEXN_SYNTAXERR, */
/*         JSEXN_TYPEERR, */
/*         JSEXN_URIERR, */
/*         JSEXN_LIMIT */
/* } JSExnType; */
/* FIXME:
I don't know how to override which JSException type we want ... we don't get ours from jsfreej.msg api still uses js.msg :(
static JSExnType errorToExceptionNum[] = {
#define MSG_DEF(name, number, count, exception, format) \
    exception,
#include "jsfreej.msg"
#undef MSG_DEF
};
*/
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
    ::error("%s: argument %u is not a number",__FUNCTION__,argnum);	\
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

#define DECLARE_CLASS_GC(class_name, class_struct, class_constructor, gc) \
JSClass class_struct = { \
  class_name, JSCLASS_HAS_PRIVATE, \
  JS_PropertyStub,  JS_PropertyStub, \
  JS_PropertyStub,  JS_PropertyStub, \
  JS_EnumerateStub, JS_ResolveStub, \
  JS_ConvertStub,   gc, \
  NULL,   NULL, \
  class_constructor \
}

#define REGISTER_CLASS(class_name, class_struct, class_constructor, class_methods, parent_class) \
    layer_object = JS_InitClass(js_context, global_object, parent_class, \
				&class_struct, class_constructor, 0, \
				0, class_methods, 0, 0); \
    if(!layer_object) { \
        error("JsParser::init() can't instantiate %s class",class_name); \
    } 

#define JS_CONSTRUCTOR(constructor_name, constructor_func, constructor_class) \
JS(constructor_func) {                                                        \
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);                            \
  constructor_class *layer = NULL;					      \
  char excp_msg[MAX_ERR_MSG + 1];                                             \
  layer = new constructor_class();                                            \
  if(!layer) {                                                                \
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,                   \
                         JSSMSG_FJ_CANT_CREATE, __func__,                     \
                      "cannot create constructor_class");                     \
    return JS_FALSE;						              \
  }									      \
  rval = (jsval*)layer->js_constructor(env, cx, obj, argc, argv, excp_msg);   \
  if(!rval) {                                                                 \
    delete layer;                                                             \
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,                   \
                         JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);          \
    return JS_FALSE;                                                          \
  }                                                                           \
  layer->data = (void*)rval;                                                         \
  return JS_TRUE;							      \
}
// this was removed from the error proccedure in the macro above:
//    cx->newborn[GCX_OBJECT] = NULL;				\
// as since javascript 1.7 the newborn field of struct doesn't exists anymore
// hopefully the object is null'd and freed correctly -jrml

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
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL, \
  JSSMSG_FJ_WICKED, \
  __FUNCTION__,str); \
  return JS_FALSE; \
}

extern Context *env;
extern bool stop_script;

void js_sigint_handler(int sig);
JSBool js_static_branch_callback(JSContext* Context, JSScript* Script);
void js_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report);

// using this macro keeps the actual function name for the error messages
#define js_is_instanceOf(clasp, v) \
    if(!_js_is_instanceOf(cx, clasp, v, __func__)) \
        return JS_FALSE;

JSBool _js_is_instanceOf(JSContext*, JSClass*, jsval, const char*);
// JS Destructor

#endif
