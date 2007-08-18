/*  FreeJ
 *
 *  Copyright (C) 2004-2006
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
 * "$Id$"
 *
 */

#include <string.h>


#include <context.h>
#include <signal.h>
#include <config.h>
#include <jutils.h>
#include <errno.h>

#include <callbacks_js.h>
#include <jsparser.h>
#include <jsparser_data.h> // private data header

#include <impl_layers.h>


JsParser::JsParser(Context *_env) {
    if(_env!=NULL)
	env=_env;
    init();
    notice("JavaScript parser initialized");
}

JsParser::~JsParser() {
    /** The world is over */
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
    func("JsParser::close()");
}

void JsParser::init() {
  JSBool ret;
  stop_script=false;
  
  notice("Initializing %s", JS_GetImplementationVersion());

    /* Create a new runtime environment. */
    js_runtime = JS_NewRuntime(8L * 1024L * 1024L);
    if (!js_runtime) {
	error("JsParser :: error creating runtime");
	return ; /* XXX should return int or ptr! */
    }

    /* Create a new context. */
    js_context = JS_NewContext(js_runtime, STACK_CHUNK_SIZE);

    // Store a reference to ourselves in the context ...
    JS_SetContextPrivate(js_context, this);

    /* if js_context does not have a value, end the program here */
    if (js_context == NULL) {
	error("JsParser :: error creating context");
	return ;
    }

    /* Set a more strict error checking */
    JS_SetOptions(js_context, JSOPTION_VAROBJFIX); // | JSOPTION_STRICT);

    /* Create the global object here */
    global_object = JS_NewObject(js_context, &global_class, NULL, NULL);
    //    JS_SetGlobalObject(js_context, global_object);
    //    this is done in JS_InitStandardClasses.

    /* Set the branch callback */
    JS_SetBranchCallback(js_context, js_static_branch_callback);

    /* Set the error reporter */
    JS_SetErrorReporter(js_context, js_error_reporter);

    /* Sets maximum (if stack grows upward) or minimum (downward) legal stack byte
     * address in limitAddr for the thread or process stack used by cx.  To disable
     * stack size checking, pass 0 for limitAddr.
     * JS_SetThreadStackLimit(js_context, 0x0);
     */
   
    /* Initialize the built-in JS objects and the global object */
    JS_InitStandardClasses(js_context, global_object);

    /* Declare shell functions */
    if (!JS_DefineFunctions(js_context, global_object, global_functions)) {
	error("JsParser :: error defining global functions");
	return ;
    }

    ///////////////////////////////////////////////////////////
    // Initialize classes

    REGISTER_CLASS("Layer",
		   layer_class,
		   layer_constructor,
		   layer_methods);

    REGISTER_CLASS("ParticleLayer",
		   particle_layer_class,
		   particle_layer_constructor,
		   particle_layer_methods);

    REGISTER_CLASS("GeometryLayer",
		   geometry_layer_class,
		   geometry_layer_constructor,
		   geometry_layer_methods);

    REGISTER_CLASS("VScrollLayer",
		   vscroll_layer_class,
		   vscroll_layer_constructor,
		   vscroll_layer_methods);

    REGISTER_CLASS("ImageLayer",
		   image_layer_class,
		   image_layer_constructor,
		   image_layer_methods);

    REGISTER_CLASS("FlashLayer",
		   flash_layer_class,
		   flash_layer_constructor,
		   flash_layer_methods);

#ifdef WITH_V4L
    REGISTER_CLASS("CamLayer",
		   v4l_layer_class,
		   v4l_layer_constructor,
		   v4l_layer_methods);
#endif

#ifdef WITH_AVCODEC
    REGISTER_CLASS("MovieLayer",
		   video_layer_class,
		   video_layer_constructor,
		   video_layer_methods);
#endif

#ifdef WITH_AVIFILE
   REGISTER_CLASS("MovieLayer",
		   avi_layer_class,
		   avi_layer_constructor,
		   avi_layer_methods);
#endif

#ifdef WITH_FT2
    REGISTER_CLASS("TextLayer",
		   txt_layer_class,
		   txt_layer_constructor,
		   txt_layer_methods);
#endif
    
    
    REGISTER_CLASS("Effect",
                   effect_class,
                   effect_constructor,
                   effect_methods);

    // controller classes

    REGISTER_CLASS("KeyboardController",
		   js_kbd_ctrl_class,
		   js_kbd_ctrl_constructor,
		   js_kbd_ctrl_methods);

    REGISTER_CLASS("JoystickController",
		   js_joy_ctrl_class,
		   js_joy_ctrl_constructor,
		   js_joy_ctrl_methods);

    REGISTER_CLASS("TriggerController",
           js_trigger_ctrl_class,
           js_trigger_ctrl_constructor,
           js_trigger_ctrl_methods);

#ifdef WITH_OGGTHEORA
    // encoder class
    REGISTER_CLASS("VideoEncoder",
		   js_vid_enc_class,
		   js_vid_enc_constructor,
		   js_vid_enc_methods);
#endif

#ifdef WITH_MIDI
    REGISTER_CLASS("MidiController",
		   js_midi_ctrl_class,
		   js_midi_ctrl_constructor,
		   js_midi_ctrl_methods);
#endif

//    JS_DefineProperties(js_context, layer_object, layer_properties);

   /** register SIGINT signal */
   signal(SIGINT, js_sigint_handler);

   ///////////////////////////////
   // setup the freej context
   env->osd.active = false;


   return;
}

