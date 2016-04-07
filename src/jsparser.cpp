/*  FreeJ
 *
 *  Copyright (C) 2004-2010 Denis Roio       <jaromil@dyne.org>
 *  Copyright (C) 2004-2006 Silvano Galliani <kysucix@dyne.org>
 *  Copyright (C) 2010      Andrea Guzzo     <xant@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
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

#ifndef PREFIX
#define PREFIX "/usr"
#endif

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
#include <linklist.h>

Context *global_environment;

JsExecutionContext::JsExecutionContext(JsParser *jsParser)
{
  parser = jsParser;
  /* Create a new runtime environment. */
  rt = JS_NewRuntime(8L * 1024L * 1024L);
  if (!rt) {
    error("JsParser :: error creating runtime");
    return; /* XXX should return int or ptr! */
  }

  /* Create a new context. */
  cx = JS_NewContext(rt, STACK_CHUNK_SIZE);
  /* if global_context does not have a value, end the program here */
  if (cx == NULL) {
    error("JsParser :: error creating context");
    return;
  }

  JS_BeginRequest(cx);
  // Store a reference to ourselves in the context ...
  JS_SetContextPrivate(cx, parser);

  /* Set a more strict error checking */
  JS_SetOptions(cx, JSOPTION_VAROBJFIX); // | JSOPTION_STRICT);

  /* Set the branch callback */
#if defined JSOPTION_NATIVE_BRANCH_CALLBACK
  JS_SetBranchCallback(cx, js_static_branch_callback);
#else
  JS_SetOperationCallback(cx, js_static_branch_callback);
#endif

  /* Set the error reporter */
  JS_SetErrorReporter(cx, js_error_reporter);

  /* Create the global object here */
  //  JS_SetGlobalObject(global_context, global_object);
  //  this is done in init_class / JS_InitStandardClasses.
  obj = JS_NewObject(cx, &global_class, NULL, NULL);
  init_class();
  JS_EndRequest(cx);
  // deassociate this context from the creating thread
  // so that it can be used in other threads
  // https://developer.mozilla.org/en/SpiderMonkey/JSAPI_Reference/JS_NewContext
  JS_ClearContextThread(cx);
  /** register SIGINT signal */
  //   signal(SIGINT, js_sigint_handler);
}

JsExecutionContext::~JsExecutionContext()
{
  JS_SetContextThread(cx);
  JS_GC(cx);
  JS_BeginRequest(cx);
  JS_ClearScope(cx, obj);
  JS_EndRequest(cx);
  JS_ClearContextThread(cx);
  JS_DestroyContext(cx);
  JS_DestroyRuntime(rt);
}

void JsExecutionContext::gc()
{
  JS_SetContextThread(cx);
  JS_MaybeGC(cx);
  JS_ClearContextThread(cx);
}

