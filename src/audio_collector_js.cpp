/*  FreeJ
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id $
 *
 */

#include <config.h>
#ifdef WITH_AUDIO
#include <context.h>
#include <jutils.h>

#include <callbacks_js.h>
#include <jsparser_data.h>

#include <audio_collector.h>
#include <audio_jack.h>

/// Javascript audio collector
JS(js_audio_jack_constructor);
void js_audio_jack_gc(JSContext *cx, JSObject *obj);

DECLARE_CLASS_GC("AudioJack", js_audio_jack_class,
		 js_audio_jack_constructor, js_audio_jack_gc)

JS(js_audio_jack_add_output);
JS(js_audio_jack_get_harmonic);
JS(js_audio_jack_fft);

JSFunctionSpec js_audio_jack_methods[] = {
  {"set_layer", js_audio_jack_add_layer, 1},
  {"add_output", js_audio_jack_add_output, 1},
  {"get_harmonic", js_audio_jack_get_harmonic, 1},
  {"fft", js_audio_jack_fft, 0},
  {0}
};

JS(js_audio_jack_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char excp_msg[MAX_ERR_MSG + 1];
  char *port;

  JS_CHECK_ARGC(3);


  JS_ARG_STRING(port,0);
  JS_ARG_NUMBER(sample,1);
  JS_ARG_NUMBER(rate,2);
  
  AudioCollector *audio = new AudioCollector(port, (int)sample, (int)rate);

  if( ! JS_SetPrivate(cx, obj, (void*)audio) ) {
    sprintf(excp_msg, "failed assigning audio jack to javascript");
    goto error;
  }
 
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  //  cx->newborn[GCX_OBJECT] = NULL;
  delete audio;
  return JS_FALSE;
}

JS(js_audio_jack_add_output) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];
  Layer *lay;
  AudioCollector *audio;
  JSObject *jslayer;

  JS_CHECK_ARGC(1);
  //  js_is_instanceOf(&layer_class, argv[0]);

  audio = (AudioCollector*)JS_GetPrivate(cx, obj);
  if(!audio) {
    sprintf(excp_msg, "audio collector core data is NULL");
    goto error;
  }

  jslayer = JSVAL_TO_OBJECT(argv[0]);
  lay = (Layer*)JS_GetPrivate(cx, jslayer);
  if(!lay) {
    sprintf(excp_msg, "audio add_output called on an invalid object");
    goto error;
  }

  // assign the audio collector to the layer
  lay->audio = audio;

  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  return JS_FALSE;
}

JS(js_audio_jack_get_harmonic) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];
  float harmonic;

  JS_CHECK_ARGC(1);
  
  JS_ARG_NUMBER(hc,0);
  
  AudioCollector *audio = (AudioCollector*)JS_GetPrivate(cx, obj);
  if(!audio) {
    sprintf(excp_msg, "audio collector core data is NULL");
    goto error;
  }

  harmonic = audio->GetHarmonic((int)hc);

  return JS_NewNumberValue(cx, (double)harmonic, rval);
  
 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  return JS_FALSE;
}

JS(js_audio_jack_add_layer) {
   func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

   Layer *lay;
   JSObject *jslayer;
   *rval=JSVAL_FALSE;

   if(argc<1) JS_ERROR("missing argument");
   //   js_is_instanceOf(&layer_class, argv[0]);
   
   jslayer = JSVAL_TO_OBJECT(argv[0]);
   lay = (Layer *) JS_GetPrivate(cx, jslayer);
   if(!lay) JS_ERROR("Layer core data is NULL");

   if(!lay->screen) {
     error("layer %s is not added to any screen", lay->name);
     *rval = JSVAL_FALSE;
     return JS_TRUE;
   }
   
   AudioCollector *audio = (AudioCollector*)JS_GetPrivate(cx, obj);
   if(!audio) JS_ERROR("Audio core data is NULL");
   if(!audio->attached) {
     error("audio jack is not attached (jack daemon not running?)");
     *rval = JSVAL_FALSE;
     return JS_TRUE;
   }

   lay->screen->add_audio( audio->Jack );

   *rval = JSVAL_TRUE;

   return JS_TRUE;
}

JS(js_audio_jack_fft) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];

  AudioCollector *audio = (AudioCollector*)JS_GetPrivate(cx, obj);
  if(!audio) {
    sprintf(excp_msg, "audio collector core data is NULL");
    goto error;
  }
  
  audio->GetFFT();

  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  return JS_FALSE;
}

void js_audio_jack_gc(JSContext *cx, JSObject *obj) {
  func("%s",__PRETTY_FUNCTION__);

  AudioCollector *audio = (AudioCollector*)JS_GetPrivate(cx, obj);
  if(!audio) return;

  delete audio;

}
  
#endif
