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
#define JS(fun) \
JSBool fun(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

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
JS(cafudda);
JS(quit);
JS(add_layer);
JS(rem_layer);

static	JSFunctionSpec global_functions[] = {
    /*    name          native			nargs    */
    {"cafudda",         cafudda,                1},
    {"quit",            quit,                   0},
    {"add_layer",	add_layer,		1},
    {"remove_layer",	rem_layer,		1},
    {0}
};


//                  Linklist Entry inherited methods
JS(entry_up);
JS(entry_down);
JS(entry_move);

//                  Layer


JS(layer_constructor);

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
//JS(layer_set_active);
//JS(layer_get_active);
JS(layer_activate);
JS(layer_deactivate);
JS(layer_get_name);
JS(layer_get_filename);
//JS( layer_get_geometry);
JS(layer_set_blit);
JS(layer_get_blit);
JS(layer_set_alpha);
JS(layer_get_alpha);
JS(layer_set_position);
JS(layer_get_x_position);
JS(layer_get_y_position);
JS(add_filter);
JS(rem_filter);
JS(get_filter_at);


JSFunctionSpec layer_methods[] = {
    //    name		native		nargs    
    //    {"set_active",	layer_set_active,	1},
    //    {"get_active",      layer_get_active,       0},
    {"activate",	layer_activate,	0},
    {"deactivate",	layer_deactivate,	0},
    {"get_name",	layer_get_name,	0},
    {"get_filename",	layer_get_filename,	0},
    //    {"get_geometry",	layer_get_geometry,	0},
    {"set_blit",	layer_set_blit,	1},
    {"get_blit",	layer_get_blit,	0},
    {"set_alpha",	layer_set_alpha,	1},
    {"get_alpha",	layer_get_alpha,	0},
    {"set_position",	layer_set_position,	2},
    {"get_x_position",	layer_get_x_position,	2},
    {"get_y_position",	layer_get_x_position,	2},
    {"add_filter",	add_filter,	1},
    {"rem_filter",	rem_filter,	1},
//    {"get_filter_at",	get_filter_at,	0},

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
JS(filter_constructor);

   static JSClass filter_class = {
   "Filter", JSCLASS_HAS_PRIVATE,
   JS_PropertyStub,  JS_PropertyStub,
   JS_PropertyStub,  JS_PropertyStub,
   JS_EnumerateStub, JS_ResolveStub,
   JS_ConvertStub,   JS_FinalizeStub,
   NULL,   NULL,
   filter_constructor
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
   {"activate",	activate,	0},
   {"deactivate",	deactivate,	0},
   {"get_num",		get_num,	0},
   {"get_name",	get_name,	0},
   {"get_filename",	get_filename,	0},
   {"get_geometry",	get_geometry,	0},

   {"set_blit",	set_blit,	1},
   {"get_blit",	get_blit,	0},

   {"set_alpha",	set_alpha,	1},
   {"get_alpha",	get_alpha,	0},
   {"set_position",	set_position,	1},
   {"get_position",	get_position,	0},
   {0} 
   };
   */
