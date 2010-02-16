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

Context *global_environment;

JsParser::JsParser(Context *_env) {
    if(_env!=NULL)
      global_environment=_env;
    init();
    act("javascript parser initialized");
}

JsParser::~JsParser() {
    /** The world is over */
  //    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
    func("JsParser::close()");
}

void JsParser::gc() {
    JS_GC(global_context);
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
    global_context = JS_NewContext(js_runtime, STACK_CHUNK_SIZE);

    // Store a reference to ourselves in the context ...
    JS_SetContextPrivate(global_context, this);

    /* if global_context does not have a value, end the program here */
    if (global_context == NULL) {
	error("JsParser :: error creating context");
	return ;
    }

    /* Set a more strict error checking */
    JS_SetOptions(global_context, JSOPTION_VAROBJFIX); // | JSOPTION_STRICT);

    /* Set the branch callback */
#if defined JSOPTION_NATIVE_BRANCH_CALLBACK
    JS_SetBranchCallback(global_context, js_static_branch_callback);
#else
    JS_SetOperationCallback(global_context, js_static_branch_callback);
#endif

    /* Set the error reporter */
    JS_SetErrorReporter(global_context, js_error_reporter);

    /* Create the global object here */
    //    JS_SetGlobalObject(global_context, global_object);
    //    this is done in init_class / JS_InitStandardClasses.

	global_object = JS_NewObject(global_context, &global_class, NULL, NULL);
	init_class(global_context, global_object);

   /** register SIGINT signal */
	//   signal(SIGINT, js_sigint_handler);


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


	// Screen (in C++ ViewPort) has only one class type
	// all implementations are masked behind the factory
	REGISTER_CLASS("Screen",
		       screen_class,
		       screen_constructor,
		       NULL,
		       screen_methods,
		       NULL);
	Screen = layer_object;

	REGISTER_CLASS("Layer",
		       layer_class,
		       layer_constructor,
		       layer_properties,
		       layer_methods,
		       NULL);
	Layer = layer_object; // last created object
	object_proto = Layer; // following registrations inherit from parent class Layer

	REGISTER_CLASS("GeometryLayer",
		       geometry_layer_class,
		       geometry_layer_constructor,
		       NULL,
		       geometry_layer_methods,
		       object_proto);
	GeometryLayer = layer_object;
	
	REGISTER_CLASS("GeneratorLayer",
		       generator_layer_class,
		       generator_layer_constructor,
		       NULL,
		       generator_layer_methods,
		       object_proto);
	GeneratorLayer = layer_object;

// 	REGISTER_CLASS("VScrollLayer",
// 		vscroll_layer_class,
// 		vscroll_layer_constructor,
// 		vscroll_layer_methods,
// 		object_proto);

	REGISTER_CLASS("ImageLayer",
		       image_layer_class,
		       image_layer_constructor,
		       NULL,
		       image_layer_methods,
		       object_proto);
	ImageLayer = layer_object;

#ifdef WITH_FLASH
	REGISTER_CLASS("FlashLayer",
		       flash_layer_class,
		       flash_layer_constructor,
		       NULL,
		       flash_layer_methods,
		       object_proto);
	FlashLayer = layer_object;
#endif

#ifdef WITH_GOOM
	REGISTER_CLASS("GoomLayer",
		       goom_layer_class,
		       goom_layer_constructor,
		       NULL, // properties
		       goom_layer_methods,
		       object_proto);
	GoomLayer = layer_object;
#endif

#ifdef WITH_AUDIO
	REGISTER_CLASS("AudioJack",
		       js_audio_jack_class,
		       js_audio_jack_constructor,
		       NULL, // properties
		       js_audio_jack_methods,
		       object_proto);
	AudioJack = layer_object;
#endif

#ifdef WITH_V4L
	REGISTER_CLASS("CamLayer",
		       v4l_layer_class,
		       v4l_layer_constructor,
		       NULL, // properties
		       v4l_layer_methods,
		       object_proto);
	CamLayer = layer_object;
#endif

#ifdef WITH_UNICAP
	REGISTER_CLASS("UnicapLayer",
		       unicap_layer_class,
		       unicap_layer_constructor,
		       NULL, // properties
		       unicap_layer_methods,
		       object_proto);
	UnicapLayer = layer_object;
#endif

#ifdef WITH_FFMPEG
	REGISTER_CLASS("MovieLayer",
		       video_layer_class,
		       video_layer_constructor,
		       NULL, // properties
		       video_layer_methods,
		       object_proto);
	MovieLayer = layer_object;
#endif

#ifdef WITH_AVIFILE
   REGISTER_CLASS("MovieLayer",
		  avi_layer_class,
		  avi_layer_constructor,
		  NULL, // properties
		  avi_layer_methods,
		  object_proto);
   MovieLayer = layer_object;

#endif

#if defined WITH_TEXTLAYER
	REGISTER_CLASS("TextLayer",
		       txt_layer_class,
		       txt_layer_constructor,
		       NULL, // properties
		       txt_layer_methods,
		       object_proto);
	TextLayer = layer_object;
#endif

#ifdef WITH_XGRAB
	REGISTER_CLASS("XGrabLayer",
		       js_xgrab_class,
		       js_xgrab_constructor,
		       NULL, // properties
		       js_xgrab_methods,
		       object_proto);
	XGrabLayer = layer_object;
#endif	

#ifdef WITH_CAIRO
	REGISTER_CLASS("VectorLayer",
		       vector_layer_class,
		       vector_layer_constructor,
		       vector_layer_properties,
		       vector_layer_methods,
		       object_proto);
	VectorLayer = layer_object;
#endif		       

	REGISTER_CLASS("Filter",
		       filter_class,
		       filter_constructor,
		       NULL, // properties
		       filter_methods,
		       NULL);
	Filter = layer_object;

	// controller classes
	REGISTER_CLASS("Controller",
		       js_ctrl_class,
		       NULL,
		       NULL, // properties
		       js_ctrl_methods,
		       NULL);
	Controller = layer_object;
	object_proto = Controller;

	REGISTER_CLASS("KeyboardController",
		       js_kbd_ctrl_class,
		       js_kbd_ctrl_constructor,
		       NULL, // properties
		       js_kbd_ctrl_methods,
		       object_proto);
	KeyboardController = layer_object;

	REGISTER_CLASS("MouseController",
		       js_mouse_ctrl_class,
		       js_mouse_ctrl_constructor,
		       NULL, // properties
		       js_mouse_ctrl_methods,
		       object_proto);
	MouseController = layer_object;

	REGISTER_CLASS("JoystickController",
		       js_joy_ctrl_class,
		       js_joy_ctrl_constructor,
		       NULL, // properties
		       js_joy_ctrl_methods,
		       object_proto);
	JoystickController = layer_object;

	REGISTER_CLASS("TriggerController",
		       js_trigger_ctrl_class,
		       js_trigger_ctrl_constructor,
		       NULL, // properties
		       js_trigger_ctrl_methods,
		       object_proto);
	TriggerController = layer_object;

	REGISTER_CLASS("ViMoController",
		       js_vimo_ctrl_class,
		       js_vimo_ctrl_constructor,
		       NULL, // properties
		       js_vimo_ctrl_methods,
		       object_proto);
	ViMoController = layer_object;

#ifdef WITH_MIDI
	REGISTER_CLASS("MidiController",
		       js_midi_ctrl_class,
		       js_midi_ctrl_constructor,
		       NULL, // properties
		       js_midi_ctrl_methods,
		       object_proto);
	MidiController = layer_object;
#endif

    REGISTER_CLASS("OscController",
		   js_osc_ctrl_class,
		   js_osc_ctrl_constructor,
		   NULL, // properties
		   js_osc_ctrl_methods,
		   object_proto);
    OscController = layer_object;

#ifdef WITH_CWIID
    REGISTER_CLASS("WiiController",
		   js_wii_ctrl_class,
		   js_wii_ctrl_constructor,
		   NULL, // properties
		   js_wii_ctrl_methods,
		   object_proto);
    WiiController = layer_object;
#endif

#ifdef WITH_OGGTHEORA
	// encoder class
    REGISTER_CLASS("VideoEncoder",
		   js_vid_enc_class,
		   js_vid_enc_constructor,
		   NULL, // properties
		   js_vid_enc_methods,
		   NULL);
    VideoEncoder = layer_object;
#endif

//	  JS_DefineProperties(global_context, layer_object, layer_properties);


   return;
}

