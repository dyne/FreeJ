/*  FreeJ
 *
 *  Copyright (C) 2004
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
#include <dirent.h>

#include <context.h>
#include <signal.h>
#include <config.h>
#include <jutils.h>
#include <errno.h>

#ifdef WITH_JAVASCRIPT

#include <jsparser.h>
#include <jsparser_data.h> // private data header
#include <impl_layers.h>



/* we declare the Context pointer static here
   in order to have it accessed from callback functions
   which are not class methods */
static Context *env;
static bool stop_script;

static void sigint_handler(int sig) {
    stop_script=true;
}


static JSBool static_branch_callback(JSContext* Context, JSScript* Script) {
    if(stop_script) {
	stop_script=false;
	return JS_FALSE;
    }
    return JS_TRUE;
}

static void js_error_reporter(JSContext* Context, const char *Message, JSErrorReport *Report) {
  ::error("script error in %s:",Report->filename);
  if(Message) ::error("%s",(char *)Message);
}


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
    notice("JsParser::close()");
}

void JsParser::init() {
  JSBool ret;
  stop_script=false;
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

    /* Create the global object here */
    global_object = JS_NewObject(js_context, &global_class, NULL, NULL);

    /* Set the branch callback */
    JS_SetBranchCallback(js_context, static_branch_callback);

    /* Set the error reporter */
    JS_SetErrorReporter(js_context, js_error_reporter);


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

#ifdef WITH_PNG
    REGISTER_CLASS("PngLayer",
		   png_layer_class,
		   png_layer_constructor,
		   png_layer_methods);
#endif



    REGISTER_CLASS("Effect",
                   effect_class,
                   effect_constructor,
                   effect_methods);

//    JS_DefineProperties(js_context, layer_object, layer_properties);
//
   /** register SIGINT signal */
   signal(SIGINT, sigint_handler);

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
  unsigned int c;

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

  buf = (char*)calloc(len+10,sizeof(char));
  func("JsParser allocated %u bytes",len);
  fread(buf,len,sizeof(char),fd);

  fclose(fd);

  if( JS_EvaluateScript (js_context, global_object,
			 buf, len, script_file, c, &ret_val)
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


JS(cafudda) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  double tmp_double;
  double *seconds;
  int int_seconds;

  if(JSVAL_IS_DOUBLE(argv[0])) {
    
    // JSVAL_TO_DOUBLE segfault when there's an int as input
    seconds=JSVAL_TO_DOUBLE(argv[0]);
    
  } else if(JSVAL_IS_INT(argv[0])) {

    int_seconds=JSVAL_TO_INT(argv[0]);
    seconds=&tmp_double;
    *seconds=(double )int_seconds;

  }
  
  func("JsParser :: run for %f seconds",*seconds);
  env->cafudda(*seconds);

  return JS_TRUE;
}

JS(pause) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  env->pause = !env->pause;

  return JS_TRUE;
}

JS(quit) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 env->quit = true;
 return JS_TRUE;
}


JS(rem_layer) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jslayer;
    Layer *lay;

    jslayer = JSVAL_TO_OBJECT(argv[0]);
    if(!jslayer) JS_ERROR("missing argument");

    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) JS_ERROR("Layer core data is NULL");

    lay->rem();
    lay->quit=true;
    lay->signal_feed();
    lay->join();

    delete lay;
    return JS_TRUE;
}

JS(add_layer) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    Layer *lay;
    JSObject *jslayer;
    *rval=JSVAL_FALSE;

    if(argc<1) JS_ERROR("missing argument");

    jslayer = JSVAL_TO_OBJECT(argv[0]);

    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) JS_ERROR("Layer core data is NULL");

    /** really add layer */
    if(lay->init(env)) {
      env->layers.add(lay);
      *rval=JSVAL_TRUE;
      //      env->layers.sel(0); // deselect others
      //      lay->sel(true);
    } else error("%s: problem occurred initializing Layer",__FUNCTION__);

    return JS_TRUE;
}

