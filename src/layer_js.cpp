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

#include <jsparser.h>
#include <callbacks_js.h>
#include <jsparser_data.h>
#include <layer.h>
//#include <fps.h>
#include <blitter.h>

void js_layer_gc (JSContext *cx, JSObject *obj);

DECLARE_CLASS("Layer",layer_class,layer_constructor);

JSFunctionSpec layer_methods[] = {
    ENTRY_METHODS,
    {"activate",	layer_activate,	        0},
    {"deactivate",	layer_deactivate,	0},
    {"start",		layer_start,		0},
    {"stop",		layer_stop,		0},
    {"set_blit",	layer_set_blit,	        1},
    {"get_blit",	layer_get_blit,	        0},
    {"set_blit_value",	layer_set_blit_value,	1},
    {"get_blit_value",	layer_get_blit_value,	0},
    {"fade_blit_value", layer_fade_blit_value,  2},
    {"set_position",	layer_set_position,	2},
    {"add_filter",      layer_add_filter,	1},
    {"rem_filter",	layer_rem_filter,	1},
    {"rotate",          layer_rotate,           1},
    {"zoom",            layer_zoom,             2},
    {"fit",             layer_fit,              0},
    {0}
};

JSPropertySpec layer_properties[] = {
  // r/w
  { "x",    0, JSPROP_ENUMERATE | JSPROP_PERMANENT, layer_get_x, layer_set_x },
  { "y",    1, JSPROP_ENUMERATE | JSPROP_PERMANENT, layer_get_y, layer_set_y },
  { "fps",  2, JSPROP_ENUMERATE | JSPROP_PERMANENT, layer_get_fps, layer_set_fps },
  { "name", 3, JSPROP_ENUMERATE | JSPROP_PERMANENT, layer_get_name, layer_set_name },
  // ro
  { "filename",   4, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, layer_get_filename, NULL },
  { "w",          5, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, layer_get_width, NULL },
  { "h" ,         6, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, layer_get_height, NULL },
  { "filters",    7, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, layer_list_filters, NULL },
  { "parameters", 8, JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY, layer_list_parameters, NULL },
  {0}
};


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

	if(l) {
	  func("js gc deleting layer %s", l->name);
	  //	l->data = NULL; // Entry~ calls free(data)
	  delete l;
	}

}

JS(layer_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  //    JSObject *this_obj;
  char *filename;

  Layer *layer;
  /*  if (jsclass_s != OBJ_GET_CLASS(cx, obj)) {
    JS_ERROR("Sorry, this gimmik is not supported.");
    } */

  if(argc < 1)
      JS_ERROR("missing argument");

  // recognize the extension and open the file given in argument
  filename = js_get_string(argv[0]);

  layer = global_environment->open( filename );
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

JS(selected_layer) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  Layer *lay;
  JSObject *objtmp;
  jsval val;

  if( global_environment->screens.selected()->layers.len() == 0 ) {
    error("can't return selected layer: no layers are present");
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }
  
  lay = (Layer*)global_environment->screens.selected()->layers.selected();

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




////////////////////////////////
// Generic Layer methods

JS(layer_set_blit) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char *blit_name;

  GET_LAYER(Layer);

  blit_name = js_get_string(argv[0]);

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

JS(layer_set_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    if(argc<2)
        JS_ERROR("missing argument");
    GET_LAYER(Layer);

    int32 x, y;
    x = js_get_int(argv[0]);
    y = js_get_int(argv[1]);
    func("set position x:%i y:%u", x, y);
    lay->set_position(x, y);

    return JS_TRUE;
}

JS(layer_set_blit_value) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    if(argc<1)
        JS_ERROR("missing argument");
    jsint value = js_get_double(argv[0]);

    GET_LAYER(Layer);

    Parameter *p;
    // when this function is used we
    // assume blit has only one parameter
    // (basically we keep this for backward compat)

    if(!lay->current_blit)
      error("layer %s has no blit selected (not added yet?)", lay->name);
    else
      p = lay->current_blit->parameters.begin();

    if(!p) 
      warning("no blit parameter found on layer %s", lay->name);
    else
      p->set((void*)&value);

    return JS_TRUE;
}
JS(layer_fade_blit_value) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  if(argc<2)
      JS_ERROR("missing argument");
  jsint value = js_get_double(argv[0]);
  jsint step = js_get_double(argv[1]);

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
  
  JSObject *jsfilter = NULL;
  FilterInstance *filter_instance = NULL;

  JS_BeginRequest(cx);

  if(argc<1)
      JS_ERROR("missing argument");
  //  js_is_instanceOf(&filter_class, argv[0]);

  jsfilter = JSVAL_TO_OBJECT(argv[0]);
  if (!jsfilter) {
      error("Filter %s nt found", JSVAL_TO_STRING(argv[0]));
      JS_EndRequest(cx);
      JS_ClearContextThread(cx);
      return JS_FALSE;
  }
  /**
   * Extract filter and layer pointers from js objects
   */
  filter_instance = (FilterInstance *) JS_GetPrivate(cx, jsfilter);
  if(!filter_instance)
      JS_ERROR("Effect is NULL");

  GET_LAYER(Layer);
  JS_EndRequest(cx);

  if(filter_instance->inuse()) {
    error("filter %s is already in use", filter_instance->proto->name);
    return JS_TRUE;
  }
  
  filter_instance->apply(lay);
  
  return JS_TRUE;
}

