/*  FreeJ
 *  (c) Copyright 2004 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
 *
 *  This file contains private data used by spidermonkey parser in jsparser.cpp
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
 *
 */

#include <jsapi.h>

JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool kolos(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static JSClass global_class = {
    "Freej", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub
};
static JSClass layer_class = {
    "Layer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub,
    NULL,   NULL,
    layer_constructor
};
static	JSFunctionSpec shell_functions[] = {
    /*    name          native          nargs    */
    {"add_layer",         add_layer,        1},
    {"kolos",         kolos,        2},
    {0}
};
/* class property example. Declare them with JS_DefineProperties
	layer_properties = {
    {"color",       MY_COLOR,       JSPROP_ENUMERATE},
    {"height",      MY_HEIGHT,      JSPROP_ENUMERATE},
    {"width",       MY_WIDTH,       JSPROP_ENUMERATE},
    {"funny",       MY_FUNNY,       JSPROP_ENUMERATE},
    {"array",       MY_ARRAY,       JSPROP_ENUMERATE},
    {"rdonly",      MY_RDONLY,      JSPROP_READONLY},
    {0}
};
*/
/* methods example. Declare them with JS_DefineFunctions
	JSFunctionSpec shell_functions[] = {
	        name          native          nargs    
	    {"add_layer",         add_layer,        1},
	    {"kolos",         kolos,        2},
	    {0}
	};
*/
