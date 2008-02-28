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

void JsParser::gc() {
    JS_GC(js_context);
}

void JsParser::init() {
  //JSBool ret;
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

    /* Set the branch callback */
    JS_SetBranchCallback(js_context, js_static_branch_callback);

    /* Set the error reporter */
    JS_SetErrorReporter(js_context, js_error_reporter);

    /* Create the global object here */
    //    JS_SetGlobalObject(js_context, global_object);
    //    this is done in init_class / JS_InitStandardClasses.

	global_object = JS_NewObject(js_context, &global_class, NULL, NULL);
	init_class(js_context, global_object);

   /** register SIGINT signal */
   signal(SIGINT, js_sigint_handler);

   ///////////////////////////////
   // setup the freej context
   env->osd.active = false;

   return;
}

void JsParser::init_class(JSContext *cx, JSObject *obj) {
   
	/* Initialize the built-in JS objects and the global object 
	* As a side effect, JS_InitStandardClasses establishes obj as
	* the global object for cx, if one is not already established. */
	JS_InitStandardClasses(cx, obj);

	/* Declare shell functions */
	if (!JS_DefineFunctions(cx, obj, global_functions)) {
		error("JsParser :: error defining global functions");
		return ;
	}

	///////////////////////////////////////////////////////////
	// Initialize classes

	JSObject *object_proto; // reminder for inher.
	JSObject *layer_object; // used in REGISTER_CLASS macro
	REGISTER_CLASS("Layer",
		layer_class,
		layer_constructor,
		layer_methods,
		NULL);
	object_proto = layer_object; // last created object

	REGISTER_CLASS("ParticleLayer",
		particle_layer_class,
		particle_layer_constructor,
		particle_layer_methods,
		object_proto);

	REGISTER_CLASS("GeometryLayer",
		geometry_layer_class,
		geometry_layer_constructor,
		geometry_layer_methods,
		object_proto);

	REGISTER_CLASS("VScrollLayer",
		vscroll_layer_class,
		vscroll_layer_constructor,
		vscroll_layer_methods,
		object_proto);

	REGISTER_CLASS("ImageLayer",
		image_layer_class,
		image_layer_constructor,
		image_layer_methods,
		object_proto);

	REGISTER_CLASS("FlashLayer",
		flash_layer_class,
		flash_layer_constructor,
		flash_layer_methods,
		object_proto);

	REGISTER_CLASS("GoomLayer",
		goom_layer_class,
		goom_layer_constructor,
		goom_layer_methods,
		object_proto);

#ifdef WITH_V4L
	REGISTER_CLASS("CamLayer",
		v4l_layer_class,
		v4l_layer_constructor,
		v4l_layer_methods,
		object_proto);
#endif

#ifdef WITH_FFMPEG
	REGISTER_CLASS("MovieLayer",
		video_layer_class,
		video_layer_constructor,
		video_layer_methods,
		object_proto);
#endif

#ifdef WITH_AVIFILE
   REGISTER_CLASS("MovieLayer",
		avi_layer_class,
		avi_layer_constructor,
		avi_layer_methods,
		object_proto);
#endif

#ifdef WITH_FT2
	REGISTER_CLASS("TextLayer",
		txt_layer_class,
		txt_layer_constructor,
		txt_layer_methods,
		object_proto);
#endif
    
    
    REGISTER_CLASS("Filter",
                   filter_class,
                   filter_constructor,
                   filter_methods,
                   NULL);

    // controller classes
    REGISTER_CLASS("Controller",
		   js_ctrl_class,
		   NULL,
		   js_ctrl_methods,
           NULL);
    object_proto = layer_object;

    REGISTER_CLASS("KeyboardController",
		   js_kbd_ctrl_class,
		   js_kbd_ctrl_constructor,
		   js_kbd_ctrl_methods,
           object_proto);

	REGISTER_CLASS("MouseController",
		js_mouse_ctrl_class,
		js_mouse_ctrl_constructor,
		js_mouse_ctrl_methods,
		object_proto);

	REGISTER_CLASS("JoystickController",
		js_joy_ctrl_class,
		js_joy_ctrl_constructor,
		js_joy_ctrl_methods,
		object_proto);

	REGISTER_CLASS("TriggerController",
		js_trigger_ctrl_class,
		js_trigger_ctrl_constructor,
		js_trigger_ctrl_methods,
		object_proto);

	REGISTER_CLASS("ViMoController",
		js_vimo_ctrl_class,
		js_vimo_ctrl_constructor,
		js_vimo_ctrl_methods,
		object_proto);

#ifdef WITH_MIDI
	REGISTER_CLASS("MidiController",
		js_midi_ctrl_class,
		js_midi_ctrl_constructor,
		js_midi_ctrl_methods,
		object_proto);
#endif

    REGISTER_CLASS("OscController",
		   js_osc_ctrl_class,
		   js_osc_ctrl_constructor,
		   js_osc_ctrl_methods,
		   object_proto);

#ifdef WITH_OGGTHEORA
	// encoder class
	REGISTER_CLASS("VideoEncoder",
		js_vid_enc_class,
		js_vid_enc_constructor,
		js_vid_enc_methods,
		NULL);
#endif

//	  JS_DefineProperties(js_context, layer_object, layer_properties);


   return;
}

