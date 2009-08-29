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
#include <linklist.h>
#include <jsparser.h>
#include <jsparser_data.h>
#include <callbacks_js.h>

Controller::Controller() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	initialized = active = false;
	indestructible = false;
	javascript = false;
	jsenv = NULL;
	jsobj = NULL;
}

Controller::~Controller() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	rem();
	//	if (jsobj)
	  //		JS_SetPrivate(jsenv, jsobj, NULL);
	//	jsobj = NULL;
}

bool Controller::init(Context *freej) {
  func("%s",__PRETTY_FUNCTION__);
  env = freej;

  if(freej->js) {
    jsenv = freej->js->global_context;
    
    // the object is set to global, but should be overwritten
    // in every specific object constructor with the "obj" from JS
    jsobj = freej->js->global_object;
  }
  
  initialized = true;
  return(true);
}

// other functions are pure virtual
JS(js_ctrl_constructor);
DECLARE_CLASS_GC("Controller", js_ctrl_class, NULL, js_ctrl_gc);

JSFunctionSpec js_ctrl_methods[] = { 
  {"activate", controller_activate, 0},
  {0} 
};

JS(controller_activate) {
  Controller *ctrl = (Controller *) JS_GetPrivate(cx, obj);
  if(!ctrl) {
    error("%u:%s:%s :: Controller core data is NULL",	\
	  __LINE__,__FILE__,__FUNCTION__);		\
    return JS_FALSE;					\
  }
  
  *rval = BOOLEAN_TO_JSVAL(ctrl->active);
  if (argc == 1) {
	  JS_ARG_NUMBER(var,0);
	  ctrl->active = var;
  }
  return JS_TRUE;
}

// Garbage collector callback for Controller classes
void js_ctrl_gc (JSContext *cx, JSObject *obj) {
	func("%s",__PRETTY_FUNCTION__);
	Controller* ctrl;
	JSClass *jc;
	if (!obj) {
		error("%n called with NULL object", __PRETTY_FUNCTION__);
		return;
	}
	warning("controller garbage collector called (TODO)");
	// This callback is declared a Controller Class only,
	// we can skip the typecheck of obj, can't we?
// 	ctrl = (Controller *) JS_GetPrivate(cx, obj);

// 	if (ctrl) {
// 	  //	  jc = JS_GET_CLASS(cx,obj);
// 	  func("JSvalcmp(%s): %p / %p ctrl: %p", jc->name, OBJECT_TO_JSVAL(obj), ctrl->jsobj, ctrl);
// 	  notice("JSgc: deleting %s Controller %s", jc->name, ctrl->name);
// 	  delete ctrl;
// 	} else {
// 	  func("Mh, object(%s) has no private data", jc->name);
// 	}
	
}

/* JSCall function by name, cvalues will be converted
 *
 * deactivates controller if any script errors!
 *
 * format values:
 * case 'b': BOOLEAN_TO_JSVAL((JSBool) va_arg(ap, int));
 * case 'c': INT_TO_JSVAL((uint16) va_arg(ap, unsigned int));
 * case 'i':
 * case 'j': js_NewNumberValue(cx, (jsdouble) va_arg(ap, int32), sp)
 * case 'u': js_NewNumberValue(cx, (jsdouble) va_arg(ap, uint32), sp)
 * case 'd':
 * case 'I': js_NewDoubleValue(cx, va_arg(ap, jsdouble), sp)
 * case 's': JS_NewStringCopyZ(cx, va_arg(ap, char *))
 * case 'W': JS_NewUCStringCopyZ(cx, va_arg(ap, jschar *))
 * case 'S': va_arg(ap, JSString *)
 * case 'o': OBJECT_TO_JSVAL(va_arg(ap, JSObject *)
 * case 'f':
 * fun = va_arg(ap, JSFunction *);
 *       fun ? OBJECT_TO_JSVAL(fun->object) : JSVAL_NULL;
 * case 'v': va_arg(ap, jsval);
 */
int Controller::JSCall(const char *funcname, int argc, const char *format, ...) {
	va_list ap;
	jsval fval = JSVAL_VOID;
	jsval ret = JSVAL_VOID;

	func("%s try calling method %s.%s(argc:%i)", __func__, name, funcname, argc);
	int res = JS_GetProperty(jsenv, jsobj, funcname, &fval);

	if(JSVAL_IS_VOID(fval)) {
	  warning("method unresolved by JS_GetProperty");
	} else {
		jsval *argv;
		void *markp;

		va_start(ap, format);
		argv = JS_PushArgumentsVA(jsenv, &markp, format, ap);
		va_end(ap);

		res = JS_CallFunctionValue(jsenv, jsobj, fval, argc, argv, &ret);
		JS_PopArguments(jsenv, &markp);

		if (res) {
			if(!JSVAL_IS_VOID(ret)) {
				JSBool ok;
				JS_ValueToBoolean(jsenv, ret, &ok);
				if (ok) // JSfunc returned 'true', so event is done
					return 1;
			}
		} else { // script error
			error("%s.%s() call failed, deactivating ctrl", name, funcname);
			active = false;
		}
		return 0; // requeue event for next controller
	}
	return 0; // no callback, redo on next controller
}

/* less bloat but this only works with 4 byte argv values
 */
int Controller::JSCall(const char *funcname, int argc, jsval *argv) {
  JSObject *objp = NULL;
  jsval fval = JSVAL_VOID;
  jsval ret = JSVAL_VOID;
  JSBool res;

  func("calling js %s.%s()", name, funcname);
  res = JS_GetMethod(jsenv, jsobj, funcname, &objp, &fval);
  if(!res || JSVAL_IS_VOID(fval)) {
    // using func() instead of error() because this is not a real error condition.
    // controller could ask for unregistered functions ...
    // for instance in the case of a keyboardcontroller which propagates keystrokes 
    // for unregistered keys 
    func("method %s not found in %s controller", funcname, name);
    return(0);
  }

  res = JS_CallFunctionValue(jsenv, jsobj, fval, argc, argv, &ret);
  if(res == JS_FALSE) {
    error("%s : failed call", __PRETTY_FUNCTION__);
    return(0);
  }
  
  return(1);
}