JS(list_layers) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;
  
  Layer *lay;
  jsval val;
  int c = 0;
  
  if( env->layers.len() == 0 ) {
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  lay = (Layer*)env->layers.begin();
  while(lay) {
    objtmp = JS_NewObject(cx, &layer_class, NULL, obj);

    JS_SetPrivate(cx,objtmp,(void*) lay);

    val = OBJECT_TO_JSVAL(objtmp);

    JS_SetElement(cx, arr, c, &val );
    
    c++;
    lay = (Layer*)lay->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

JS(layer_list_effects) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;

  Filter *filt;
  jsval val;
  int c = 0;

  GET_LAYER(Layer);

  // no effects
  if(lay->filters.len() == 0) {
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  filt = (Filter*)lay->filters.begin();
  
  while(filt) {

    objtmp = JS_NewObject(cx, &effect_class, NULL, obj);
    
    JS_SetPrivate(cx, objtmp, (void*) filt);

    val = OBJECT_TO_JSVAL(objtmp);

    JS_SetElement(cx, arr, c, &val );
    
    c++;
    filt = (Filter*)filt->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}  

JS(fullscreen) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  env->screen->fullscreen();
  env->clear_all = !env->clear_all;
  return JS_TRUE;
}

JS(set_resolution) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int w = JSVAL_TO_INT(argv[0]);
  int h = JSVAL_TO_INT(argv[1]);
  env->screen->resize(w, h);
  return JS_TRUE;
}


JS(stream_start) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  notice ("Streaming to %s:%u",env->shouter->host(), env->shouter->port());
  act ("Saving to %s", env -> video_encoder -> get_filename());
  env->save_to_file = true;
}
JS(stream_stop) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  ::notice ("Stopped stream to %s:%u", env->shouter->host(), env->shouter->port());
  ::act ("Video saved in file %s",env -> video_encoder -> get_filename());
  env->save_to_file = false;
}

static int dir_selector(const struct dirent *dir) {
  if(dir->d_name[0]=='.') return(0); // remove hidden files
  return(1);
}
JS(freej_scandir) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;
    
  struct dirent **filelist;
  int found;
  int c = 0;
  char *dir;
  
  JS_ARG_STRING(dir,0);
  
  found = scandir(dir,&filelist,dir_selector,alphasort);
  if(found<0) {
    error("scandir error: %s",strerror(errno));
    return JS_TRUE; // fatal error
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  // now fill up the array  
  while(found--) {
    
    str = JS_NewStringCopyZ(cx, filelist[found]->d_name); 
    val = STRING_TO_JSVAL(str);    
    JS_SetElement(cx, arr, c, &val );
    c++;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

JS(freej_echo) {
  char *msg;
  JS_ARG_STRING(msg,0);
  fprintf(stderr,"%s\n",msg);
  return JS_TRUE;
}

JS(freej_strstr) {
  char *haystack;
  char *needle;
  char *res;
  int intval;
  JS_ARG_STRING(haystack,0);
  JS_ARG_STRING(needle,1);
  res = strstr(haystack, needle);
  if(res == NULL)
    intval = 0;
  else intval = 1;
  *rval = INT_TO_JSVAL(intval);
  return JS_TRUE;
}
  

// debugging commodity
// run freej with -D3 to see this
JS(debug) {
  char *msg;
  
  JS_ARG_STRING(msg,0);
 
  func("%s", msg);

  return JS_TRUE;
}


JS(rand) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  int r;

  r = rand();

  if(argc<1) *rval = 1+(int)(r/(RAND_MAX+1.0));
  else {
    JS_ARG_NUMBER(max, 0);
    func("randomizing with max %f",max);
    r = 1+(int)(max*r/(RAND_MAX+1.0));
    *rval = INT_TO_JSVAL(r);
  }

  return JS_TRUE;
}
JS(srand) {
  // this is not fast and you'd better NOT use it often
  // to achieve more randomization on higher numbers:
  // u get unpredictably sloow
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int seed;
  if(argc<1)
    seed = time(NULL);
  else {
    JS_ARG_NUMBER(r,0);
    seed = (int)r;
  }
  
  srand(seed);

  return JS_TRUE;
}


JS(layer_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  //    JSObject *this_obj;
  char *filename;

  Layer *layer;

  if(argc < 1) JS_ERROR("missing argument");

  // recognize the extension and open the file given in argument
  JS_ARG_STRING(filename,0);

  layer = create_layer( filename );
  if(!layer) {
    error("%s: cannot create a Layer using %s",__FUNCTION__,filename);
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  //    this_obj = JS_NewObject(cx, &layer_class, NULL, obj);
  if (!JS_SetPrivate(cx, obj, (void *) layer))
    JS_ERROR("internal error setting private value");

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

JS(effect_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  Filter *filter;
  char *effect_name;

  if(argc < 1) JS_ERROR("missing argument");

  JS_ARG_STRING(effect_name,0);

  filter = env->plugger.pick(effect_name);

  if(filter==NULL) {
    error("JsParser::effect_constructor : filter not found :%s",effect_name); 
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }
  
  if (!JS_SetPrivate(cx, obj, (void *) filter))
    JS_ERROR("internal error setting private value");

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}



////////////////////////////////
// Linklist Entry Methods

JS(entry_down) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Entry);

 lay->down();
 
 return JS_TRUE;
}
JS(entry_up) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Entry);

 lay->up();

 return JS_TRUE;
}

