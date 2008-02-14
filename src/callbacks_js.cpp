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
#include <jsapi.h> // spidermonkey header

// include exception messags
#include <callbacks_js.h>

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


/* we declare the Context pointer static here
   in order to have it accessed from callback functions
   which are not class methods */
Context *env;
bool stop_script;

void js_sigint_handler(int sig) {
    stop_script=true;
}


JSBool js_static_branch_callback(JSContext* Context, JSScript* Script) {
    if(stop_script) {
	stop_script=false;
	return JS_FALSE;
    }
    return JS_TRUE;
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
    if (!v || !JSVAL_IS_OBJECT(v)) {
        JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
           JSSMSG_FJ_WICKED , caller, "argument is not an object"
        );
        JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
           JSSMSG_FJ_WICKED , caller, "argument is not an object"
        );
        return JS_FALSE;
    }
    JSObject *obj = JSVAL_TO_OBJECT(v);
    while ((obj = OBJ_GET_PROTO(cx, obj)) != NULL) {
        if (OBJ_GET_CLASS(cx, obj) == clasp)
            return JS_TRUE;
    }
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
        JSSMSG_FJ_WRONGTYPE, caller,
        OBJ_GET_CLASS(cx, JSVAL_TO_OBJECT(v))->name,
        clasp->name
    );
    return JS_FALSE;
}


