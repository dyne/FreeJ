/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: freej.cpp 654 2005-08-18 16:52:47Z jaromil $"
 *
 */

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <config.h>

#ifdef WITH_V4L
#include <v4l_layer.h>

DECLARE_CLASS_GC("CamLayer",v4l_layer_class,v4l_layer_constructor,js_layer_gc);

////////////////////////////////
// Video4Linux Layer methods
JSFunctionSpec v4l_layer_methods[] = {
  ENTRY_METHODS  ,
  //    name		native		        nargs
  {     "chan",         v4l_layer_chan,         1},
  {     "band",         v4l_layer_band,         1},
  {     "freq",         v4l_layer_freq,         1},
  {0}
};

JS_CONSTRUCTOR("V4lLayer", v4l_layer_constructor, V4lGrabber);
JS(v4l_layer_chan) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  if(argc<1)
    return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int chan=JSVAL_TO_INT(argv[0]);
  lay->set_chan(chan);

  return JS_TRUE;
}
JS(v4l_layer_freq) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1)
    return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int freq=JSVAL_TO_INT(argv[0]);
  lay->set_freq(freq);

  return JS_TRUE;
}
JS(v4l_layer_band) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1)
    return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int band=JSVAL_TO_INT(argv[0]);
  lay->set_band(band);

  return JS_TRUE;
}
#endif




