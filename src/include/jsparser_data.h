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
 * This file contains private data used by spidermonkey parser in jsparser.cpp
 *
 * "$Id$"
 *
 */



#include <jsapi.h>
#include <jsparser.h>
#include <config.h>


/* =================================================
   JAVASCRIPT DECLARATIONS
   classes, methods, properties - everything is declared here
   that's the best place to start documenting, in order to experiment
   with javascript and freej.
*/



////////////////////////////////
////////////////////////////////
// CLASS DECLARATIONS
////////////////////////////////
////////////////////////////////

// constructors
//static JSBool static_branch_callback(JSContext* Context, JSScript* Script);
//static void static_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report);

JS(effect_constructor);
JS(layer_constructor);
JS(particle_layer_constructor);
JS(vscroll_layer_constructor);
JS(filter_constructor);
#ifdef WITH_V4L
JS(v4l_layer_constructor);
#endif
#ifdef WITH_AVIFILE
JS(avi_layer_constructor);
#endif
#ifdef WITH_AVCODEC
JS(video_layer_constructor);
#endif
#ifdef WITH_FT2
JS(txt_layer_constructor);
#endif
#ifdef WITH_PNG
JS(png_layer_constructor);
#endif

// global environment class
static JSClass global_class = {
  "Freej", JSCLASS_NEW_RESOLVE,
  JS_PropertyStub,  JS_PropertyStub,
  JS_PropertyStub,  JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub,
  JS_ConvertStub,   JS_FinalizeStub
};

// class declarations
DECLARE_CLASS("Effect",effect_class,effect_constructor);
DECLARE_CLASS("Layer",layer_class,layer_constructor);
DECLARE_CLASS("ParticleLayer",particle_layer_class,particle_layer_constructor);
DECLARE_CLASS("VScrollLayer",vscroll_layer_class,vscroll_layer_constructor);
#ifdef WITH_V4L
DECLARE_CLASS("CamLayer",v4l_layer_class,v4l_layer_constructor);
#endif
#ifdef WITH_AVCODEC
DECLARE_CLASS("MovieLayer",video_layer_class,video_layer_constructor);
#endif
#ifdef WITH_AVIFILE
DECLARE_CLASS("MovieLayer",avi_layer_class,avi_layer_constructor);
#endif
#ifdef WITH_FT2
DECLARE_CLASS("TextLayer",txt_layer_class,txt_layer_constructor);
#endif
#ifdef WITH_PNG
DECLARE_CLASS("ImageLayer",png_layer_class,png_layer_constructor);
#endif

////////////////////////////////
////////////////////////////////
// METHOD DECLARATIONS
////////////////////////////////
////////////////////////////////


////////////////////////////////
// Environment methods

JS(cafudda);
JS(quit);
JS(add_layer);
JS(rem_layer);
JS(list_layers);
JS(debug);
JS(rand);
JS(srand);
JS(pause);
JS(fullscreen);
JS(set_size);

////////////////////////////////
// Linklist Entry methods

JS(entry_up);
JS(entry_down);
JS(entry_move);
#define ENTRY_METHODS \
    {"move",            entry_move,             1}, \
    {"up",              entry_up,               0}, \
    {"down",            entry_down,             0}

////////////////////////////////
// parent Layer methods

JS(layer_activate);
JS(layer_deactivate);
JS(layer_get_name);
JS(layer_get_filename);
JS(layer_set_blit);
JS(layer_get_blit);
JS(layer_set_blit_value);
JS(layer_get_blit_value);
JS(layer_set_position);
JS(layer_get_x_position);
JS(layer_get_y_position);
JS(layer_add_effect);
JS(layer_rem_effect);
JS(layer_rotate);
JS(layer_zoom);
JS(layer_spin);
JS(layer_list_effects);

#define LAYER_METHODS \
    {"activate",	layer_activate,	        0}, \
    {"deactivate",	layer_deactivate,	0}, \
    {"get_name",	layer_get_name,	        0}, \
    {"get_filename",	layer_get_filename,	0}, \
    {"set_blit",	layer_set_blit,	        1}, \
    {"get_blit",	layer_get_blit,	        0}, \
    {"set_blit_value",	layer_set_blit_value,	1}, \
    {"get_blit_value",	layer_get_blit_value,	0}, \
    {"set_position",	layer_set_position,	2}, \
    {"get_x_position",	layer_get_x_position,	1}, \
    {"get_y_position",	layer_get_y_position,	1}, \
    {"add_effect",      layer_add_effect,	1}, \
    {"rem_effect",	layer_rem_effect,	1}, \
    {"rotate",          layer_rotate,           1}, \
    {"zoom",            layer_zoom,             2}, \
    {"spin",            layer_spin,             2}, \
    {"list_effects",    layer_list_effects,     0}

