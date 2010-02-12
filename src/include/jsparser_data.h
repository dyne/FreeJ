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

JS(screen_constructor);
//void js_screen_gc (JSContext *cx, JSObject *obj);
JS(filter_constructor);
JS(layer_constructor);
//void js_layer_gc (JSContext *cx, JSObject *obj);
JS(vscroll_layer_constructor);
JS(geometry_layer_constructor);
JS(generator_layer_constructor);
JS(image_layer_constructor);
JS(flash_layer_constructor);
JS(movie_layer_constructor);

#ifdef WITH_AUDIO
JS(goom_layer_constructor);
JS(js_audio_jack_constructor);
#endif

#ifdef WITH_V4L
JS(v4l_layer_constructor);
#endif
#ifdef WITH_UNICAP
JS(unicap_layer_constructor);
#endif
#ifdef WITH_FFMPEG
JS(video_layer_constructor);
#else
JS(movie_layer_constructor);
#endif
#if defined WITH_TEXTLAYER
JS(txt_layer_constructor);
#endif
#ifdef WITH_CAIRO
JS(vector_layer_constructor);
#endif

// controller constructors
JS(js_kbd_ctrl_constructor);
JS(js_mouse_ctrl_constructor);
JS(js_joy_ctrl_constructor);
JS(js_vimo_ctrl_constructor);
#ifdef WITH_MIDI
JS(js_midi_ctrl_constructor);
#endif
JS(js_trigger_ctrl_constructor);
JS(js_osc_ctrl_constructor);
JS(js_wii_ctrl_constructor);
void js_ctrl_gc (JSContext *cx, JSObject *obj);

// encoder constructor
#ifdef WITH_OGGTHEORA
JS(js_vid_enc_constructor);
JS(js_shouter_constructor);
#endif


//////////////////////////////////////////////////////////////
// classes
//
//

// pseudoclass for holding namespace of a use(filename) object
extern JSClass UseScriptClass;

extern JSClass global_class;
extern JSFunctionSpec global_functions[];

// Screen
extern JSClass screen_class;
extern JSFunctionSpec screen_methods[];

// Effect
extern JSClass filter_class;
extern JSFunctionSpec filter_methods[];

// Layer
extern JSClass layer_class;
extern JSFunctionSpec layer_methods[];

// Controller
extern JSClass js_ctrl_class;
extern JSFunctionSpec js_ctrl_methods[];

// KeyboardController
extern JSClass js_kbd_ctrl_class;
extern JSFunctionSpec js_kbd_ctrl_methods[];

// MouseController
extern JSClass js_mouse_ctrl_class;
extern JSFunctionSpec js_mouse_ctrl_methods[];

// VideoMouseController
extern JSClass js_vimo_ctrl_class;
extern JSFunctionSpec js_vimo_ctrl_methods[];

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

// OSC Controller
extern JSClass js_osc_ctrl_class;
extern JSFunctionSpec js_osc_ctrl_methods[];

// WII Controller
extern JSClass js_wii_ctrl_class;
extern JSFunctionSpec js_wii_ctrl_methods[];

// GeometryLayer
extern JSClass geometry_layer_class;
extern JSFunctionSpec geometry_layer_methods[];

// GeneratorLayer
extern JSClass generator_layer_class;
extern JSFunctionSpec generator_layer_methods[];

// VScrollLayer
extern JSClass vscroll_layer_class;
extern JSFunctionSpec vscroll_layer_methods[];

// ImageLayer
extern JSClass image_layer_class;
extern JSFunctionSpec image_layer_methods[];

// FlashLayer
extern JSClass flash_layer_class;
extern JSFunctionSpec flash_layer_methods[];

#ifdef WITH_AUDIO
// Audio collector
extern JSClass js_audio_jack_class;
extern JSFunctionSpec js_audio_jack_methods[];
// GoomLayer
extern JSClass goom_layer_class;
extern JSFunctionSpec goom_layer_methods[];
#endif

// CamLayer
#ifdef WITH_V4L
extern JSClass v4l_layer_class;
extern JSFunctionSpec v4l_layer_methods[];
#endif

// Unicap Layer
#ifdef WITH_UNICAP
extern JSClass unicap_layer_class;
extern JSFunctionSpec unicap_layer_methods[];
#endif

// TextLayer
#if defined WITH_TEXTLAYER
extern JSClass txt_layer_class;
extern JSFunctionSpec txt_layer_methods[];
#endif

// VectorLayer
#if defined WITH_CAIRO
extern JSClass vector_layer_class;
extern JSFunctionSpec vector_layer_methods[];
extern JSPropertySpec vector_layer_properties[];
#endif

// MovieLayer
#ifdef WITH_FFMPEG
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