/* return lines read, or 0 on error */
int JsParser::open(const char* script_file) {
  jsval ret_val;
  FILE *fd;
  char *buf;
  int len;

  char header[1024];

  fd = fopen(script_file,"r");
  if(!fd) {
    error("JsParser::open : %s : %s",script_file,strerror(errno));
    return 0;
  }

  // read all the file in once: line by line won't work well in blocks
  func("JsParser reading from file %s",script_file);
  fseek(fd,0,SEEK_END);
  len = ftell(fd);
  rewind(fd);


  // exclude the first line if it calls the shell interpreter
  fgets(header,1023,fd);
  if(header[0]!='#')
    rewind(fd);
  else
    len -= strlen(header);

  buf = (char*)calloc(len+128,sizeof(char));
  func("JsParser allocated %u bytes",len);
  fread(buf,len,sizeof(char),fd);

  fclose(fd);

  if( JS_EvaluateScript (js_context, global_object,
			 buf, len, script_file, lineno, &ret_val)
      == JS_FALSE)
    error("execution of script aborted");

  return ret_val;
}

int JsParser::parse(const char *command) {
  jsval res;
  JSBool ok;

  if(!command) { /* true paranoia */
    warning("NULL command passed to javascript parser");
    return 0;
  }

  func("JsParser::parse : %s",command);

  ok =
    JS_EvaluateScript(js_context, global_object,
		      command, strlen(command), "console", 1, &res);
  if(!ok) {
    char err[512];
    JS_ReportError(js_context, "%s", err);
    error("%s",err);
    return 0;
  }
  return 1;
}

void JsParser::stop() {
    stop_script=true;
}



#ifdef WITH_AVIFILE
////////////////////////////////
// Avi Layer methods
JS_CONSTRUCTOR("AviLayer",avi_layer_constructor,AviLayer);
JS(avi_layer_forward) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int num;

  GET_LAYER(AviLayer);

  if(argc<1) num = 1; // no argument: forward one
  else num = JSVAL_TO_INT(argv[0]);

  *rval = INT_TO_JSVAL(  lay->forward(num)  );

  return JS_TRUE;
}
JS(avi_layer_rewind) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int num;

  GET_LAYER(AviLayer);

  if(argc<1) num = 1; // no argument: forward one
  else num = JSVAL_TO_INT(argv[0]);

  *rval = INT_TO_JSVAL(  lay->rewind(num)  );

  return JS_TRUE;
}
JS(avi_layer_mark_in) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(AviLayer);

  *rval = INT_TO_JSVAL
    ( lay->mark_in
      ( JSVAL_TO_INT(argv[0])
	)
      );
  return JS_TRUE;
}
JS(avi_layer_mark_in_now) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  GET_LAYER(AviLayer);
  lay->mark_in_now();
  return JS_TRUE;
}
JS(avi_layer_mark_out) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(AviLayer);

  *rval = INT_TO_JSVAL
    ( lay->mark_out
      ( JSVAL_TO_INT(argv[0])
	)
      );
  return JS_TRUE;
}
JS(avi_layer_mark_out_now) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  GET_LAYER(AviLayer);
  lay->mark_out_now();
  return JS_TRUE;
}
JS(avi_layer_getpos) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  GET_LAYER(AviLayer);
  *rval = INT_TO_JSVAL(  lay->getpos()  );
  return JS_TRUE;
}
JS(avi_layer_setpos) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(argc<1) return JS_FALSE;
  GET_LAYER(AviLayer);
  *rval = INT_TO_JSVAL
    (  lay->setpos
       ( JSVAL_TO_INT( argv[0] )
	 )
       );
  return JS_TRUE;
}
JS(avi_layer_pause) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  GET_LAYER(AviLayer);
  lay->pause();
  return JS_TRUE;
}
#endif