JS(entry_move) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Entry);

 int pos = JSVAL_TO_INT(argv[0]);
 lay->move(pos);

 return JS_TRUE;
}

////////////////////////////////
// Generic Layer methods

JS(layer_set_blit) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char *blit_name;

  GET_LAYER(Layer);

  JS_ARG_STRING(blit_name, 0);

  lay->blitter.set_blit( blit_name );

  return JS_TRUE;
}

JS(layer_get_blit) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    char *blit_type=lay->blitter.current_blit->get_name();
    JSString *str = JS_NewStringCopyZ(cx, blit_type); 
    *rval = STRING_TO_JSVAL(str);

    return JS_TRUE;
}
JS(layer_get_name) { 
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    char *layer_name = lay->get_name();
    JSString *str = JS_NewStringCopyZ(cx, layer_name); 
    *rval = STRING_TO_JSVAL(str);

    return JS_TRUE;
}
JS(layer_get_filename) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    char *layer_filename = lay->get_filename();
    JSString *str = JS_NewStringCopyZ(cx, layer_filename); 
    *rval = STRING_TO_JSVAL(str);

    return JS_TRUE;
}

JS(layer_set_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    if(argc<2) JS_ERROR("missing argument");
    
    GET_LAYER(Layer);

    int new_x_position=JSVAL_TO_INT(argv[0]);
    int new_y_position=JSVAL_TO_INT(argv[1]);
    lay->set_position(new_x_position,new_y_position);

    return JS_TRUE;
}
JS(layer_get_x_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->geo.x);

    return JS_TRUE;
}
JS(layer_get_y_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->geo.y);

    return JS_TRUE;
}
JS(layer_set_blit_value) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    if(argc<1) JS_ERROR("missing argument");
    JS_ARG_NUMBER(value,0);

    GET_LAYER(Layer);
    
    value = 255.0*value;
    if(value>255) {
      warning("blit values should be float ranged between 0.0 and 1.0");
      value = 255;
    }

    //    lay->blitter.fade_value(1,new_value);
    lay->blitter.set_value((int)value);

    return JS_TRUE;
}
JS(layer_get_blit_value) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->blitter.current_blit->value);

    return JS_TRUE;
}
JS(layer_activate) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    lay->active = true;

    return JS_TRUE;
}
JS(layer_deactivate) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    lay->active = false;

    return JS_TRUE;
}
JS(layer_add_effect) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter=NULL;
    Filter *filter;

    jsfilter = JSVAL_TO_OBJECT(argv[0]);
    if(!jsfilter) JS_ERROR("missing argument");

    /**
     * Extract filter and layer pointers from js objects
     */
    filter = (Filter *) JS_GetPrivate(cx, jsfilter);
    if(!filter) JS_ERROR("Effect is NULL");

    GET_LAYER(Layer);

    if(!filter->init(&lay->geo)) {
      error("Effect %s can't initialize for Layer %s",
        filter->getname(), lay->get_name());
      return JS_TRUE;
    }

   lay->filters.add(filter);
   return JS_TRUE;
}

JS(layer_rem_effect) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter=NULL;
    Filter *filter;

    if(argc<1) JS_ERROR("missing argument");

    /** TODO overload with filter name and position */
    if(JSVAL_IS_OBJECT(argv[0])) {
	jsfilter = JSVAL_TO_OBJECT(argv[0]);
	if(!jsfilter) JS_ERROR("missing argument");

	filter = (Filter *) JS_GetPrivate(cx, jsfilter);
	if(!filter) JS_ERROR("Effect data is NULL");

 	filter->rem();
	filter->clean();
	filter = NULL;
    }
    return JS_TRUE;
}

