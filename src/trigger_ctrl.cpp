/*  FreeJ - Trigger controller
 *
 *  (c) Copyright 2007 Christoph Rudorff <goil@dyne.org>
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
 * "$Id: trigger_ctrl.cpp 881 2007-08-02 01:06:11Z mrgoil $"
 *
 */


#include <string.h>

#include <trigger_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>


#include <callbacks_js.h> // javascript
#include <jsparser.h>
#include <jsparser_data.h>

JS(js_trigger_ctrl_constructor);

DECLARE_CLASS("TriggerController",js_trigger_ctrl_class, js_trigger_ctrl_constructor);

JSFunctionSpec js_trigger_ctrl_methods[] = {
  {0}
};

FACTORY_REGISTER_INSTANTIATOR(Controller, TriggerController, TriggerController, core);

TriggerController::TriggerController()
  :Controller() {
    set_name("Trigger");
}

TriggerController::~TriggerController() {
}

int TriggerController::poll() {

  if(javascript) {
    jsval ret = JSVAL_VOID;
    JSBool res;
    TriggerListener *listener = listeners.begin();
    while (listener) {
      listener->frame();
      listener = (TriggerListener *)listener->next;
    }
  }
  return 1;
}

int TriggerController::dispatch() {

  // if no javascript is initilized then this function
  // should be overridden by other binded languages

/*
    if (res == JS_FALSE) {
      error("trigger call frame() failed, deactivate ctrl");
      active = false;
    }
 */
    
  return(1);
}

bool TriggerController::add_listener(JSContext *cx, JSObject *obj)
{
    TriggerListener *listener = new TriggerListener(cx, obj);
    listeners.append(listener);
    return true;
}

JS(js_trigger_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  TriggerController *trigger = (TriggerController *)Factory<Controller>::get_instance( "TriggerController" );
  if (!trigger)
    return JS_FALSE;
  JS_BeginRequest(cx);
  // initialize with javascript context
  if (!trigger->initialized) {
      if(! trigger->init(global_environment) ) {
          error("failed initializing keyboard controller");
		  JS_EndRequest(cx);
          delete trigger; return JS_FALSE;
      }
        
      // assign the real js object
      //trigger->jsobj = obj;
      trigger->javascript = true;        
  } else {
      //obj = trigger->jsobj;
  }
	
  // mark that this controller was created by javascript
  trigger->javascript = true;

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)trigger) ) {
    error("failed assigning trigger controller to javascript");
    JS_EndRequest(cx);  
    delete trigger; return JS_FALSE;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  trigger->add_listener(cx, obj);
  JS_EndRequest(cx);  
  return JS_TRUE;
}


// TriggerListener

TriggerListener::TriggerListener(JSContext *cx, JSObject *obj)
{
    jsContext = cx;
    jsObject = obj;
    frameFunc = NULL;
}

TriggerListener::~TriggerListener()
{
}

bool TriggerListener::frame()
{
    jsval ret = JSVAL_VOID;
    JSBool res;

    JS_BeginRequest(jsContext);

    if (!frameFunc) {
        res = JS_GetProperty(jsContext, jsObject, "frame", &frameFunc);
        if(!res || JSVAL_IS_VOID(frameFunc)) {
            error("method frame not found in TriggerController"); 
            return false;
        }
    }
    res = JS_CallFunctionValue(jsContext, jsObject, frameFunc, 0, NULL, &ret);

    JS_EndRequest(jsContext);
    if (res == JS_FALSE) {
        error("trigger call frame() failed, deactivate ctrl");
        //active = false;
        return false;
    }
    return true;
}