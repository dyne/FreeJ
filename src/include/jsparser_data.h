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

/** shell functions */
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool remove_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool move_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool move_layer_up(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool move_layer_down(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

/** layer constructor */
JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

/** layer methods */

JSBool activate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool deactivate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_num(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_filename(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_geometry(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool set_blit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_blit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool set_alpha(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool get_alpha(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool set_position(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

/** filter constructor */
//JSBool filter_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

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
/*
static JSClass filter_class = {
    "Filter", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub,
    NULL,   NULL,
    filter_constructor
};
*/
static	JSFunctionSpec shell_functions[] = {
    /*    name          native			nargs    */
    {"add_layer",	add_layer,		1},
    {"remove_layer",	remove_layer,		1},
    {"move_layer_up",	move_layer_up,		1},
    {"move_layer_down",	move_layer_down,	1},
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
/* 
JSFunctionSpec layer_methods[] = {
    name		native		nargs    
    {"activate",	activate,	1},
    {"deactivate",	deactivate,	1},
    {"get_num",		get_num,	0},
    {"get_name",	get_name,	0},
    {"get_filename",	get_filename,	0},
    {"get_geometry",	get_geometry,	0},

    {"set_blit",	set_blit,	1},
    {"get_blit",	get_blit,	0},

    {"set_alpha",	set_alpha,	1},
    {"get_alpha",	get_alpha,	0},
    {"set_position",	set_position,	1},
    {0} 
};
*/
