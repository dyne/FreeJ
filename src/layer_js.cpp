/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
#include <layer.h>
//#include <fps.h>
#include <blitter.h>

DECLARE_CLASS_GC("Layer",layer_class,layer_constructor, js_layer_gc);

JSFunctionSpec layer_methods[] = {
    ENTRY_METHODS,
    {"activate",	layer_activate,	        0},
    {"deactivate",	layer_deactivate,	0},
    {"start",		layer_start,		0},
    {"stop",		layer_stop,		0},
    {"get_name",	layer_get_name,	        0},
    {"get_filename",	layer_get_filename,	0},
    {"set_blit",	layer_set_blit,	        1},
    {"get_blit",	layer_get_blit,	        0},
    {"set_blit_value",	layer_set_blit_value,	1},
    {"get_blit_value",	layer_get_blit_value,	0},
    {"fade_blit_value", layer_fade_blit_value,  2},
    {"set_position",	layer_set_position,	2},
    {"set_fps",		layer_set_fps,		0},
    {"get_fps",		layer_get_fps,   	0},
    {"slide_position",  layer_slide_position,   2},
    {"get_x_position",	layer_get_x_position,	0},
    {"x",               layer_get_x_position,   0},
    {"get_y_position",	layer_get_y_position,	0},
    {"y",               layer_get_y_position,   0},
    {"get_width",       layer_get_width,        0},
    {"w",               layer_get_width,        0},
    {"get_height",      layer_get_height,       0},
    {"h",               layer_get_height,       0},
    {"add_filter",      layer_add_filter,	1},
    {"rem_filter",	layer_rem_filter,	1},
    {"rotate",          layer_rotate,           1},
    {"zoom",            layer_zoom,             2},
    {"spin",            layer_spin,             2},
    {"list_filters",    layer_list_filters,     0},
    {"list_parameters", layer_list_parameters,  0},
    {0}
};

void *Layer::js_constructor(Context *env, JSContext *cx, JSObject *obj,
			    int argc, void *aargv, char *err_msg) {

  char *filename;

  uint16_t width  = env->screen->w;
  uint16_t height = env->screen->h;

  jsval *argv = (jsval*)aargv;

  if(argc==0) {
    if(!init(env)) {
      sprintf(err_msg, "Layer constructor failed initialization");
      return NULL;    }

  } else if(argc==1) {
    JS_ARG_STRING(filename,0);
    if(!init(env)) {
      sprintf(err_msg, "Layer constructor failed initialization");
      return NULL;    }

    if(!open(filename)) {
      snprintf(err_msg, MAX_ERR_MSG, "Layer constructor failed open(%s): %s",
	       filename, strerror(errno));
      return NULL;    }
    
  } else if(argc==2) {
    JS_ValueToUint16(cx, argv[0], &width);
    JS_ValueToUint16(cx, argv[1], &height);
    if(!init(env, width, height)) {
      snprintf(err_msg, MAX_ERR_MSG,
	       "Layer constructor failed initialization w[%u] h[%u]", width, height);
      return NULL;
    }
    
  } else if(argc==3) {
    JS_ValueToUint16(cx, argv[0], &width);
    JS_ValueToUint16(cx, argv[1], &height);
    JS_ARG_STRING(filename,2);
    if(!init(env, width, height)) {
      snprintf(err_msg, MAX_ERR_MSG,
	       "Layer constructor failed initializaztion w[%u] h[%u]", width, height);
      return NULL;
    }
    if(!open(filename)) {
      snprintf(err_msg, MAX_ERR_MSG,
	       "Layer constructor failed initialization (%s): %s", filename, strerror(errno));
      return NULL;
    }
    
  } else {
    sprintf(err_msg,
	    "Wrong numbers of arguments\n use (\"filename\") or (width, height, \"filename\") or ()");
    return NULL;
  }
  if(!JS_SetPrivate(cx,obj,(void*)this)) {
    sprintf(err_msg, "%s", "JS_SetPrivate failed");
    return NULL;
  }

  jsobj = obj; // save the JS instance object into the C++ instance object

  return (void*)OBJECT_TO_JSVAL(obj);

}

JS(layer_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  //    JSObject *this_obj;
  char *filename;

  Layer *layer;
  /*  if (jsclass_s != OBJ_GET_CLASS(cx, obj)) {
    JS_ERROR("Sorry, this gimmik is not supported.");
    } */

  if(argc < 1) JS_ERROR("missing argument");

  // recognize the extension and open the file given in argument
  JS_ARG_STRING(filename,0);

  layer = create_layer( env, filename );
  if(!layer) {
    error("%s: cannot create a Layer using %s",__FUNCTION__,filename);
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
                     JSSMSG_FJ_CANT_CREATE, filename,
                     strerror(errno));
    return JS_FALSE;
  }

  //*rval is obj but wrong class. so we cheat here our autodetect ...
  JSObject *thisobj = JS_NewObject(cx, layer->jsclass, NULL, NULL);
  if (!JS_SetPrivate(cx, thisobj, (void *) layer))
    JS_ERROR("internal error setting private value");

  *rval = OBJECT_TO_JSVAL(thisobj);
  return JS_TRUE;
}

