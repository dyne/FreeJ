/*  FreeJ
 *  (c) Copyright 2004 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
 *
 */

#include <jsparser.h>
#include <context.h>

#include <string.h>


static Context *env;

JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
JSBool add_filter(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

JsParser::JsParser(Context *_env) {
    if(_env!=NULL)
	env=_env;
    init();
    parse_count=0;
    notice("JsParser::JsParser created");
}

JsParser::~JsParser() {
    /** The world is over */
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
    notice("JsParser::close()");
}

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

void JsParser::init() {
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

    /* Now initialize our class. */
    JS_InitClass(js_context, global_object, NULL,
		 &layer_class, layer_constructor,
		 0, NULL, NULL, NULL, NULL);

//    JS_DefineProperties(js_context, layer_object, layer_properties);

    /* Declare shell functions */
    if (!JS_DefineFunctions(js_context, global_object, shell_functions)) {
	error("JsParser :: error defining shell functions");
	return ;
    }
    return ;
}

JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    JSObject *this_obj;
    func("JsParser::layer_constructor()");
    Layer *layer;

    if(argc < 1)
	return JS_TRUE;
    else
	layer=create_layer(JS_GetStringBytes(JS_ValueToString(cx,argv[0])));
    if(layer==NULL)
	return JS_FALSE;

//    this_obj = JS_NewObject(cx, &layer_class, NULL, obj); 
    if (!JS_SetPrivate(cx, obj, (void *) layer)) {
	 error("JsParser::layer_constructor : COULDN't SET THE PRIVATE VALUE"); 
	 return JS_FALSE;
    }
    *rval = OBJECT_TO_JSVAL(obj);
 //   func("this_obj JSObject : %p",this_obj);
    func("obj JSObject : %p",obj);
    return JS_TRUE;
}
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    func("JsParser :: add_layer()");
    JSObject *jslayer;

    jslayer = JSVAL_TO_OBJECT(argv[0]);
    if(!jslayer) {
      error("add_layer called with NULL argument");
      return JS_FALSE;
    }
    //    if(JSVAL_IS_OBJECT(jslayer)) {
    //	error("JsParser:add_layer : Il primo argomento non e' un oggetto %p",jslayer);
    //	return JS_FALSE;
    //    }

    func("layer JSObject : %p",jslayer);
    Layer *lay;
    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) {
      error("JsParser::add_layer : Layer core data is null");
      return JS_FALSE;
    }

    if(lay->init(env)) {
      env->layers.add(lay);
      env->layers.sel(0); /* deselect others */
      lay->sel(true);
    } else delete lay;

    return JS_TRUE;
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
				   line, strlen(line), script_file, 2, &ret_val);

    if(ok!=JS_TRUE) {
      error("JsParser::open : %s : error evaluating script:");
      error("%03i : %s",c,line);
    }

  }

  return ret_val;
}
/*
=======
/**
 * XXX TODO una cifra di work (translation: a LOOOOT of work ;)

int JsParser::parse(){
//    char *script="kolos(32,\"pippo\")";
    char *script="add_layer(new Layer(\"/home/kysucix/video/rotolascia.avi\"))";
    //    char *script="var pippo=new Layer(\"/home/kysucix/video/Flower.png\"); add_layer(pippo)";
    if(parse_count>0)
    return 0;
    parse_count++;
    func("JsParser::parse()");
    jsval ret_val;
    
    JSBool ok = JS_EvaluateScript(js_context, global_object, script, strlen(script), "refugees.js", 1, &ret_val);
    }
*/