int JsParser::include(const char* jscript) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  const char *debian_path = "share/doc/freej/scripts/javascript/lib";
  int res = 0;
  char temp[512];
  FILE *fd;

  snprintf(temp,512,"%s",jscript);
  fd = fopen(temp,"r");
  if(!fd) {
    snprintf(temp,511,"%s/%s/%s",PREFIX,debian_path,jscript);
    fd = fopen(temp,"r");
    if(!fd) {
      error("included file %s not found", jscript);
      error("locations checked: current and %s/%s",
	    PREFIX,debian_path);
      error("javascript include('%s') failed", jscript);
      
      return res;
    }
  }
  
  fclose(fd);
  
  res = this->open(temp); 
  if (!res) {
    // all errors already reported,
    // js->open talks a lot
    error("JS include('%s') failed", jscript);
    return res;
  }
  
  func("JS: included %s", jscript);
  return res;
}

/* return lines read, or 0 on error */
int JsParser::open(const char* script_file) {
	return open(global_context, global_object, script_file);
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
	func("%s eval: %p", __PRETTY_FUNCTION__, obj);
	eval_res = JS_EvaluateScript(cx, obj,
		buf, len, script_file, 0, &res);
	// if anything more was wrong, our ErrorReporter was called!
	free(buf);
	func("%s evalres: %i", __func__, eval_res);
	//	gc();
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
  eval_res = JS_EvaluateScript(global_context, global_object,
		      command, strlen(command), "console", 0, &res);
  // return the result (to console)
  if(!JSVAL_IS_VOID(res)){
      str=JS_ValueToString(global_context, res);
      if(str){
          act("JS parse res: %s", JS_GetStringBytes(str));
      } else {
          JS_ReportError(global_context, "Can't convert result to string");
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
	JS_ClearScope(global_context, global_object);
	init_class(global_context, global_object);
	gc();
	return 0;
}

void js_debug_property(JSContext *cx, jsval vp) {
  func(" vp mem address %p", &vp);
  int tag = JSVAL_TAG(vp);
  func(" type tag is %i: %s",tag,
       (tag==0x0)?"object":
       (tag==0x1)?"integer":
       (tag==0x2)?"double":
       (tag==0x4)?"string":
       (tag==0x6)?"boolean":
       "unknown");

  switch(tag) {
  case 0x0:
    {
      JSObject *obj = JSVAL_TO_OBJECT(vp);
      jsval val;
      if( JS_IsArrayObject(cx, obj) ) {
	jsuint len; JS_GetArrayLength(cx, obj, &len);
	func(" object is an array of %u elements", len);
	for(jsuint c = 0; c<len; c++) {
	  func(" dumping element %u:",c);
	  JS_GetElement(cx, obj, c, &val);
	  if(val == JSVAL_VOID)
	    func(" content is VOID");
	  else
	    js_debug_property(cx, val);
	}
      } else {
	func(" object type is unknown to us (not an array?)");
      }
    }
    break;
  case 0x1:
    {
      JS_PROP_INT(num, vp);
      func("  Sint[ %i ] Uint[ %u ]",
	   num, num);
    }
    break;

  case 0x2:
    {
      JS_PROP_DOUBLE(num, vp);
      func("  double is %.4f",num);
    }
    break;
    
  case 0x4:
    {
      char *cap = NULL;
      JS_PROP_STRING(cap);
      func("  string is \"%s\"",cap);
    }
    break;

  case 0x6:
    {
      bool b = false;
      b = JSVAL_TO_BOOLEAN(vp);
      func("  boolean is %i",b);
    }
    break;

  default:
    func(" tag %u is unhandled, probably double");
    JS_PROP_DOUBLE(num, vp);
    func("  Double [ %.4f ] - Sint[ %i ] - Uint[ %u ]",
	 num, num, num);
  }
}

void js_debug_argument(JSContext *cx, jsval vp) {
  func(" arg mem address %p", &vp);
  int tag = JSVAL_TAG(vp);

  func(" type tag is %i: %s",tag,
       (tag==0x0)?"object":
       (tag==0x1)?"integer":
       (tag==0x2)?"double":
       (tag==0x4)?"string":
       (tag==0x6)?"boolean":
       "unknown");

  switch(tag) {
  case 0x0:
    {
      JSObject *obj = JSVAL_TO_OBJECT(vp);
      jsval val;
      if( JS_IsArrayObject(cx, obj) ) {
	jsuint len; JS_GetArrayLength(cx, obj, &len);
	func(" object is an array of %u elements", len);
	for(jsuint c = 0; c<len; c++) {
	  func(" dumping element %u:",c);
	  JS_GetElement(cx, obj, c, &val);
	  if(val == JSVAL_VOID)
	    func(" content is VOID");
	  else
	    js_debug_argument(cx, val);
	}
      } else {
	func(" object type is unknown to us (not an array?)");
      }
    }
    break;
  case 0x1:
    {
      jsint num = js_get_int(vp);
      func("  Sint[ %i ] Uint[ %u ] Double [ %.2f ]", num, num, num);
    }
    break;

  case 0x2:
    {
      jsdouble num = js_get_double(vp);
      func("  Double [ %.2f ]", num);
    }
    break;
    
  case 0x4:
    {
      char *cap;
      if(JSVAL_IS_STRING(vp)) {
	cap = JS_GetStringBytes( JS_ValueToString(cx, vp) );
	func("  string is \"%s\"",cap);
      } else {
	JS_ReportError(cx,"%s: argument value is not a string",__FUNCTION__);
	::error("%s: argument value is not a string",__FUNCTION__);
      }
    }
    break;
    
  case 0x6:
    {
      bool b = false;
      b = JSVAL_TO_BOOLEAN(vp);
      func("  boolean is %i",b);
    }
    break;

  default:
    func(" arg %u is unhandled, probably double");
    jsint num = js_get_double(vp);
    func("  Double [ %.4f ] - Sint[ %i ] - Uint[ %u ]",
	 num, num, num);
  }
}