JS(layer_set_fps) {
	GET_LAYER(Layer);
	int fps_old = lay->fps.get();

	if(argc==1) {
		JS_ARG_NUMBER(fps, 0);
		fps_old = lay->fps.set(fps);
	}

	return JS_NewNumberValue(cx, fps_old, rval);
}

JS(layer_get_fps) {
	GET_LAYER(Layer);
	double fps = double(lay->fps.get());
	return JS_NewNumberValue(cx, fps, rval);
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
    if (lay->data) {
func("reusing %p", lay->data);
    	val = (jsval)lay->data;
    } else {
func("new JS Object");
	objtmp = JS_NewObject(cx, lay->jsclass, NULL, obj);

	JS_SetPrivate(cx,objtmp,(void*) lay);

	val = OBJECT_TO_JSVAL(objtmp);

	lay->data = (void*)val;
    }

    JS_SetElement(cx, arr, c, &val );
    
    c++;
    lay = (Layer*)lay->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

JS(selected_layer) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  Layer *lay;
  JSObject *objtmp;
  jsval val;

  if( env->layers.len() == 0 ) {
    error("can't return selected layer: no layers are present");
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }
  
  lay = (Layer*)env->layers.selected();

  if(!lay) {
    warning("there is no selected layer");
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }
    if (lay->data) {
    	val = (jsval)lay->data;
    } else {
	objtmp = JS_NewObject(cx, lay->jsclass, NULL, obj);
	func("create: %s", lay->jsclass->name);
	JS_SetPrivate(cx, objtmp, (void*) lay);
	val = OBJECT_TO_JSVAL(objtmp);
	lay->data = (void*)val;
    }

  *rval = val;

  return JS_TRUE;
}
  
JS(layer_list_filters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;

  FilterDuo *duo;

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

  duo = new FilterDuo();

  duo->instance = (FilterInstance*)lay->filters.begin();

  while(duo->instance) {

    duo->proto = duo->instance->proto;
  
    objtmp = JS_NewObject(cx, &filter_class, NULL, obj);
    
    JS_SetPrivate(cx, objtmp, (void*) duo);

    val = OBJECT_TO_JSVAL(objtmp);

    JS_SetElement(cx, arr, c, &val );
    
    c++;

    duo->instance = (FilterInstance*)duo->instance->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}  

JS(layer_list_blits) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;
  int c = 0;
  Entry *b;

  GET_LAYER(Layer);
  if(!lay) {
    *rval = JSVAL_FALSE;
    return JS_FALSE;
  }
  
  arr = JS_NewArrayObject(cx, 0, NULL); //create void array
  if(!arr) return JS_FALSE;
  
  b = lay->blitter->blitlist.begin();
  while(b) {
    
    str = JS_NewStringCopyZ(cx, b->name);
    val = STRING_TO_JSVAL(str);
    JS_SetElement(cx, arr, c, &val);
    c++;
    b = b->next;
  }
  
  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

JS(layer_list_parameters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;


  GET_LAYER(Layer);
  if(!lay->parameters) {
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); //create void array
  if(!arr) return JS_FALSE;

  Parameter *parm = (Parameter*)lay->parameters->begin();
  int c = 0;
  while(parm) {
    str = JS_NewStringCopyZ(cx, parm->name);
    val = STRING_TO_JSVAL(str);
    JS_SetElement(cx, arr, c, &val);
    c++;
    parm = (Parameter*)parm->next;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}



////////////////////////////////
// Generic Layer methods

JS(layer_set_blit) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char *blit_name;

  GET_LAYER(Layer);

  JS_ARG_STRING(blit_name, 0);

  lay->set_blit( blit_name );

  return JS_TRUE;
}

JS(layer_get_blit) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    char *blit_type=lay->current_blit->name;
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

    JS_ARG_NUMBER(x,0);
    JS_ARG_NUMBER(y,1);

    lay->set_position((int)x, (int)y);

    return JS_TRUE;
}


JS(layer_slide_position) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(2);
  
  GET_LAYER(Layer);
  
  int speed = 1;
  int x,y;

  x = JSVAL_TO_INT(argv[0]);
  y = JSVAL_TO_INT(argv[1]);
  
  if(argc == 3)
    speed = JSVAL_TO_INT(argv[2]);
  
  lay->slide_position(x, y, speed);

  return JS_TRUE;
}

