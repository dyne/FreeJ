/*  FreeJ
 *
 *  Copyright (C) 2004
 *  Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
#include <config.h>

#ifdef WITH_JAVASCRIPT

#include <jsparser.h>
#include <jsparser_data.h> // private data header
#include <impl_layers.h>



/* we declare the Context pointer static here
   in order to have it accessed from callback functions
   which are not class methods */
static Context *env;

JsParser::JsParser(Context *_env) {
    if(_env!=NULL)
	env=_env;
    init();
    parse_count=0;
    notice("JavaScript parser initialized");
}

JsParser::~JsParser() {
    /** The world is over */
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
    notice("JsParser::close()");
}

/*
JSBool kolos(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    JSString *str;
    char *h;
    printf("kolos() has %d arguments.\n", argc);
    printf("Argument number %u is %d\n", 1,JSVAL_TO_INT(argv[0]));
    printf("Argument number %u is %s\n", 2,JS_GetStringBytes(JS_ValueToString(cx,argv[1])));
    printf("\n");
    h=strdup("Stringa di ritorno");
    str = JS_NewStringCopyZ(cx, h); 
    *rval = STRING_TO_JSVAL(str);

    return JS_TRUE;
}
*/

void JsParser::init() {
  JSBool ret;
    /* Create a new runtime environment. */
    js_runtime = JS_NewRuntime(8L * 1024L * 1024L);
    if (!js_runtime) {
	error("JsParser :: error creating runtime");
	return ; /* XXX should return int or ptr! */
    }

    /* Create a new context. */
    js_context = JS_NewContext(js_runtime, STACK_CHUNK_SIZE);

    /* if js_context does not have a value, end the program here */
    if (js_context == NULL) {
	error("JsParser :: error creating context");
	return ;
    }

    /* Create the global object here */
    global_object = JS_NewObject(js_context, &global_class, NULL, NULL);

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
    
    REGISTER_CLASS("VScrollLayer",
		   vscroll_layer_class,
		   vscroll_layer_constructor,
		   vscroll_layer_methods);

    REGISTER_CLASS("V4lLayer",
		   v4l_layer_class,
		   v4l_layer_constructor,
		   v4l_layer_methods);

#ifdef WITH_AVIFILE
    REGISTER_CLASS("AviLayer",
		   avi_layer_class,
		   avi_layer_constructor,
		   avi_layer_methods);
#endif

    REGISTER_CLASS("TxtLayer",
		   txt_layer_class,
		   txt_layer_constructor,
		   txt_layer_methods);

    REGISTER_CLASS("PngLayer",
		   png_layer_class,
		   png_layer_constructor,
		   png_layer_methods);

   /** Initialize Filter class. TODO */
   JS_InitClass(js_context, global_object, NULL,
		 &filter_class, filter_constructor,
		 0, NULL, NULL, NULL, NULL);
//    JS_DefineProperties(js_context, layer_object, layer_properties);

   return;
}

/* return lines read, or 0 on error */
int JsParser::open(const char* script_file) {
  jsval ret_val;
  FILE *fd;
  int c = 0;
  char line[512]; /* is it enough? */
  
  fd = fopen(script_file,"r");
  if(!fd) {
    error("JsParser::open : %s : %s",script_file,strerror(errno));
    return 0;
  }
  func("JsParser reading from file %s",script_file);
  while(fgets(line,512,fd)) {
    c++;
    func("%03i : %s",c,line);
    
    JSBool ok = JS_EvaluateScript (js_context, global_object,
				   line, strlen(line), script_file, c, &ret_val);

    if(ok!=JS_TRUE) {
      error("JsParser::open : %s : error evaluating script:",script_file);
      error("%03i : %s",c,line);
    }

  }

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


JS(cafudda) {
  double *seconds = JSVAL_TO_DOUBLE(argv[0]);
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  func("cafudda for %f seconds",*seconds);
  env->cafudda(*seconds);

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

    jslayer = JSVAL_TO_OBJECT(argv[0]);
    if(!jslayer) {
      error("JsParser :: remove_layer called with NULL argument");
      return JS_FALSE;
    }

    func("JsParser :: layer JSObject : %p",jslayer);
    Layer *lay;
    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) {
      error("JsParser :: remove_layer : Layer core data is null");
      return JS_FALSE;
    }
    /** remove layer in real life */
    if(lay) {
	lay->rem();
	delete lay;
	lay = NULL;
    }
    return JS_TRUE;
}

JS(add_layer) { 
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    Layer *lay;
    JSObject *jslayer;
    *rval=JSVAL_FALSE;

    if(JSVAL_IS_NULL(argv[0])) {
      error("JsParser :: add_layer called with NULL argument");
      return JS_FALSE;
    }
    jslayer = JSVAL_TO_OBJECT(argv[0]);

    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) {
      error("JsParser :: add_layer : Layer core data is null");
      return JS_FALSE;
    }

    /** really add layer */
    if(lay->init(env)) {
      env->layers.add(lay);
      //      env->layers.sel(0); // deselect others
      //      lay->sel(true);
    } else delete lay;
    *rval=JSVAL_TRUE;

    return JS_TRUE;
}