/* return lines read, or 0 on error */
int JsParser::open(const char* script_file) {
	return open(js_context, global_object, script_file);
}
int JsParser::open(JSContext *cx, JSObject *obj, const char* script_file) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	jsval res;
	JSBool eval_res = JS_TRUE;
	FILE *fd;
	char *buf = NULL;
	int len;

	fd = fopen(script_file,"r");
	if(!fd) {
		//error("%s: %s : %s",__func__,script_file,strerror(errno));
		JS_ReportErrorNumber(
			cx, JSFreej_GetErrorMessage, NULL,
			JSSMSG_FJ_WICKED, script_file, strerror(errno)
		);
		return 0;
	}
	buf = readFile(fd, &len);
	fclose(fd);

	if (!buf) {
		JS_ReportErrorNumber(
			cx, JSFreej_GetErrorMessage, NULL,
			JSSMSG_FJ_WICKED, script_file, "No buffer for file .... out of memory?"
		);
		return 0;
	}

	res = JSVAL_VOID;
	notice("%s eval: %p", __PRETTY_FUNCTION__, obj);
	eval_res = JS_EvaluateScript(cx, obj,
		buf, len, script_file, 0, &res);
	// if anything more was wrong, our ErrorReporter was called!
	free(buf);
	func("%s evalres: %i", __func__, eval_res);
	gc();
	return eval_res;
}

void js_usescript_gc(JSContext *cx, JSObject *obj);
JSClass UseScriptClass = {
	"UseScript", JSCLASS_HAS_PRIVATE,
	JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,  js_usescript_gc
};

// compile a script and root it to an object
int JsParser::use(JSContext *cx, JSObject *obj, const char* script_file) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	JSObject *scriptObject;
	JSScript *script;
	FILE *fd;
	char *buf = NULL;
	int len;

	fd = fopen(script_file,"r");
	if(!fd) {
		JS_ReportErrorNumber(
			cx, JSFreej_GetErrorMessage, NULL,
			JSSMSG_FJ_WICKED, script_file, strerror(errno)
		);
		return 0;
	}
	buf = readFile(fd, &len);
	fclose(fd);

	if (!buf) {
		JS_ReportErrorNumber(
			cx, JSFreej_GetErrorMessage, NULL,
			JSSMSG_FJ_WICKED, script_file, "No buffer for file .... out of memory?"
		);
		return 0;
	}

	// use a clean obj and put freej inside
	scriptObject = JS_NewObject(cx, &UseScriptClass, NULL, NULL);
	init_class(cx, scriptObject);

	notice("%s from: %p new: %p glob: %p", __PRETTY_FUNCTION__, obj, scriptObject, global_object);
	if(!scriptObject){
		JS_ReportError(cx, "Can't create script");
		return JS_FALSE;
	}

	script = JS_CompileScript(cx, scriptObject, buf, len, script_file, 0);
	if(!script){
		JS_ReportError(cx, "Can't compile script");
		return JS_FALSE;
	}

	jsval rval;
	JS_ExecuteScriptPart(cx, scriptObject, script, JSEXEC_PROLOG, &rval);

	/* save script as private data for the object */
	if(!JS_SetPrivate(cx, scriptObject, (void*)script)){
		return JS_FALSE;
	}

	JS_DefineFunction(cx, scriptObject, "exec", ExecScript, 0, 0);
	return OBJECT_TO_JSVAL(scriptObject);
}

JS(ExecScript) {
	void *p = JS_GetInstancePrivate(cx, obj, &UseScriptClass, NULL);
	JSScript *script;
	*rval = JSVAL_FALSE;

	if(!p)
		return JS_TRUE;

	script = (JSScript*)p;
	notice("%s : obj:%p  sc:%p", __PRETTY_FUNCTION__, obj, script);
	if (JS_ExecuteScriptPart(cx, obj, script, JSEXEC_MAIN, rval)) {
		*rval = JSVAL_TRUE;
	}
	//JS_SetPrivate(cx, obj, NULL);
    JS_GC(cx);
	return JS_TRUE;
}

void js_usescript_gc(JSContext *cx, JSObject *obj) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	JSScript *script;
	void *p = JS_GetInstancePrivate(cx, obj, &UseScriptClass, NULL);
	if(!p)
		return;

	script = (JSScript*)p;
	notice("destroy script %p of %p", script, obj);
	JS_SetPrivate(cx, obj, NULL);
	JS_ClearScope(cx, obj);
	JS_DestroyScript(cx, script);
}

int JsParser::parse(const char *command) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSBool eval_res = JS_TRUE;
  jsval res;
  JSString *str;

  if(!command) { /* true paranoia */
    warning("NULL command passed to javascript parser");
    return 0;
  }

  func("JsParser::parse : %s obj: %p",command, global_object);

  res = JSVAL_VOID;
  eval_res = JS_EvaluateScript(js_context, global_object,
		      command, strlen(command), "console", 0, &res);
  // return the result (to console)
  if(!JSVAL_IS_VOID(res)){
      str=JS_ValueToString(js_context, res);
      if(str){
          act("JS parse res: %s", JS_GetStringBytes(str));
      } else {
          JS_ReportError(js_context, "Can't convert result to string");
      }
  } // else
    // if anything more was wrong, our ErrorReporter was called!
  gc();
  func("%s evalres: %i", __func__, eval_res);
  return eval_res;
}

void JsParser::stop() {
    stop_script=true;
}

char* JsParser::readFile(FILE *file, int *len){
	char *buf;
	int ch;

	fseek(file,0,SEEK_END);
	*len = ftell(file);
	rewind(file);
	ch=fgetc(file);
	/* ignore first line starting with # */
	if(ch=='#') {
		*len-=1;
		while((ch=fgetc(file))!=EOF) {
			*len-=1;
			if(ch=='\n')
				break;
		}
	} else
		ungetc(ch,file);

	buf = (char*)calloc(*len,sizeof(char));
	if (!buf)
		return NULL;
	fread(buf,*len,sizeof(char),file);

	return buf;
}

int JsParser::reset() {
	JS_ClearScope(js_context, global_object);
	init_class(js_context, global_object);
	gc();
	return 0;
}

