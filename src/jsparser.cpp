/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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

JsParser::JsParser(Context *_env) {
    if(_env!=NULL)
	env=_env;
    init();
    notice("JsParser::JsParser created");
}

JsParser::~JsParser() {
    /** The world is over */
    JS_DestroyContext(js_context);
    JS_DestroyRuntime(js_runtime);
    JS_ShutDown();
    notice("JsParser::close()");
}
void JsParser::init_structs() {
	layer_class.name="Layer";
	layer_class.flags=JSCLASS_NEW_RESOLVE;
	layer_class.addProperty=JS_PropertyStub;
	layer_class.delProperty=JS_PropertyStub;
	layer_class.getProperty=JS_PropertyStub;
	layer_class.setProperty=JS_PropertyStub;
	layer_class.enumerate=JS_EnumerateStub;
	layer_class.resolve=JS_ResolveStub;
	layer_class.convert=JS_ConvertStub;
	layer_class.finalize=JS_FinalizeStub;
//	layer_class.getObjectOps=NULL;
//	layer_class.checkAccess=NULL;
//	layer_class.construct=layer_constructor;

	global_class.name="global";
	global_class.flags=JSCLASS_NEW_RESOLVE;
	global_class.addProperty=JS_PropertyStub;
	global_class.delProperty=JS_PropertyStub;
	global_class.getProperty=JS_PropertyStub;
	global_class.setProperty=JS_PropertyStub;
	global_class.enumerate=JS_EnumerateStub;
	global_class.resolve=JS_ResolveStub;
	global_class.convert=JS_ConvertStub;
	global_class.finalize=JS_FinalizeStub;

	JSFunctionSpec shell_functions[] = {
	    /*    name          native          nargs    */
	    {"add_layer",         add_layer,        1},
	    {"kolos",         kolos,        2},
	    {0}
	};
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
    /** fill in class structures */
    init_structs();

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
//    JS_InitClass(js_context, global_object, NULL, &layer_class, layer_constructor, 0, NULL, NULL, NULL, NULL);

    JSObject *layer=JS_DefineObject(js_context,global_object,"Layer",&layer_class,NULL,0);
    if(layer==NULL) {
	error("JsParser :: error creating object");
	return ;
    }

    /* Declare shell functions */
    if (!JS_DefineFunctions(js_context, global_object, shell_functions)) {
	error("JsParser :: error defining shell functions");
	return ;
    }

    return ;
}

JSBool layer_constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    func("JsParser::layer_constructor()");
    Layer *layer;
    if(argc < 1)
	return JS_TRUE;
    else
	layer=create_layer(JS_GetStringBytes(JS_ValueToString(cx,argv[0])));

    if(layer==NULL)
	return JS_FALSE;
    JS_SetPrivate(cx,obj,layer);
    return JS_TRUE;
}
JSBool add_layer(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    func("JsParser::add_layer()");
    JSObject *layer=JSVAL_TO_OBJECT(argv[1]);
    Layer *lay;
    func("JsParser :: add_layer()");
    lay = (Layer *) JS_GetPrivate(cx, layer);
    if(!lay) return JS_TRUE;
    if(lay->init(env)) {
	if(lay) env->layers.add(lay);
	env->layers.sel(0); /* deselect others */
	lay->sel(true);
    } else delete lay;
}
int JsParser::open(const char* script_file) {
}
int JsParser::parse(){
    func("JsParser::parse()");
//    char *script="add_layer(new Layer(\"/home/ksycuxi/video/Flower.png\"))";
    char *script="kolos(32,\"pippo\")";
    jsval ret_val;

    JSBool ok = JS_EvaluateScript(js_context, global_object, script, strlen(script), "refugees.js", 1, &ret_val);
}
