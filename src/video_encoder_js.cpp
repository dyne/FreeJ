/*  FreeJ
 *  (c) Copyright 2007 Denis Rojo <jaromil@dyne.org>
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

#ifdef WITH_OGGTHEORA

#include <video_encoder.h>
#include <oggtheora_encoder.h>

DECLARE_CLASS("VideoEncoder", js_vid_enc_class, js_vid_enc_constructor);

////////////////////////////////
// Video Encoder methods
JSFunctionSpec js_vid_enc_methods[] = {
  { "set_quality",    vid_enc_set_quality,  2},
  { "set_bitrate",    vid_enc_set_bitrate,  2},
  { "start_filesave", vid_enc_start_filesave, 1},
  { "stop_filesave",  vid_enc_stop_filesave,  0},
  {0}
};

JS(js_vid_enc_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  OggTheoraEncoder *enc;
  enc = new OggTheoraEncoder();
  if(!enc) {
    error("JS::VideoEncoder : error constructing ogg theora video encoder");
    return JS_FALSE;
  }
  if(!enc->init(env)) {
    error("JS::VideoEncoder : failed initialization");
    delete enc; return JS_FALSE;
  }
  if(!JS_SetPrivate(cx,obj,(void*)enc)) {
    error("JS::VideoEncoder : can't set the private value");
    delete enc; return JS_FALSE;
  }
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

JS(vid_enc_set_bitrate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  JS_CHECK_ARGC(1);

  if(argc >= 1)

    enc->video_bitrate = JSVAL_TO_INT(argv[0]);

  if(argc == 2)

    enc->audio_bitrate = JSVAL_TO_INT(argv[1]);

  return(JS_TRUE);

}

JS(vid_enc_set_quality) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);


  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }
  
  JS_CHECK_ARGC(1);

  if(argc >= 1) {

    // just the video quality is set
    enc->video_quality = JSVAL_TO_INT(argv[0]);
    if(enc->video_quality < 0)   enc->video_quality = 0;
    if(enc->video_quality > 100) enc->video_quality = 100;
    
  }

  if(argc == 2) {

    // video and audio quality are set
    enc->audio_quality = JSVAL_TO_INT(argv[1]);
    if(enc->audio_quality < 0)   enc->audio_quality = 0;
    if(enc->audio_quality > 100) enc->audio_quality = 100;

  }

  return(JS_TRUE);
}

JS(vid_enc_start_filesave) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }
  
  JS_CHECK_ARGC(1);

  char *file;

  JS_ARG_STRING(file, 0);

  enc->set_filedump(file);

  return JS_TRUE;
}


JS(vid_enc_stop_filesave) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  enc->set_filedump(NULL);

  return JS_TRUE;
}

#endif
