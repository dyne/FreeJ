/*  FreeJ
 *  (c) Copyright 2006 Denis Roio aka jaromil <jaromil@dyne.org>
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

 * Virtual controller class to be inherited by other controllers

 */

#include <controller.h>

#include <kbd_ctrl.h>

#include <callbacks_js.h>
#include <jsparser_data.h>

Controller::Controller() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);
  initialized = active = false;
}

Controller::~Controller() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);
}


char *Controller::get_name() {
  return name;
}

// other functions are pure virtual
JS(js_ctrl_constructor);
DECLARE_CLASS_GC("Controller", js_ctrl_class, NULL, js_ctrl_gc);

JSFunctionSpec js_ctrl_methods[] = { 
  {"activate", controller_activate, 1},
  {0} 
};

JS(controller_activate) {

  JS_CHECK_ARGC(1);

  bool var = (bool)JSVAL_TO_BOOLEAN(argv[0]);

  Controller *ctrl = (Controller *) JS_GetPrivate(cx, obj);
  if(!ctrl) {
    error("%u:%s:%s :: Controller core data is NULL",	\
	  __LINE__,__FILE__,__FUNCTION__);		\
    return JS_FALSE;					\
  }
  
  ctrl->active = var;
  if(var) act("controller %s activated", ctrl->name);
  else    act("controller %s deactivated", ctrl->name);

  return JS_TRUE;
}

void js_ctrl_gc (JSContext *cx, JSObject *obj) {
	func("%s",__PRETTY_FUNCTION__);
	Controller* ctrl;
	if (!obj) {
		error("%n called with NULL object", __PRETTY_FUNCTION__);
		return;
	}
	// This callback is declared a Controller Class only,
	// we can skip the typecheck of obj, can't we?
	ctrl = (Controller *) JS_GetPrivate(cx, obj);
	JSClass *jc = JS_GET_CLASS(cx,obj);

	if (ctrl) {
		func("JSvalcmp(%s): %p / %p ctrl: %p", jc->name, OBJECT_TO_JSVAL(obj), ctrl->data, ctrl);
		notice("JSgc: removing %s Controller %s", jc->name, ctrl->name);
		ctrl->rem();
		ctrl->data = NULL;
		delete ctrl;
	} else {
		func("Mh, object(%s) has no private data", jc->name);
	}
}