/// rotozooming the layer
JS(layer_rotate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) JS_ERROR("missing argument");
  JS_ARG_NUMBER(degrees,0);

  GET_LAYER(Layer);

  lay->blitter.set_rotate(degrees);

  return JS_TRUE;
}
JS(layer_zoom) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<2) JS_ERROR("missing argument");
  JS_ARG_NUMBER(xmagn,0);
  JS_ARG_NUMBER(ymagn,1);
  
  GET_LAYER(Layer);

  lay->blitter.set_zoom(xmagn,ymagn);

  return JS_TRUE;
}
JS(layer_spin) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<2) JS_ERROR("missing argument");
  JS_ARG_NUMBER(rot,0);
  JS_ARG_NUMBER(magn,1);

  GET_LAYER(Layer);

  lay->blitter.set_spin(rot, magn);

  return JS_TRUE;
}

////////////////////////////////
// Geometry Layer methods
JS_CONSTRUCTOR("GeometryLayer",geometry_layer_constructor,GeoLayer);  
JS(geometry_layer_clear) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  int color = 0x0;
  if(argc>0) color = JSVAL_TO_INT(argv[0]);
  GET_LAYER(GeoLayer);
  lay->clear(color);
  return JS_TRUE;
}
JS(geometry_layer_pixel) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  
  if(argc<3) return JS_FALSE;

  GET_LAYER(GeoLayer);
  
  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int col = JSVAL_TO_INT(argv[2]);

  lay->pixel(x,y,col);

  return JS_TRUE;
}
JS(geometry_layer_hline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int x2 = JSVAL_TO_INT(argv[1]);
  int y = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);
  
  lay->hline(x1, x2, y, col);

  return JS_TRUE;
}
JS(geometry_layer_vline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int y2 = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->vline(x, y1, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_rectangle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->rectangle(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_rectangle_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->rectangle_fill(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_line) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->line(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_aaline) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);
  
  lay->aaline(x1, y1, x2, y2, col);

  return JS_TRUE;
}
JS(geometry_layer_circle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->circle(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_aacircle) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->aacircle(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_circle_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int r = JSVAL_TO_INT(argv[2]);
  int col = JSVAL_TO_INT(argv[3]);

  lay->circle_fill(x, y, r, col);

  return JS_TRUE;
}
JS(geometry_layer_ellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->ellipse(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_aaellipse) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->aaellipse(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_ellipse_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<5) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rx = JSVAL_TO_INT(argv[2]);
  int ry = JSVAL_TO_INT(argv[3]);
  int col = JSVAL_TO_INT(argv[4]);

  lay->ellipse_fill(x, y, rx, ry, col);

  return JS_TRUE;
}
JS(geometry_layer_pie) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<6) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rad = JSVAL_TO_INT(argv[2]);
  int start = JSVAL_TO_INT(argv[3]);
  int end = JSVAL_TO_INT(argv[4]);
  int col = JSVAL_TO_INT(argv[5]);

  lay->pie(x, y, rad, start, end, col);

  return JS_TRUE;
}
JS(geometry_layer_pie_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<4) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x = JSVAL_TO_INT(argv[0]);
  int y = JSVAL_TO_INT(argv[1]);
  int rad = JSVAL_TO_INT(argv[2]);
  int start = JSVAL_TO_INT(argv[3]);
  int end = JSVAL_TO_INT(argv[4]);
  int col = JSVAL_TO_INT(argv[5]);

  lay->pie_fill(x, y, rad, start, end, col);

  return JS_TRUE;
}
JS(geometry_layer_trigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->trigon(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
JS(geometry_layer_aatrigon) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->aatrigon(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
JS(geometry_layer_trigon_fill) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  
  if(argc<7) return JS_FALSE;
  GET_LAYER(GeoLayer);

  int x1 = JSVAL_TO_INT(argv[0]);
  int y1 = JSVAL_TO_INT(argv[1]);
  int x2 = JSVAL_TO_INT(argv[2]);
  int y2 = JSVAL_TO_INT(argv[3]);
  int x3 = JSVAL_TO_INT(argv[4]);
  int y3 = JSVAL_TO_INT(argv[5]);
  int col = JSVAL_TO_INT(argv[6]);

  lay->trigon_fill(x1, y1, x2, y2, x3, y3, col);

  return JS_TRUE;
}
/// TODO: polygon and bezier


////////////////////////////////
// Particle Layer methods
JS_CONSTRUCTOR("ParticleLayer",particle_layer_constructor,GenLayer);
JS(particle_layer_blossom) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(GenLayer);

  int direction = JSVAL_TO_INT(argv[0]);

  (direction>0)?
    lay->blossom_recal(true) :
    lay->blossom_recal(false);

  return JS_TRUE;
}

////////////////////////////////
// VScroll Layer methods
JS_CONSTRUCTOR("VScrollLayer",vscroll_layer_constructor,ScrollLayer);
JS(vscroll_layer_append) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  char *str = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!str) {
    error("JsParser :: invalid string in VScrollLayer::append");
    return JS_FALSE;
  }
  lay->append(str);

  return JS_TRUE;
}
JS(vscroll_layer_speed) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int s = JSVAL_TO_INT(argv[0]);
  lay->step = s;

  return JS_TRUE;
}
JS(vscroll_layer_linespace) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int l = JSVAL_TO_INT(argv[0]);
  lay->line_space = l;

  return JS_TRUE;
}
JS(vscroll_layer_kerning) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(ScrollLayer);

  int k = JSVAL_TO_INT(argv[0]);
  lay->kerning = k;

  return JS_TRUE;
}