JS(fastrand) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  *rval = INT_TO_JSVAL( fastrand() );
  
  return JS_TRUE;
}
JS(fastsrand) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) return JS_FALSE;

  int seed = JSVAL_TO_INT(argv[0]);
  fastsrand(seed);

  return JS_TRUE;
}

   
JS(layer_constructor) { 
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  //    JSObject *this_obj;
  
  Layer *layer;
  
  if(argc < 1)
    return JS_TRUE;
  else
    layer=create_layer(JS_GetStringBytes(JS_ValueToString(cx,argv[0])));
  if(layer==NULL)
    return JS_FALSE;
  
  //    this_obj = JS_NewObject(cx, &layer_class, NULL, obj); 
  if (!JS_SetPrivate(cx, obj, (void *) layer)) {
    error("JsParser::layer_constructor : couldn't set the private value"); 
    return JS_FALSE;
  }
  *rval = OBJECT_TO_JSVAL(obj);
  //   func("this_obj JSObject : %p",this_obj);
  //    func("obj JSObject : %p",obj);
  return JS_TRUE;
}

JS(filter_constructor) { 
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  Filter *filter;
  char *filter_string;
  
  filter_string=JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(argc < 1)
    return JS_TRUE;
  else
    filter=env->plugger.pick(filter_string);
  if(filter==NULL) {
    error("JsParser::filter_constructor : filter not found :%s",filter_string); 
    return JS_FALSE;
  }
  
  if (!JS_SetPrivate(cx, obj, (void *) filter)) {
    error("JsParser::filter_constructor : couldn't set the private value"); 
    return JS_FALSE;
  }
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

  GET_LAYER(Layer);
  
  char *blit_type=JS_GetStringBytes(JS_ValueToString(cx,argv[0]));
  if(!blit_type) {
    error("JsParser :: set_blit called with NULL argument");
    return JS_FALSE;
  }
  lay->blitter.set_blit(blit_type);
    
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

    if(argc<2) return JS_FALSE;
    
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

    if(argc<1) return JS_FALSE;

    GET_LAYER(Layer);

    int new_alpha=JSVAL_TO_INT(argv[0]);
    lay->blitter.set_value(new_alpha);

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

    lay->hidden = false;

    return JS_TRUE;
}
JS(layer_deactivate) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    lay->hidden = true;

    return JS_TRUE;
}
JS(add_filter) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter=NULL;
    Filter *filter;
    Layer *lay;

    jsfilter = JSVAL_TO_OBJECT(argv[0]);
    if(!jsfilter) {
      error("JsParser :: add_layer called with NULL argument");
      return JS_FALSE;
    }

    /**
     * Extract filter and layer pointers from js objects
     */
    filter = (Filter *) JS_GetPrivate(cx, jsfilter);
    lay = (Layer *) JS_GetPrivate(cx, obj);
    if(!lay || !filter) {
      error("JsParser :: Layer core data is null");
      return JS_FALSE;
    }
    else {
	if(!filter->init(&lay->geo)) {
	    error("Filter %s can't initialize",filter->getname());
	    return 0;
	}
	lay->filters.add(filter);
    }
    return JS_TRUE;
}
JS(rem_filter) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter=NULL;
    Filter *filter;
    Layer *lay;

    /**
     * TODO overloading with filter position
     */
    if(JSVAL_IS_OBJECT(argv[0])) {
	jsfilter = JSVAL_TO_OBJECT(argv[0]);
	if(!jsfilter) {
	    error("JsParser :: rem_layer called with NULL argument");
	    return JS_FALSE;
	}

	/**
	 * Extract filter pointers from js objects
	 */
	filter = (Filter *) JS_GetPrivate(cx, jsfilter);
	lay = (Layer *) JS_GetPrivate(cx, obj);
	if(!lay || !filter) {
	    error("JsParser :: Layer core data is null");
	    return JS_FALSE;
	}
	else {
	    filter->rem();
	    lay->filters.sel(0);
	    filter->clean();
	    filter = NULL;
	}
    }
    return JS_TRUE;
}


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

#ifdef WITH_AVIFILE
////////////////////////////////
// Avi Layer methods
JS_CONSTRUCTOR("AviLayer",avi_layer_constructor,AviLayer);
JS(avi_layer_forward) { return JS_TRUE; }
JS(avi_layer_rewind) { return JS_TRUE; }
JS(avi_layer_mark_in) { return JS_TRUE; }
JS(avi_layer_mark_out) { return JS_TRUE; }
JS(avi_layer_pos) { return JS_TRUE; }
JS(avi_layer_pause) { return JS_TRUE; }
#endif

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

  int size = JSVAL_TO_INT(argv[0]);
  lay->set_character_size(size);

  return JS_TRUE;
}
JS(txt_layer_font) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);  

  if(argc<1) return JS_FALSE;

  GET_LAYER(TxtLayer);

  int font = JSVAL_TO_INT(argv[0]);
  lay->set_font(font);

  return JS_TRUE;
}
JS(txt_layer_next) { return JS_TRUE; }
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

////////////////////////////////
// Png Layer methods
JS_CONSTRUCTOR("PngLayer",png_layer_constructor,PngLayer);

#endif
