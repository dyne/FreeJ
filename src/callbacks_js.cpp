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
  ::error("script error in %s:",Report->filename);
  if(Message) ::error("%s",(char *)Message);
}
