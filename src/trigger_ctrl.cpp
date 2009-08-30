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



TriggerController::TriggerController()
  :Controller() {
    set_name("Trigger");
    frame_func = NULL;
}

TriggerController::~TriggerController() {
}

int TriggerController::poll() {

  if(javascript)
    if(!frame_func) {
      JSObject *objp = NULL;
      JSBool res;
      res = JS_GetMethod(jsenv, jsobj, "frame", &objp, &frame_func);
      if(!res || JSVAL_IS_VOID(frame_func)) {
	error("method frame not found in TriggerController"); 
	return(-1);
      }
    }

  return dispatch();
}

int TriggerController::dispatch() {

  // if no javascript is initilized then this function
  // should be overridden by other binded languages
  if(javascript) {
    jsval ret = JSVAL_VOID;
    JSBool res;
    
    res = JS_CallFunctionValue(jsenv, jsobj, frame_func, 0, NULL, &ret);
    if (res == JS_FALSE) {
      error("trigger call frame() failed, deactivate ctrl");
      frame_func = NULL;
      active = false;
    }
  }

    
  return(1);
}

JS(js_trigger_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  TriggerController *trigger = new TriggerController();

  // initialize with javascript context
  if(! trigger->init(global_environment) ) {
    error("failed initializing trigger controller");
    delete trigger; return JS_FALSE;
  }

  trigger->jsobj = obj;

  // mark that this controller was created by javascript
  trigger->javascript = true;

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)trigger) ) {
    error("failed assigning trigger controller to javascript");
    delete trigger; return JS_FALSE;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}
