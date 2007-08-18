/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Roio aka jaromil <jaromil@dyne.org>
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


// I used a Logitech WingMan here. It got 2 joys and 7 Buttons

#include <string.h>

#include <trigger_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>


#include <callbacks_js.h> // javascript

JS(js_trigger_ctrl_constructor);

DECLARE_CLASS("TriggerController",js_trigger_ctrl_class, js_trigger_ctrl_constructor);

JSFunctionSpec js_trigger_ctrl_methods[] = { {0} }; // all dynamic methods



TriggerCtrl::TriggerCtrl()
  :Controller() {
    set_name("Trigger");
}

TriggerCtrl::~TriggerCtrl() {
}

bool TriggerCtrl::init(JSContext *env, JSObject *obj) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  jsenv = env;
  jsobj = obj;
  
  initialized = true;
  return(true);
}

int TriggerCtrl::peep(Context *env) {
    poll(env);
}

int TriggerCtrl::poll(Context *env) {
  jsval ret;
  JS_CallFunctionName(jsenv, jsobj, "frame", 0, NULL, &ret);
  return(1);
}

JS(js_trigger_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  TriggerCtrl *trigger = new TriggerCtrl();

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)trigger) ) {
    error("failed assigning trigger controller to javascript");
    delete trigger; return JS_FALSE;
  }

  // initialize with javascript context
  if(! trigger->init(cx, obj) ) {
    error("failed initializing trigger controller");
    delete trigger; return JS_FALSE;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}
