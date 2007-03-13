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

DECLARE_CLASS("Layer",layer_class,layer_constructor);

JSFunctionSpec layer_methods[] = {
  LAYER_METHODS  ,
  ENTRY_METHODS  ,
  {0}
};


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


JS(list_layers) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;
  
  Layer *lay;
  jsval val;
  int c = 0;
  
  if( freej.screen->layers.len() == 0 ) {
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  lay = (Layer*)freej.screen->layers.begin();
  while(lay) {
    objtmp = JS_NewObject(cx, lay->jsclass, NULL, obj);

    JS_SetPrivate(cx,objtmp,(void*) lay);

    val = OBJECT_TO_JSVAL(objtmp);

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

  if( freej.screen->layers.len() == 0 ) {
    error("can't return selected layer: no layers are present");
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }
  
  lay = (Layer*)freej.screen->layers.begin();

  while(lay) {
    if(lay->select) break;
    else lay = (Layer*) lay->next;
  }

  if(!lay) {
    warning("there is no selected layer");
    *rval = JSVAL_FALSE;
    return JS_TRUE;
  }

  objtmp = JS_NewObject(cx, lay->jsclass, NULL, obj);

  JS_SetPrivate(cx, objtmp, (void*) lay);

  *rval = OBJECT_TO_JSVAL( objtmp );

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

    *rval=INT_TO_JSVAL(lay->geo.x);

    return JS_TRUE;
}
JS(layer_get_y_position) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->geo.y);

    return JS_TRUE;
}
JS(layer_get_width) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->geo.w);

    return JS_TRUE;
}
JS(layer_get_height) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    GET_LAYER(Layer);

    *rval=INT_TO_JSVAL(lay->geo.h);

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