JS(layer_rem_filter) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jsfilter = NULL;
    FilterInstance *filter_instance = NULL;

    if(argc<1)
        JS_ERROR("missing argument");
    //    js_is_instanceOf(&filter_class, argv[0]);

    /** TODO overload with filter name and position */
    jsfilter = JSVAL_TO_OBJECT(argv[0]);
    if(!jsfilter)
        JS_ERROR("missing argument");

    filter_instance = (FilterInstance *) JS_GetPrivate(cx, jsfilter);
    if(!filter_instance)
        JS_ERROR("Effect data is NULL");

    filter_instance->rem();
    //delete filter_instance;
    //duo->instance = NULL;

    return JS_TRUE;
}

/// rotozooming the layer
JS(layer_rotate) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1)
      JS_ERROR("missing argument");

  js_debug_argument(cx, argv[0]);

  jsdouble degrees = js_get_double(argv[0]);


  GET_LAYER(Layer);

  lay->set_rotate(degrees);

  return JS_TRUE;
}
JS(layer_zoom) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(argc<1)
      JS_ERROR("missing argument");
  // take ymang=xmagn on .zoom(val)
  jsdouble xmagn, ymagn;
  xmagn = js_get_double(argv[0]);
  if(argc>1) ymagn = js_get_double(argv[1]);
  else ymagn = xmagn;
  
  GET_LAYER(Layer);

  lay->set_zoom(xmagn,ymagn);

  return JS_TRUE;
}

JS(layer_start) {
  GET_LAYER(Layer);
  lay->active = true;
  if(!lay->is_running()) lay->start();

  return JS_TRUE;
}

JS(layer_stop) {
  GET_LAYER(Layer);
  lay->active = false;
  lay->stop();

  return JS_TRUE;
}

JS(layer_fit) {
  GET_LAYER(Layer);
  lay->fit();
  return JS_TRUE;
}


/////////////////////////////////////////
//// Layer Properties

JSP(layer_set_x) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    jsint nx = js_get_int(*vp);
    lay->set_position(nx, lay->geo.y);
    return JS_TRUE;
    //    return JS_NewNumberValue(cx, (double)lay->geo.x, rval);
}
JSP(layer_get_x) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    return JS_NewNumberValue(cx, (jsint)lay->geo.x, vp);    
}
JSP(layer_set_y) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    jsint ny = js_get_int(*vp);
    lay->set_position(lay->geo.x, ny);
    return JS_TRUE;
}
JSP(layer_get_y) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    return JS_NewNumberValue(cx, (jsint)lay->geo.y, vp);    
}
JSP(layer_get_width) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    return JS_NewNumberValue(cx, (jsint)lay->geo.w, vp);
}
JSP(layer_get_height) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    return JS_NewNumberValue(cx, (jsint)lay->geo.h, vp);
}
JSP(layer_get_filename) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    JSString *str = JS_NewStringCopyZ(cx, lay->get_filename());
    *vp = STRING_TO_JSVAL(str);
    return JS_TRUE;
}
JSP(layer_set_name) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    char *nn = js_get_string(*vp);
    lay->set_name(nn);
    return JS_TRUE;
}  
JSP(layer_get_name) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    JSString *nn = JS_NewStringCopyZ(cx, lay->get_name());
    *vp = STRING_TO_JSVAL(nn);
    return JS_TRUE;
}  
JSP(layer_set_fps) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    jsint nf = js_get_int(*vp);
    lay->fps.set(nf);
    return JS_TRUE;
}
JSP(layer_get_fps) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    GET_LAYER(Layer);
    return JS_NewNumberValue(cx, (jsint)lay->fps.get(), vp);    
}

JSP(layer_list_filters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;

  FilterInstance *filter_instance;

  jsval val;
  int c = 0;

  GET_LAYER(Layer);

  // no effects
  if(lay->filters.len() == 0) {
    *vp = JSVAL_FALSE;
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr)
    return JS_FALSE;


  filter_instance = (FilterInstance*)lay->filters.begin();

  while(filter_instance) {
  
    objtmp = JS_NewObject(cx, &filter_class, NULL, obj);
    
    JS_SetPrivate(cx, objtmp, (void*) filter_instance);

    val = OBJECT_TO_JSVAL(objtmp);

    JS_SetElement(cx, arr, c, &val );
    
    c++;

    filter_instance = (FilterInstance*)filter_instance->next;
  }

  *vp = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}  

JSP(layer_list_parameters) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;

  GET_LAYER(Layer);
  if(!lay->parameters) {
    *vp = JSVAL_FALSE;
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

  *vp = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}