void JsExecutionContext::init_class() {

  /* Initialize the built-in JS objects and the global object
   * As a side effect, JS_InitStandardClasses establishes obj as
   * the global object for cx, if one is not already established. */
  JS_InitStandardClasses(cx, obj);

  /* Declare shell functions */
  if (!JS_DefineFunctions(cx, obj, global_functions)) {
    JS_EndRequest(cx);
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
    screen_properties,
    screen_methods,
    NULL);

  REGISTER_CLASS("Parameter",
    parameter_class,
    parameter_constructor,
    parameter_properties,
    parameter_methods,
    NULL);

  REGISTER_CLASS("Layer",
    layer_class,
    layer_constructor,
    layer_properties,
    layer_methods,
    NULL);
  object_proto = layer_object; // following registrations inherit from parent class Layer

  REGISTER_CLASS("GeometryLayer",
    geometry_layer_class,
    geometry_layer_constructor,
    NULL,
    geometry_layer_methods,
    object_proto);

  REGISTER_CLASS("GeneratorLayer",
    generator_layer_class,
    generator_layer_constructor,
    NULL,
    generator_layer_methods,
    object_proto);

  REGISTER_CLASS("ImageLayer",
    image_layer_class,
    image_layer_constructor,
    NULL,
    image_layer_methods,
    object_proto);

#ifdef WITH_FLASH
  REGISTER_CLASS("FlashLayer",
    flash_layer_class,
    flash_layer_constructor,
    NULL,
    flash_layer_methods,
    object_proto);
#endif

#ifdef WITH_GOOM
  REGISTER_CLASS("GoomLayer",
    goom_layer_class,
    goom_layer_constructor,
    NULL, // properties
    goom_layer_methods,
    object_proto);
#endif

#ifdef WITH_AUDIO
  REGISTER_CLASS("AudioJack",
    js_audio_jack_class,
    js_audio_jack_constructor,
    NULL, // properties
    js_audio_jack_methods,
    object_proto);
#endif

  REGISTER_CLASS("CamLayer",
    cam_layer_class,
    cam_layer_constructor,
    cam_layer_properties,
    cam_layer_methods,
    object_proto);

// #ifdef WITH_FFMPEG
//   REGISTER_CLASS("MovieLayer",
//     video_layer_class,
//     video_layer_constructor,
//     NULL, // properties
//     video_layer_methods,
//     object_proto);
// #endif

#ifdef WITH_AVIFILE
  REGISTER_CLASS("MovieLayer",
    avi_layer_class,
    avi_layer_constructor,
    NULL, // properties
    avi_layer_methods,
    object_proto);

#endif

#if defined WITH_TEXTLAYER
  REGISTER_CLASS("TextLayer",
    txt_layer_class,
    txt_layer_constructor,
    NULL, // properties
    txt_layer_methods,
    object_proto);
#endif

#ifdef WITH_XGRAB
  REGISTER_CLASS("XGrabLayer",
    js_xgrab_class,
    js_xgrab_constructor,
    NULL, // properties
    js_xgrab_methods,
    object_proto);
#endif

#ifdef WITH_CAIRO
  REGISTER_CLASS("VectorLayer",
    vector_layer_class,
    vector_layer_constructor,
    vector_layer_properties,
    vector_layer_methods,
    object_proto);
#endif

  REGISTER_CLASS("Filter",
    filter_class,
    filter_constructor,
    filter_properties,
    filter_methods,
    NULL);

// controller classes
  REGISTER_CLASS("Controller",
    js_ctrl_class,
    NULL,
    NULL, // properties
    js_ctrl_methods,
    NULL);
  object_proto = layer_object;

  REGISTER_CLASS("KeyboardController",
    js_kbd_ctrl_class,
    js_kbd_ctrl_constructor,
    NULL, // properties
    js_kbd_ctrl_methods,
    object_proto);

  REGISTER_CLASS("MouseController",
    js_mouse_ctrl_class,
    js_mouse_ctrl_constructor,
    NULL, // properties
    js_mouse_ctrl_methods,
    object_proto);

  REGISTER_CLASS("JoystickController",
    js_joy_ctrl_class,
    js_joy_ctrl_constructor,
    NULL, // properties
    js_joy_ctrl_methods,
    object_proto);

  REGISTER_CLASS("TriggerController",
    js_trigger_ctrl_class,
    js_trigger_ctrl_constructor,
    NULL, // properties
    js_trigger_ctrl_methods,
    object_proto);

  REGISTER_CLASS("ViMoController",
    js_vimo_ctrl_class,
    js_vimo_ctrl_constructor,
    NULL, // properties
    js_vimo_ctrl_methods,
    object_proto);

#ifdef WITH_MIDI
  REGISTER_CLASS("MidiController",
    js_midi_ctrl_class,
    js_midi_ctrl_constructor,
    NULL, // properties
    js_midi_ctrl_methods,
    object_proto);
#endif

  REGISTER_CLASS("OscController",
    js_osc_ctrl_class,
    js_osc_ctrl_constructor,
    NULL, // properties
    js_osc_ctrl_methods,
    object_proto);

#ifdef WITH_CWIID
  REGISTER_CLASS("WiiController",
    js_wii_ctrl_class,
    js_wii_ctrl_constructor,
    NULL, // properties
    js_wii_ctrl_methods,
    object_proto);
#endif

#ifdef WITH_OGGTHEORA
// encoder class
  REGISTER_CLASS("VideoEncoder",
    js_vid_enc_class,
    js_vid_enc_constructor,
    NULL, // properties
    js_vid_enc_methods,
    NULL);
#endif

  //JS_DefineProperties(global_context, layer_object, layer_properties);
  return;
}

/////////////// JsParser ////////////////

JsParser::JsParser(Context *_env) {
  if(_env!=NULL)
    global_environment=_env;
  init();
  act("javascript parser initialized");
}

JsParser::~JsParser() {
  /** The world is over */
  //  JS_DestroyContext(js_context);
  JS_DestroyRuntime(js_runtime);
  JS_ShutDown();
  func("JsParser::close()");
}

