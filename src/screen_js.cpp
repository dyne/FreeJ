/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio <jaromil@dyne.org>
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
 */



#include <callbacks_js.h>
#include <jsparser_data.h>
#include <screen.h>
#include <blitter.h>
#include <factory.h>

DECLARE_CLASS("Screen",screen_class,screen_constructor);

JSFunctionSpec screen_methods[] = {
  ENTRY_METHODS,
  {"init",              screen_init,            2},
  {"add_layer",         screen_add_layer,       1},
  {"rem_layer",         screen_rem_layer,       1},
  {"list_layers",       screen_list_layers,     0},
  {"is_initialized",    screen_initialized,     0}, 
  {0}
};

JS(screen_constructor) {
  func("%s",__PRETTY_FUNCTION__);
  char *type = NULL;
  ViewPort *screen = NULL;
  
  if(argc >= 1) {  
    // a specific screen type has been requested
    char *type = js_get_string(argv[0]);
    screen = Factory<ViewPort>::get_instance( "Screen", type );
  } else {
    // no screen type has been specified, return the default one
    screen = Factory<ViewPort>::get_instance( "Screen" );
  }

  if(!screen) {
    error("%s: cannot obtain current Screen",__FUNCTION__);
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
			 JSSMSG_FJ_CANT_CREATE, type,
			 strerror(errno));
    return JS_FALSE;
  }

  // if already existing, return the singleton
  if(screen->jsobj) {
    obj = screen->jsobj;
  } else {
    if (!JS_SetPrivate(cx, obj, (void *) screen))
        JS_ERROR("internal error setting private value");

    // assign the real js object
    screen->jsclass = &screen_class;
    screen->jsobj = obj;
  }
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

JS(screen_init) {
  func("%s",__PRETTY_FUNCTION__);
  
  JS_CHECK_ARGC(2);
  
  jsint w = js_get_int(argv[0]);
  jsint h = js_get_int(argv[1]);
    
  ViewPort *screen = (ViewPort*)JS_GetPrivate(cx,obj);
  if(!screen) {
    JS_ERROR("Screen core data is NULL");
    return JS_FALSE;
  }
  
  screen->init(w, h, 32); // hardcoded at 32bit bpp

  return JS_TRUE;
}

JS(screen_initialized) {
  func("%s",__PRETTY_FUNCTION__);
  ViewPort *screen = (ViewPort*)JS_GetPrivate(cx,obj);
  if(!screen) {
    JS_ERROR("Screen core data is NULL");
    return JS_FALSE;
  }
  *rval = BOOLEAN_TO_JSVAL(screen->initialized?JS_TRUE:JS_FALSE);
  return JS_TRUE;
}

JS(screen_list_layers) {
  func("%s",__PRETTY_FUNCTION__);
  JSObject *arr;
  JSObject *objtmp;
  
  Layer *lay;
  
  jsval val;
  int c = 0;
  
  ViewPort *screen = (ViewPort*)JS_GetPrivate(cx,obj);
  if(!screen) {
    JS_ERROR("Screen core data is NULL");
    return JS_TRUE;
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  // XXX check this carefully
  // caedes reports some weird problems after calling list_layers
  // looks like here might be the hairy point
  lay = screen->layers.begin();
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

JS(screen_add_layer) {
  func("%s",__PRETTY_FUNCTION__);

  JSObject *jslayer = NULL;
  Layer *lay;

  if(argc<1) JS_ERROR("missing argument");
  //  js_is_instanceOf(&layer_class, argv[0]);

  jslayer = JSVAL_TO_OBJECT(argv[0]);
  lay = (Layer*) JS_GetPrivate(cx, jslayer);
  if(!lay) JS_ERROR("Layer is NULL");

  ViewPort *screen = (ViewPort*)JS_GetPrivate(cx,obj);
  if(!screen) {
    JS_ERROR("Screen core data is NULL");
    return JS_TRUE;
  }

  screen->add_layer(lay);
  lay->start();

  return JS_TRUE;
}

JS(screen_rem_layer) {
  func("%s",__PRETTY_FUNCTION__);

  JSObject *jslayer = NULL;
  Layer *lay;

  if(argc<1) JS_ERROR("missing argument");
  //  js_is_instanceOf(&layer_class, argv[0]);

  jslayer = JSVAL_TO_OBJECT(argv[0]);
  lay = (Layer*) JS_GetPrivate(cx, jslayer);
  if(!lay) JS_ERROR("Layer is NULL");

  ViewPort *screen = (ViewPort*)JS_GetPrivate(cx,obj);
  if(!screen) {
    JS_ERROR("Screen core data is NULL");
    return JS_TRUE;
  }

  screen->rem_layer(lay);

  return JS_TRUE;
}

/*
void js_screen_gc (JSContext *cx, JSObject *obj) {
  func("%s",__PRETTY_FUNCTION__);
  return; // XXXX check js GC TODO
  ViewPort* view;
  if (!obj) {
    error("garbage collector called with NULL object: %s", __PRETTY_FUNCTION__);
    return;
  }
  // This callback is declared in Screen class only,
  // we can skip the typecheck of obj, can't we?
  view = (ViewPort *) JS_GetPrivate(cx, obj);
  if(view) {
    if(!view->deleted) {
      func("Javascript garbage collector deleting Screen %s", view->name);
      delete view;
    } else {
      func("Javascript object for Screen %s has been already deleted", view->name);
    }
  }
}


*/
