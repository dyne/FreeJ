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
  { "start_filesave", vid_enc_start_filesave, 1},
  { "stop_filesave",  vid_enc_stop_filesave,  0},
  { "start_stream", start_stream, 0},
  { "stop_stream", stop_stream, 0},

  { "stream_host",   stream_host,  1},
  { "stream_port",   stream_port,  1},
  { "stream_mountpoint",  stream_mount, 1},
  { "stream_title",   stream_title, 1},
  { "stream_username", stream_username, 1},
  { "stream_password", stream_password, 1},
  { "stream_homepage", stream_homepage, 1},
  { "stream_description", stream_description, 1},

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

  if(argc >= 1)

    enc->video_quality = JSVAL_TO_INT(argv[0]);

  if(argc >= 2)

    enc->video_bitrate = JSVAL_TO_INT(argv[1]);

  if(argc >= 3)

    enc->audio_quality = JSVAL_TO_INT(argv[2]);

  if(argc >= 4)

    enc->audio_bitrate = JSVAL_TO_INT(argv[3]);


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

JS(start_stream) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  //  shout_sync(enc->ice);

  if( shout_open(enc->ice) == SHOUTERR_SUCCESS ) {

    act("connected to streaming server on http://%s:%i%s",
	shout_get_host(enc->ice), shout_get_port(enc->ice), shout_get_mount(enc->ice));

    enc->write_to_stream = true;

  } else {

    error("error connecting to server %s: %s",
	  shout_get_host(enc->ice), shout_get_error(enc->ice));

    enc->write_to_stream = false;

  }

  return JS_TRUE;
}

JS(stop_stream) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  enc->write_to_stream = false;

  if(shout_close(enc->ice))
    error("shout_close: %s",shout_get_error(enc->ice));

  //  shout_sync(enc->ice);

  return JS_TRUE;
}

JS(stream_host) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  char *hostname;
  JS_ARG_STRING(hostname, 0);

  if(shout_set_host(enc->ice, hostname))
    error("shout_set_host: %s",shout_get_error(enc->ice));

  return JS_TRUE;
}

JS(stream_port) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  JS_ARG_NUMBER(port, 0);

  if(shout_set_port(enc->ice, (short unsigned int)port))
    error("shout_set_port: %s", shout_get_error(enc->ice));

  return JS_TRUE;
  
}

JS(stream_mount) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  char *mount;
  JS_ARG_STRING(mount, 0);

  if(shout_set_mount(enc->ice, mount))
    error("shout_set_mount: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}

JS(stream_title) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }


  char *title;
  JS_ARG_STRING(title, 0);

  if(shout_set_name(enc->ice, title))
    error("shout_set_title: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}

JS(stream_username) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }


  char *user;
  JS_ARG_STRING(user, 0);

  if(shout_set_user(enc->ice, user))
    error("shout_set_user: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}

JS(stream_password) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  char *pass;
  JS_ARG_STRING(pass, 0);

  if(shout_set_password(enc->ice, pass))
    error("shout_set_pass: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}

JS(stream_homepage) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  char *url;
  JS_ARG_STRING(url, 0);

  if(shout_set_url(enc->ice, url))
    error("shout_set_url: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}

JS(stream_description) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  VideoEncoder *enc = (VideoEncoder*)JS_GetPrivate(cx, obj);
  if(!enc) {
    error("%u:%s:%s :: VideoEncoder core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  char *desc;
  JS_ARG_STRING(desc, 0);

  if(shout_set_description(enc->ice, desc))
    error("shout_set_descrition: %s",shout_get_error(enc->ice));

  return JS_TRUE;

}
  
#endif