#ifdef WITH_V4L
////////////////////////////////
// Video4Linux Layer methods
JS_CONSTRUCTOR("V4lLayer",v4l_layer_constructor,V4lGrabber);
JS(v4l_layer_chan) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int chan=JSVAL_TO_INT(argv[0]);
  lay->set_chan(chan);

  return JS_TRUE;
}
JS(v4l_layer_freq) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int freq=JSVAL_TO_INT(argv[0]);
  lay->set_freq(freq);

  return JS_TRUE;
}
JS(v4l_layer_band) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(V4lGrabber);

  int band=JSVAL_TO_INT(argv[0]);
  lay->set_band(band);

  return JS_TRUE;
}
#endif

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

#ifdef WITH_FT2
////////////////////////////////
// Txt Layer methods
JS_CONSTRUCTOR("TxtLayer",txt_layer_constructor,TxtLayer);
JS(txt_layer_print) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  char *str = JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!str) {
    error("JsParser :: invalid string in TxtLayer::print");
    return JS_FALSE;
  }
  lay->print(str);

  return JS_TRUE;
}
JS(txt_layer_size) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  JS_ARG_NUMBER(size,0);

  lay->set_character_size((unsigned int)size);

  return JS_TRUE;
}
JS(txt_layer_font) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  JS_ARG_NUMBER(font,0);

  lay->set_font((unsigned int)font);

  return JS_TRUE;
}
JS(txt_layer_advance) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  GET_LAYER(TxtLayer);
  
  lay->advance();

  return JS_TRUE;
}
JS(txt_layer_blink) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(TxtLayer);

  if(argc<1) {
    if(!lay->blinking) {
      lay->blinking=true;
      lay->clear_screen=true;
    } else lay->blinking=false;
  } else {
    // fetch argument and switch blinking
    lay->blinking = (bool)JSVAL_TO_INT(argv[0]);
    lay->clear_screen = true;
  }

  return JS_TRUE;
}
JS(txt_layer_blink_on) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  int b = JSVAL_TO_INT(argv[0]);
  lay->onscreen_blink = b;

  return JS_TRUE;
}
JS(txt_layer_blink_off) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  int b = JSVAL_TO_INT(argv[0]);
  lay->offscreen_blink = b;

  return JS_TRUE;
}
#endif

#ifdef WITH_AVCODEC
////////////////////////////////
// Video Layer methods
JS_CONSTRUCTOR("VideoLayer",video_layer_constructor,VideoLayer);

/*
JS(video_layer_seek) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  if(argc<1) {
      return JS_FALSE;
  }
  else {
      double *seconds = JSVAL_TO_DOUBLE(argv[0]);
      lay->relative_seek(*seconds);
  }
  return JS_TRUE;
}
*/

JS(video_layer_forward) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  lay->forward();

  return JS_TRUE;
}
JS(video_layer_rewind) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  GET_LAYER(VideoLayer);

  lay->backward();
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
#ifdef WITH_PNG
////////////////////////////////
// Png Layer methods
JS_CONSTRUCTOR("PngLayer",png_layer_constructor,PngLayer);
#endif


#endif