////////////////////////////////
// Particle Layer methods
JS(particle_layer_blossom);

////////////////////////////////
// VScroll Layer methods
JS(vscroll_layer_append);
JS(vscroll_layer_speed);
JS(vscroll_layer_linespace);
JS(vscroll_layer_kerning);

#ifdef WITH_V4L
////////////////////////////////
// Video4Linux Layer methods
JS(v4l_layer_chan);
JS(v4l_layer_band);
JS(v4l_layer_freq);
#endif

#ifdef WITH_AVCODEC
////////////////////////////////
// Video Layer methods
JS(video_layer_forward);
JS(video_layer_rewind);
JS(video_layer_mark_in);
JS(video_layer_mark_out);
JS(video_layer_pause);
#endif
#ifdef WITH_AVIFILE
////////////////////////////////
// Avi Layer methods
JS(avi_layer_forward);
JS(avi_layer_rewind);
JS(avi_layer_mark_in);
JS(avi_layer_mark_in_now);
JS(avi_layer_mark_out);
JS(avi_layer_mark_out_now);
JS(avi_layer_getpos);
JS(avi_layer_setpos);
JS(avi_layer_pause);
#endif

#ifdef WITH_FT2
////////////////////////////////
// Txt Layer methods
JS(txt_layer_font);
JS(txt_layer_size);
JS(txt_layer_print);
JS(txt_layer_advance);
JS(txt_layer_blink);
JS(txt_layer_blink_on);
JS(txt_layer_blink_off);
#endif


////////////////////////////////
////////////////////////////////
// API FUNCTION ASSIGNEMENTS
////////////////////////////////
////////////////////////////////

static	JSFunctionSpec global_functions[] = {
    /*    name          native			nargs    */
    {"cafudda",         cafudda,                1},
    {"run",             cafudda,                1},
    {"quit",            quit,                   0},
    {"add_layer",	add_layer,		1},
    {"rem_layer",	rem_layer,		1},
    {"list_layers",     list_layers,            0},
    {"debug",           debug,                  1},
    {"rand",            rand,                   0},
    {"srand",           srand,                  1},
    {"pause",           pause,                  0},
    {"fullscreen",      fullscreen,             0},
    {"set_size",        set_size,               2},
    {0}
};

JSFunctionSpec layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {0}
};

// TODO effect methods to control effect parameters
JSFunctionSpec effect_methods[] = {
  ENTRY_METHODS  ,
  {0}
};

JSFunctionSpec particle_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "blossom",      particle_layer_blossom, 1},
  {0}
};

JSFunctionSpec vscroll_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "append",       vscroll_layer_append,   1},
  {     "kerning",      vscroll_layer_append,   1},
  {     "linespace",    vscroll_layer_linespace,1},
  {     "speed",        vscroll_layer_speed,    1}, 
  {0}
};

#ifdef WITH_V4L
JSFunctionSpec v4l_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "chan",         v4l_layer_chan,         1},
  {     "band",         v4l_layer_band,         1},
  {     "freq",         v4l_layer_freq,         1},
  {0}
};
#endif

#ifdef WITH_AVIFILE
JSFunctionSpec avi_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "forward",      avi_layer_forward,      1},
  {     "rewind",       avi_layer_rewind,       1},
  {     "mark_in",      avi_layer_mark_in,      1},
  {     "mark_in_now",  avi_layer_mark_in_now,  1},
  {     "mark_out",     avi_layer_mark_out,     1},
  {     "mark_out_now", avi_layer_mark_out_now, 1},
  {     "setpos",       avi_layer_setpos,       1},
  {     "getpos",       avi_layer_getpos,       1},  
  {     "pause",        avi_layer_pause,        1},
  {0}
};
#endif

#ifdef WITH_FT2
JSFunctionSpec txt_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  //     name           native                  nargs
  {      "print",       txt_layer_print,        1},
  {      "font",        txt_layer_font,         1},
  {      "size",        txt_layer_size,         1},
  {      "advance",     txt_layer_advance,      0},
  {      "blink",       txt_layer_blink,        1},
  {      "blink_on",    txt_layer_blink_on,     1},
  {      "blink_off",   txt_layer_blink_off,    1},
  {0}
};
#endif

#ifdef WITH_PNG
JSFunctionSpec png_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {0}
};
#endif

#ifdef WITH_AVCODEC
JSFunctionSpec video_layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {	"ff",		video_layer_forward, 		0},
  {	"rew",		video_layer_rewind, 		0},
//  {	"seek",		video_layer_seek, 		0},
  {	"mark-in",	video_layer_mark_in, 		0},
  {	"mark-out",	video_layer_mark_out, 		0},
  {	"pause",	video_layer_pause, 		0}, 
  {0}
};
#endif

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

