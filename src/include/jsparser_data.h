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

/* =================================================
   JAVASCRIPT DECLARATIONS
   classes, methods, properties - everything is declared here
   that's the best place to start documenting, in order to experiment
   with javascript and freej.
*/






//                  GLOBAL



static JSClass global_class = {
    "Freej", JSCLASS_NEW_RESOLVE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub
};

/** global environment functions */
JSBool cafudda(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool quit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool rem_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static	JSFunctionSpec global_functions[] = {
    /*    name          native			nargs    */
    {"cafudda",         cafudda,                1},
    {"quit",            quit,                   0},
    {"add_layer",	add_layer,		1},
    {"remove_layer",	rem_layer,		1},
    {0}
};


//                  Linklist Entry inherited methods
JSBool entry_up(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool entry_down(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool entry_move(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

//                  Layer


JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

static JSClass layer_class = {
    "Layer", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub,
    JS_PropertyStub,  JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,
    JS_ConvertStub,   JS_FinalizeStub,
    NULL,   NULL,
    layer_constructor
};
/** layer methods */
//JSBool layer_set_active(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_active(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_name(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_filename(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_geometry(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool layer_set_blit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_blit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_set_alpha(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_get_alpha(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
//JSBool layer_set_position(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSFunctionSpec layer_methods[] = {
  //    name		native		nargs    
  //    {"set_active",	layer_set_active,	1},
  //    {"get_active",      layer_get_active,       0},

  //    {"get_name",	layer_get_name,	0},
  //    {"get_filename",	layer_get_filename,	0},
  //    {"get_geometry",	layer_get_geometry,	0},

    {"set_blit",	layer_set_blit,	1},
    //    {"get_blit",	layer_get_blit,	0},

    //    {"set_alpha",	layer_set_alpha,	1},
    //    {"get_alpha",	layer_get_alpha,	0},

    //    {"set_position",	layer_set_position,	2},
    
    /* move up and down in the layer depth
       the methods below are shared also with some other classes
       as they are inherited from the Entry (Linklist element)
       we need to find a way to assign them generically to classes -jrml
    */
    
    {"move",            entry_move, 1},
    {"up",              entry_up, 0},
    {"down",            entry_down, 0},
    
    {0} 
};




//               FILTER (TODO)

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


/** filter constructor */
//JSBool filter_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
