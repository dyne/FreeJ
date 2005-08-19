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

#ifndef __JSPARSER_DATA_H__
#define __JSPARSER_DATA_H__

#include <callbacks_js.h>
#include <jsapi.h>
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

// declare constructors

JS(effect_constructor);
JS(layer_constructor);
JS(particle_layer_constructor);
JS(vscroll_layer_constructor);
JS(geometry_layer_constructor);
JS(image_layer_constructor);

#ifdef WITH_V4L
JS(v4l_layer_constructor);
#endif
#ifdef WITH_AVCODEC
JS(video_layer_constructor);
#endif
#ifdef WITH_FT2
JS(txt_layer_constructor);
#endif


// class and methods are defined in the specific _js.cpp files

extern JSClass global_class;
extern JSFunctionSpec global_functions[];

//DECLARE_CLASS("Effect",effect_class,effect_constructor);
extern JSClass effect_class;
extern JSFunctionSpec effect_methods[];

//DECLARE_CLASS("Layer",layer_class,layer_constructor);
extern JSClass layer_class;
extern JSFunctionSpec layer_methods[];

//DECLARE_CLASS("ParticleLayer",particle_layer_class,particle_layer_constructor);
extern JSClass particle_layer_class;
extern JSFunctionSpec particle_layer_methods[];

//DECLARE_CLASS("GeometryLayer",geometry_layer_class,geometry_layer_constructor);
extern JSClass geometry_layer_class;
extern JSFunctionSpec geometry_layer_methods[];

//DECLARE_CLASS("VScrollLayer",vscroll_layer_class,vscroll_layer_constructor);
extern JSClass vscroll_layer_class;
extern JSFunctionSpec vscroll_layer_methods[];

//DECLARE_CLASS("ImageLayer",image_layer_class,image_layer_constructor);
extern JSClass image_layer_class;
extern JSFunctionSpec image_layer_methods[];

#ifdef WITH_V4L
//DECLARE_CLASS("CamLayer",v4l_layer_class,v4l_layer_constructor);
extern JSClass v4l_layer_class;
extern JSFunctionSpec v4l_layer_methods[];
#endif

#ifdef WITH_FT2
//DECLARE_CLASS("TextLayer",txt_layer_class,txt_layer_constructor);
extern JSClass txt_layer_class;
extern JSFunctionSpec txt_layer_methods[];
#endif


#ifdef WITH_AVCODEC
//DECLARE_CLASS("MovieLayer",video_layer_class,video_layer_constructor);
extern JSClass video_layer_class;
extern JSFunctionSpec video_layer_methods[];
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
JS(set_resolution);
JS(stream_start);
JS(stream_stop);
JS(freej_scandir);
JS(freej_echo);
JS(freej_strstr);

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
// Image Layer methods
JS(image_layer_open);

////////////////////////////////
// Geometry Layer methods
JS(geometry_layer_clear);
JS(geometry_layer_pixel);
JS(geometry_layer_hline);
JS(geometry_layer_vline);
JS(geometry_layer_rectangle);
JS(geometry_layer_rectangle_fill);
JS(geometry_layer_line);
JS(geometry_layer_aaline);
JS(geometry_layer_circle);
JS(geometry_layer_aacircle);
JS(geometry_layer_circle_fill);
JS(geometry_layer_ellipse);
JS(geometry_layer_aaellipse);
JS(geometry_layer_ellipse_fill);
JS(geometry_layer_pie);
JS(geometry_layer_pie_fill);
JS(geometry_layer_trigon);
JS(geometry_layer_aatrigon);
JS(geometry_layer_trigon_fill);
//JS(geometry_layer_polygon);
//JS(geometry_layer_aapolygon);
//JS(geometry_layer_polygon_fill);
//JS(geometry_layer_bezier);

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





/* avifile now removed

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


#ifdef WITH_AVIFILE
JS(avi_layer_constructor);
#endif

#ifdef WITH_AVIFILE
DECLARE_CLASS("MovieLayer",avi_layer_class,avi_layer_constructor);
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

#endif