// XGrabLayer
#ifdef WITH_XGRAB
JS(js_xgrab_constructor);
JS(js_xgrab_open);
JS(js_xgrab_close);
extern JSClass js_xgrab_class;
extern JSFunctionSpec js_xgrab_methods[];
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
JS(add_screen);
JS(rem_screen);
JS(ctx_add_layer);
JS(selected_screen);
JS(debug);
JS(js_set_debug);
JS(rand);
JS(srand);
JS(pause);
JS(fullscreen);
JS(set_clear_all);
JS(unset_clear_all);
JS(set_fps);
JS(set_resolution);
JS(get_width);
JS(get_height);
JS(freej_scandir);
JS(freej_echo);
JS(freej_echo_func);
JS(freej_strstr);
JS(read_file);
JS(file_to_strings);
JS(register_controller);
JS(rem_controller);
JS(register_encoder);
JS(include_javascript_file);
JS(execute_javascript_command);
JS(ExecScript); // for the use() objects
JS(system_exec);
JS(list_filters);
JS(js_gc);
JS(reset_js);

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
// Screen methods
JS(screen_init);
JS(screen_initialized);
JS(screen_add_layer);
JS(screen_rem_layer);
JS(screen_list_layers);


////////////////////////////////
// Filter methods
JS(filter_apply);
JS(filter_set_parameter);
JS(filter_list_parameters);
JS(filter_activate);
#define FILTER_METHODS \
  {"set_parameter",           filter_set_parameter,             4}, \
  {"list_parameters",         filter_list_parameters,           0}, \
  {"activate",                filter_activate,                  1}

////////////////////////////////
// Controller methods
JS(controller_activate);
JS(js_mouse_grab);
JS(js_vimo_open);
JS(js_vimo_close);
#ifdef HAVE_LINUX
// joystick rumble is supported only under linux so far...
JS(js_joy_init_rumble);
JS(js_joy_rumble);
#endif
////////////////////////////////
// parent Layer methods

JS(layer_activate);
JS(layer_deactivate);
JS(layer_start);
JS(layer_stop);
JS(layer_get_name);
JS(layer_get_filename);
JS(layer_get_blit);
JS(layer_set_blit);
JS(layer_get_blit_value);
JS(layer_set_blit_value);
JS(layer_fade_blit_value);
JS(layer_set_position);
JS(layer_get_x_position);
JS(layer_get_y_position);
JS(layer_get_width);
JS(layer_get_height);
JS(layer_add_filter);
JS(layer_rem_filter);
JS(layer_rotate);
JS(layer_zoom);
JS(layer_list_filters);
JS(layer_list_parameters);
JS(layer_set_fps);
JS(layer_get_fps);

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
// Vector layer methods
#ifdef WITH_CAIRO
JS(vector_layer_color);
JS(vector_layer_translate);
JS(vector_layer_scale);
JS(vector_layer_rotate);
JS(vector_layer_save);
JS(vector_layer_restore);
JS(vector_layer_lineto);
JS(vector_layer_beginpath);
JS(vector_layer_closepath);
JS(vector_layer_moveto);
JS(vector_layer_quadcurveto);
JS(vector_layer_beziercurveto);
JS(vector_layer_arc);
JS(vector_layer_rect);
JS(vector_layer_fillrect);
JS(vector_layer_fill);
JS(vector_layer_stroke);
JS(vector_layer_push_color);
JS(vector_layer_pop_color);
// Vector layer properties
JSP(vector_layer_fillstyle_g);
JSP(vector_layer_fillstyle_s);
JSP(vector_layer_strokestyle_g);
JSP(vector_layer_strokestyle_s);
JSP(vector_layer_linecap_g);
JSP(vector_layer_linecap_s);
JSP(vector_layer_linewidth_g);
JSP(vector_layer_linewidth_s);
#endif

////////////////////////////////
// VScroll Layer methods
JS(vscroll_layer_append);
JS(vscroll_layer_speed);
JS(vscroll_layer_linespace);
JS(vscroll_layer_kerning);

#ifdef WITH_AUDIO
//////////////////////////////////
// Audio collector methods
JS(js_audio_jack_add_output);
JS(js_audio_jack_get_harmonics);
JS(js_audio_jack_fft);
JS(js_audio_jack_add_layer);

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
#endif

#ifdef WITH_V4L
////////////////////////////////
// Video4Linux Layer methods
JS(v4l_layer_chan);
JS(v4l_layer_band);
JS(v4l_layer_freq);
#endif

#ifdef WITH_UNICAP
JS(unicap_layer_open);
#endif

#ifdef WITH_FFMPEG
////////////////////////////////
// Video Layer methods
JS(video_layer_seek);
JS(video_layer_forward);
JS(video_layer_rewind);
JS(video_layer_mark_in);
JS(video_layer_mark_out);
JS(video_layer_pause);
#endif

#if defined WITH_TEXTLAYER
////////////////////////////////
// Txt Layer methods
JS(txt_layer_font);
JS(txt_layer_size);
JS(txt_layer_print);
JS(txt_layer_color);
JS(txt_layer_calculate_size);
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
JS(vid_enc_add_audio);
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

/*

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
