/*  FreeJ
 *
 *  Copyright (C) 2004-2007
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

JS(filter_constructor);
JS(layer_constructor);
JS(particle_layer_constructor);
JS(vscroll_layer_constructor);
JS(geometry_layer_constructor);
JS(image_layer_constructor);
JS(flash_layer_constructor);
JS(goom_layer_constructor);

#ifdef WITH_V4L
JS(v4l_layer_constructor);
#endif
#ifdef WITH_AVCODEC
JS(video_layer_constructor);
#endif
#ifdef WITH_FT2
JS(txt_layer_constructor);
#endif


// controller constructors
JS(js_kbd_ctrl_constructor);
JS(js_joy_ctrl_constructor);
#ifdef WITH_MIDI
JS(js_midi_ctrl_constructor);
#endif
JS(js_trigger_ctrl_constructor);
JS(js_xmlrpc_ctrl_constructor);

// encoder constructor
#ifdef WITH_OGGTHEORA
JS(js_vid_enc_constructor);
JS(js_shouter_constructor);
#endif

//////////////////////////////////////////////////////////////
// classes

extern JSClass global_class;
extern JSFunctionSpec global_functions[];

// Effect
extern JSClass filter_class;
extern JSFunctionSpec filter_methods[];

// Layer
extern JSClass layer_class;
extern JSFunctionSpec layer_methods[];

// KeyboardController
extern JSClass js_kbd_ctrl_class;
extern JSFunctionSpec js_kbd_ctrl_methods[];

// JoystickController
extern JSClass js_joy_ctrl_class;
extern JSFunctionSpec js_joy_ctrl_methods[];

// MidiController
#ifdef WITH_MIDI
extern JSClass js_midi_ctrl_class;
extern JSFunctionSpec js_midi_ctrl_methods[];
#endif

// Trigger Controller
extern JSClass js_trigger_ctrl_class;
extern JSFunctionSpec js_trigger_ctrl_methods[];

// XmlRpc Controller
extern JSClass js_xmlrpc_ctrl_class;
extern JSFunctionSpec js_xmlrpc_ctrl_methods[];

// ParticleLayer
extern JSClass particle_layer_class;
extern JSFunctionSpec particle_layer_methods[];

// GeometryLayer
extern JSClass geometry_layer_class;
extern JSFunctionSpec geometry_layer_methods[];

// VScrollLayer
extern JSClass vscroll_layer_class;
extern JSFunctionSpec vscroll_layer_methods[];

// ImageLayer
extern JSClass image_layer_class;
extern JSFunctionSpec image_layer_methods[];

// FlashLayer
extern JSClass flash_layer_class;
extern JSFunctionSpec flash_layer_methods[];

// GoomLayer
extern JSClass goom_layer_class;
extern JSFunctionSpec goom_layer_methods[];

// CamLayer
#ifdef WITH_V4L
extern JSClass v4l_layer_class;
extern JSFunctionSpec v4l_layer_methods[];
#endif

// TextLayer
#ifdef WITH_FT2
extern JSClass txt_layer_class;
extern JSFunctionSpec txt_layer_methods[];
#endif

// MovieLayer
#ifdef WITH_AVCODEC
extern JSClass video_layer_class;
extern JSFunctionSpec video_layer_methods[];
#endif

#ifdef WITH_OGGTHEORA
// VideoEncoder
extern JSClass js_vid_enc_class;
extern JSFunctionSpec js_vid_enc_methods[];
// Shouter to icecast
extern JSClass js_shouter_class;
extern JSFunctionSpec js_shouter_methods[];
#endif




////////////////////////////////
////////////////////////////////
// METHOD DECLARATIONS
////////////////////////////////
////////////////////////////////


////////////////////////////////
// Global Environment methods

JS(cafudda);
JS(quit);
JS(add_layer);
JS(rem_layer);
JS(list_layers);
JS(selected_layer);
JS(debug);
JS(rand);
JS(srand);
JS(pause);
JS(fullscreen);
JS(set_resolution);
JS(freej_scandir);
JS(freej_echo);
JS(freej_strstr);
JS(file_to_strings);
JS(register_controller);
JS(register_encoder);
JS(include_javascript);
JS(system_exec);
////////////////////////////////
// Linklist Entry methods

JS(entry_up);
JS(entry_down);
JS(entry_move);
JS(entry_next);
JS(entry_prev);
JS(entry_select);
#define ENTRY_METHODS \
    {"move",            entry_move,             1}, \
    {"up",              entry_up,               0}, \
    {"down",            entry_down,             0}, \
    {"next",            entry_next,             0}, \
    {"prev",            entry_prev,             0}, \
    {"select",          entry_select,           0}


////////////////////////////////
// Filter methods
JS(filter_apply);
JS(filter_set_parameter);
#define FILTER_METHODS \
    {"set_parameter",           filter_set_parameter,             2}

////////////////////////////////
// parent Layer methods

JS(layer_activate);
JS(layer_deactivate);
JS(layer_get_name);
JS(layer_get_filename);
JS(layer_get_blit);
JS(layer_set_blit);
JS(layer_get_blit_value);
JS(layer_set_blit_value);
JS(layer_fade_blit_value);
JS(layer_set_position);
JS(layer_slide_position);
JS(layer_get_x_position);
JS(layer_get_y_position);
JS(layer_get_width);
JS(layer_get_height);
JS(layer_add_filter);
JS(layer_rem_filter);
JS(layer_rotate);
JS(layer_zoom);
JS(layer_spin);
JS(layer_list_filters);

#define LAYER_METHODS \
    {"activate",	layer_activate,	        0}, \
    {"deactivate",	layer_deactivate,	0}, \
    {"get_name",	layer_get_name,	        0}, \
    {"get_filename",	layer_get_filename,	0}, \
    {"set_blit",	layer_set_blit,	        1}, \
    {"get_blit",	layer_get_blit,	        0}, \
    {"set_blit_value",	layer_set_blit_value,	1}, \
    {"get_blit_value",	layer_get_blit_value,	0}, \
    {"fade_blit_value", layer_fade_blit_value,  2}, \
    {"set_position",	layer_set_position,	2}, \
    {"slide_position",  layer_slide_position,   2}, \
    {"get_x_position",	layer_get_x_position,	0}, \
    {"x",               layer_get_x_position,   0}, \
    {"get_y_position",	layer_get_y_position,	0}, \
    {"y",               layer_get_y_position,   0}, \
    {"get_width",       layer_get_width,        0}, \
    {"w",               layer_get_width,        0}, \
    {"get_height",      layer_get_height,       0}, \
    {"h",               layer_get_height,       0}, \
    {"add_filter",      layer_add_filter,	1}, \
    {"rem_filter",	layer_rem_filter,	1}, \
    {"rotate",          layer_rotate,           1}, \
    {"zoom",            layer_zoom,             2}, \
    {"spin",            layer_spin,             2}, \
    {"list_filters",    layer_list_filters,     0}

////////////////////////////////
// Particle Layer methods
JS(particle_layer_blossom);

////////////////////////////////
// Image Layer methods
JS(image_layer_open);

////////////////////////////////
// Flash Layer methods
JS(flash_layer_open);

////////////////////////////////
// Geometry Layer methods
JS(geometry_layer_clear);
JS(geometry_layer_color);
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

////////////////////////////////
// Goom Layer methods
JS(goom_layer_mode);
JS(goom_layer_middle);
JS(goom_layer_reverse);
JS(goom_layer_speed);
JS(goom_layer_plane);
JS(goom_layer_wave);
JS(goom_layer_hypercos);
JS(goom_layer_noise);

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
JS(txt_layer_color);
//JS(txt_layer_open);
//JS(txt_layer_get_word);
//JS(txt_layer_wordcount);
//JS(txt_layer_string_width);
//JS(txt_layer_string_height);
//JS(txt_layer_advance);
//JS(txt_layer_blink);
//JS(txt_layer_blink_on);
//JS(txt_layer_blink_off);
#endif

#ifdef WITH_OGGTHEORA
JS(vid_enc_start_filesave);
JS(vid_enc_stop_filesave);
// Shouter methods
JS(start_stream);
JS(stop_stream);
JS(stream_host);
JS(stream_port);
JS(stream_mount);
JS(stream_title);
JS(stream_username);
JS(stream_password);
JS(stream_homepage);
JS(stream_description);
#endif

/* TODO: shouter class 
JS(set_shout_host);
JS(set_shout_port);
JS(set_shout_pass);
JS(set_shout_mountpoint);
JS(set_shout_name);
*/

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

*/


#endif