JS(layer_get_x_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    return JS_NewNumberValue(cx, (double)lay->geo.x, rval);

}
JS(layer_get_y_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    return JS_NewNumberValue(cx, (double)lay->geo.y, rval);
}
JS(layer_get_width) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

	return JS_NewNumberValue(cx, lay->geo.w, rval);
}
JS(layer_get_height) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

	return JS_NewNumberValue(cx, lay->geo.h, rval);
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
    //    lay->blitter.set_value((float)value);

    return JS_TRUE;
}
JS(layer_fade_blit_value) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  if(argc<2) JS_ERROR("missing argument");
  JS_ARG_NUMBER(value,0);
  JS_ARG_NUMBER(step,1);

  GET_LAYER(Layer);

  value = 255.0*value;
  if(value>255) {
    warning("blit values should be float ranged between 0.0 and 1.0");
    value = 255;
  }
  
  //  lay->blitter.fade_value((float)step, (float)value);

  return JS_TRUE;
}

JS(layer_get_blit_value) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

	return JS_NewNumberValue(cx, lay->current_blit->value, rval);
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
JS(layer_add_filter) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JSObject *jsfilter=NULL;
  FilterDuo *duo;

  if(argc<1) JS_ERROR("missing argument");
  js_is_instanceOf(&filter_class, argv[0]);

  jsfilter = JSVAL_TO_OBJECT(argv[0]);
  /**
   * Extract filter and layer pointers from js objects
   */
  duo = (FilterDuo *) JS_GetPrivate(cx, jsfilter);
  if(!duo) JS_ERROR("Effect is NULL");

  if(duo->instance) {
    error("filter %s is already in use", duo->proto->name);
    return JS_TRUE;
  }
  GET_LAYER(Layer);
  
  duo->instance = duo->proto->apply(lay);
  
  return JS_TRUE;
}

JS(layer_rem_filter) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter=NULL;
    FilterDuo *duo;

    if(argc<1) JS_ERROR("missing argument");
    js_is_instanceOf(&filter_class, argv[0]);

    /** TODO overload with filter name and position */
    jsfilter = JSVAL_TO_OBJECT(argv[0]);
    if(!jsfilter) JS_ERROR("missing argument");

    duo = (FilterDuo *) JS_GetPrivate(cx, jsfilter);
    if(!duo) JS_ERROR("Effect data is NULL");

    duo->instance->rem();
    delete duo->instance;
    duo->instance = NULL;

    return JS_TRUE;
}

/// rotozooming the layer
JS(layer_rotate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) JS_ERROR("missing argument");
  JS_ARG_NUMBER(degrees,0);

  GET_LAYER(Layer);

  lay->set_rotate(degrees);

  return JS_TRUE;
}
JS(layer_zoom) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1) JS_ERROR("missing argument");
  // take ymang=xmagn on .zoom(val)
  JS_ARG_NUMBER(xmagn,0);
  JS_ARG_NUMBER(ymagn,argc == 1 ? 0 : 1);
  
  GET_LAYER(Layer);

  lay->set_zoom(xmagn,ymagn);

  return JS_TRUE;
}
JS(layer_spin) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<2) JS_ERROR("missing argument");
  JS_ARG_NUMBER(rot,0);
  JS_ARG_NUMBER(magn,1);

  GET_LAYER(Layer);

  lay->set_spin(rot, magn);

  return JS_TRUE;
}

void js_layer_gc (JSContext *cx, JSObject *obj) {
	func("%s",__PRETTY_FUNCTION__);
	Layer* l;
	if (!obj) {
		error("%n called with NULL object", __PRETTY_FUNCTION__);
		return;
	}
	// This callback is declared in Layer Class only,
	// we can skip the typecheck of obj, can't we?
	l = (Layer *) JS_GetPrivate(cx, obj);
	JSClass *jc = JS_GET_CLASS(cx,obj);

	if (l) {
		func("JSvalcmp(%s): %p / %p Layer: %p", jc->name, OBJECT_TO_JSVAL(obj), l->data, l);
		if(l->list) {
		  func("JSgc: Layer %s/%s is still on stage", jc->name, l->name);
		  //l->data = NULL;
		  //		  delete l;
		} else {
		  func("JSgc: Layer %s/%s is useless, deleting", jc->name, l->name);
		  l->data = NULL; // Entry~ calls free(data)
		  l->stop();
		  delete l;
		}
	} else {
		func("Mh, object(%s) has no private data", jc->name);
	}
}

JS(layer_start) {
  GET_LAYER(Layer);
  lay->active = true;
  if(!lay->running) lay->start();

  return JS_TRUE;
}

JS(layer_stop) {
  GET_LAYER(Layer);
  lay->active = false;
  lay->stop();

  return JS_TRUE;
}
