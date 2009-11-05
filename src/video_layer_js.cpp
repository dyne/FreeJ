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

#ifdef WITH_FFMPEG
#include <video_layer.h>

DECLARE_CLASS_GC("MovieLayer",video_layer_class,video_layer_constructor,js_layer_gc);

////////////////////////////////
// Video Layer methods
JSFunctionSpec video_layer_methods[] = {
//  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {	"seek",		video_layer_seek, 		1},
  {	"mark-in",	video_layer_mark_in, 		1},
  {	"mark-out",	video_layer_mark_out, 		1},
  {	"pause",	video_layer_pause, 		0}, 
  {0}
};

JS_CONSTRUCTOR("MovieLayer",video_layer_constructor,VideoLayer);




JS(video_layer_seek) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  uint32_t pos;

  GET_LAYER(VideoLayer);

  if(argc<1) {
      return JS_FALSE;
  }
  else {
    JS_ValueToECMAUint32(cx, argv[0], &pos);
    lay->to_seek = pos;
    func("to seek: %d",lay->to_seek);
  }
  return JS_TRUE;
}


JS(video_layer_mark_in) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  lay->set_mark_in();
  return JS_TRUE;
}
JS(video_layer_mark_out) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  lay->set_mark_out();
  return JS_TRUE;
}
JS(video_layer_pause) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  lay->pause();
  return JS_TRUE;
}
#endif
