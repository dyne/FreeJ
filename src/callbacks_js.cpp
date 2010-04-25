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
 * "$Id: jsparser.cpp 658 2005-08-19 11:59:42Z jaromil $"
 *
 */

#include <signal.h>
#include <config.h>
#include <jutils.h>
#include <errno.h>
#include <math.h>

#include <jsapi.h> // spidermonkey header

// include exception messags
#include <callbacks_js.h>

#include <jsparser.h>

JSErrorFormatString jsFreej_ErrorFormatString[JSFreejErr_Limit] = {
#if JS_HAS_DFLT_MSG_STRINGS
#define MSG_DEF(name, number, count, exception, format) \
    { format, count } ,
#else
#define MSG_DEF(name, number, count, exception, format) \
    { NULL, count } ,
#endif
#include "jsfreej.msg"
#undef MSG_DEF
};

const JSErrorFormatString *
JSFreej_GetErrorMessage(void *userRef, const char *locale, const uintN errorNumber)
{
    if ((errorNumber > 0) && (errorNumber < JSFreejErr_Limit))
        return &jsFreej_ErrorFormatString[errorNumber];
	else
	    return NULL;
}

#define CHECK_JSENV() \
  if(!global_environment) { \
    error("%s : can't find freej context", __FUNCTION__); \
    return 0; \
  } \
  if(!global_environment->js) { \
    error("%s : freej context without a javascript parser", __FUNCTION__); \
    return 0; \
  } \
  if(!global_environment->js->global_context) { \
    error("%s : called on a NULL javascript context", __FUNCTION__); \
    return 0; \
  }

/* we declare the Context pointer static here
   in order to have it accessed from callback functions
   which are not class methods */
//Context *env;
bool stop_script;

void js_sigint_handler(int sig) {
    stop_script=true;
}

#if defined JSOPTION_NATIVE_BRANCH_CALLBACK
JSBool js_static_branch_callback(JSContext* Context, JSScript* Script)
#else
JSBool js_static_branch_callback(JSContext* Context)
#endif
{
  if(stop_script) {
	stop_script=false;
	return JS_FALSE;
  }
  return JS_TRUE;
}

char *js_get_string(jsval val) {
  char *res = NULL;
  JS_BeginRequest(cx);
  if(JSVAL_IS_STRING(val))
    res = JS_GetStringBytes( JS_ValueToString(global_environment->js->global_context, val) );
  else {
    JS_ReportError(global_environment->js->global_context,"argument is not a string");
    ::error("argument is not a string");
  }
  JS_EndRequest(cx);
  return res;
}


jsint js_get_int(jsval val) {
  JS_BeginRequest(cx);
  CHECK_JSENV();

  int32 res = 0;

  int tag = JSVAL_TAG(val);
  switch(tag) {
  case 0x0:
    error("argument is a JS Object, should be integer");
    break;

  case 0x1:
    JS_ValueToInt32(global_environment->js->global_context, val, &res);
    //    func("argument is an integer as expected");
    break;

  case 0x2:
    JS_ValueToInt32(global_environment->js->global_context, val, &res);
    warning("argument is a double, but should be int, got value %i", res);
    break;
    
  case 0x4:
    error("argument is a string, should be integer");
    break;

  case 0x6:
    error("argument is a boolean, shoul be integer");
    break;

  default:
    if(!val) {
      warning("argument is NULL");
      break;
    }
    {
      jsdouble tmp;
      if( !JS_ValueToNumber(global_environment->js->global_context, val, &tmp) ) {
	error("argument is of unknown type, cannot interpret");
      } else res = (int32)tmp;
    }
      // else {
    //   res = (int)floor( *tmp );
    //   warning("argument %p is of unknown type, but should be int, got value %i",val, res);
    // }

    // JS_ReportErrorNumber( global_environment->js->global_context, JSFreej_GetErrorMessage, NULL,
    // 			 JSSMSG_FJ_WICKED,__FUNCTION__, "invalid value");

    //    res = *JSVAL_TO_DOUBLE(*val);
    //    func("argument %p is %.4f",val, res);
    break;
  }
  JS_EndRequest(cx);
  return res;
}

jsdouble js_get_double(jsval val) {

  JS_BeginRequest(cx);
  CHECK_JSENV();

  jsdouble res = 0.0;

  int tag = JSVAL_TAG(val);
  switch(tag) {
  case 0x0:
    error("argument is a JS Object, should be double");
    break;

  case 0x1:
    JS_ValueToNumber(global_environment->js->global_context, val, &res);
    warning("argument is an integer, but should be double, got value %.4f", res);
    break;

  case 0x2:
    JS_ValueToNumber(global_environment->js->global_context, val, &res);
    //    func("argument is a double as expected");
    break;
    
  case 0x4:
    error("argument is a string, should be double");
    break;

  case 0x6:
    error("argument is a boolean, shoul be double");
    break;

  default:
    if(!val) {
      warning("argument is NULL");
      break;
    }
    {
      jsdouble tmp;
      if( ! JS_ValueToNumber(global_environment->js->global_context, val, &tmp) ) {
	error("argument is of unknown type, cannot interpret");
      } else res = tmp;
    }

    // jsdouble tmp;
    // JS_ValueToNumber(global_environment->js->global_context, *val, &tmp);
    // if(!tmp) {
    //   warning("argument %p is of unknown type, got null",val);
    // }
    // else {
    //   res = *tmp;
    //   warning("argument %p is of unknown type, but should be int, got value %.4f",val, res);
    // }
    // JS_ReportErrorNumber( global_environment->js->global_context, JSFreej_GetErrorMessage, NULL,
    // 			 JSSMSG_FJ_WICKED,__FUNCTION__, "invalid value");

    //    res = *JSVAL_TO_DOUBLE(*val);
    //    func("argument %p is %.4f",val, res);
    break;
  }
  JS_EndRequest(cx);
  return res;
}

void js_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report) {
::func("JS Error Reporter called");
  if(Report->filename)
    ::error("script error in %s:%i flag: %i",Report->filename, Report->lineno + 1, Report->flags);
  else
    ::error("script error %i  flags: %i while parsing", Report->errorNumber, Report->flags);

  // this doesn't prints out the line reporting error :/
  if(Report->linebuf)
    ::error("%u: %s",(uint32_t)Report->lineno, Report->linebuf);

  if(Message) ::error("JS Error Message: %s flag: %i",(char *)Message, Report->flags);
}

JSBool _js_is_instanceOf(JSContext* cx, JSClass* clasp, jsval v, const char* caller) {
	JSBool ret = JS_FALSE;
	JS_BeginRequest(cx);
    if (!v || !JSVAL_IS_OBJECT(v)) {
        JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
           JSSMSG_FJ_WICKED , caller, "argument is not an object"
        );
        JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
           JSSMSG_FJ_WICKED , caller, "argument is not an object"
        );
		JS_EndRequest(cx);
        return JS_FALSE;
    }
	JSObject *obj = JSVAL_TO_OBJECT(v);

	ret = JS_InstanceOf(cx, obj, clasp, NULL);

	JS_EndRequest(cx);
    return ret;
}