void JsParser::gc() {
  JsExecutionContext *ecx = runtimes.begin();
  while (ecx) {
    ecx->gc();
    ecx = (JsExecutionContext *)ecx->next;
  }
}

void JsParser::init() {
  //JSBool ret;
  stop_script=false;

  notice("Initializing %s", JS_GetImplementationVersion());

  // create a global execution context (used to evaluate commands on the fly
  // and for other internal operations
  global_runtime = new JsExecutionContext(this);

  /* Create a new runtime environment. */
  js_runtime = global_runtime->rt; // XXX - for retrocompatibilty
    if (!js_runtime) {
    return ; /* XXX should return int or ptr! */
  }

  /* Create a new context. */
  global_context = global_runtime->cx; // XXX - for retrocompatibilty

  /* if global_context does not have a value, end the program here */
    if (global_context == NULL) {
    return ;
  }

  global_object = global_runtime->obj;
  /** register SIGINT signal */
  //   signal(SIGINT, js_sigint_handler);
}

int JsParser::include(JSContext *cx, const char* jscript) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  const char *debian_path = "share/doc/freej/scripts/javascript/lib"; // WTF!!!
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
    } else
      func("included file %s",temp);
  }

  fclose(fd);

  JS_BeginRequest(cx);
  JSObject *gobj = JS_GetGlobalObject(cx);
  if(!gobj) {
    error("JSParser missing global object in context %p",cx);
    JS_EndRequest(cx);
    return false;
  }

  res = this->open(cx, gobj, temp);

  JS_EndRequest(cx);

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

  JsExecutionContext *new_script = new JsExecutionContext(this);
  JS_SetContextThread(new_script->cx);
  JS_BeginRequest(new_script->cx);
  int ret = open(new_script->cx, new_script->obj, script_file);
  JS_EndRequest(new_script->cx);
  JS_ClearContextThread(new_script->cx);
  if (ret) {
    new_script->set_name(script_file);
    runtimes.append(new_script);
  } else {
    delete new_script;
  }
  return ret;
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
  return evaluate(cx, obj, script_file, buf, len);
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
  JS_BeginRequest(cx);

  /* TODO -- FIX
  // use a clean obj and put freej inside
	scriptObject = JS_NewObject(cx, &UseScriptClass, NULL, NULL);
	init_class(cx, scriptObject);
  */
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
    JS_EndRequest(cx);
    return JS_FALSE;
  }

  JS_DefineFunction(cx, scriptObject, "exec", ExecScript, 0, 0);
  JS_EndRequest(cx);
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
  if (JS_ExecuteScriptPart(cx, obj, script, JSEXEC_MAIN, rval))
    *rval = JSVAL_TRUE;

  //JS_SetPrivate(cx, obj, NULL);
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
  int eval_res;
  jsval res;
  JSString *str;

  if(!command) { /* true paranoia */
    warning("NULL command passed to javascript parser");
    return 0;
  }

  func("JS parse: %s", command);

  JsExecutionContext *new_script = new JsExecutionContext(this);
  JS_SetContextThread(new_script->cx);
  JS_BeginRequest(new_script->cx);
  eval_res = evaluate(new_script->cx, new_script->obj,
		    (const char*)"parsed command", command, strlen(command));
  JS_EndRequest(new_script->cx);
  JS_ClearContextThread(new_script->cx);
  if (eval_res)
    runtimes.append(new_script);
  else
    delete new_script;

  func("JS parse result: %i", eval_res);
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
  } else {
    ungetc(ch,file);
  }

  buf = (char*)calloc(*len,sizeof(char));
  if (!buf)
    return NULL;
  fread(buf,*len,sizeof(char),file);

  return buf;
}

int JsParser::reset() {
  JSContext *cx = NULL;
  int i = 0;

  JsExecutionContext *ecx = runtimes.begin();
  while (ecx) {
    delete ecx;
    ecx = runtimes.begin();
    i++;
  }
  return i;
}

int JsParser::evaluate(JSContext *cx, JSObject *obj,
		     const char *name, const char *buf, unsigned int len) {
  jsval res;
  unsigned int lineno;
  JSBool eval_res = JS_TRUE;

  func("JS evaluating script on object %p", __PRETTY_FUNCTION__, obj);
  eval_res = JS_EvaluateScript(cx, obj, buf, len, name, lineno, &res);
  // if anything more was wrong, our ErrorReporter was called!
  func("evaluation result: %i", eval_res);
  return(eval_res);
  // TODO: error message in script evaluation, with line number report
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
