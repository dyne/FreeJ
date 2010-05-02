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
  func("%s %s (%p)",__PRETTY_FUNCTION__, name, this);
  ControllerListener *listener = listeners.begin();
  while (listener) {
    delete listener;
    listener = listeners.begin();
  }
}

bool Controller::init(Context *freej) {
  func("%s",__PRETTY_FUNCTION__);
  env = freej;

  if(freej->js) {
      // the object is set to global, but should be overwritten
      // in every specific object constructor with the "obj" from JS
      // XXX - set initial value to NULL instead of creating a fake useless object
      jsenv = freej->js->global_context;
      jsobj = freej->js->global_object;
  }
  
  initialized = true;
  return(true);
}

// other functions are pure virtual
JS(js_ctrl_constructor);
DECLARE_CLASS("Controller", js_ctrl_class, NULL);

JSFunctionSpec js_ctrl_methods[] = { 
  {"activate", controller_activate, 0},
  {0} 
};

JS(controller_activate) {
  Controller *ctrl = (Controller *) JS_GetPrivate(cx, obj);
  if(!ctrl) {
    error("%u:%s:%s :: Controller core data is NULL",    \
      __LINE__,__FILE__,__FUNCTION__);        \
    return JS_FALSE;                    \
  }
  
  *rval = BOOLEAN_TO_JSVAL(ctrl->active);
  if (argc == 1) {
    jsint var = js_get_int(argv[0]);
    ctrl->active = var;
  }
  return JS_TRUE;
}

bool Controller::add_listener(JSContext *cx, JSObject *obj)
{
    ControllerListener *listener = new ControllerListener(cx, obj);
    listeners.append(listener);
    return true;
}

void Controller::reset()
{
    active = false;
    ControllerListener *listener = listeners.begin();
    while (listener) {
        delete listener;
        listener = listeners.begin();
    }
}

int Controller::JSCall(const char *funcname, int argc, jsval *argv)
{
    int res = 0;
    ControllerListener *listener = listeners.begin();
    while (listener) {
        // TODO - unregister listener if returns false
        if (listener->call(funcname, argc, argv))
            res++;
        listener = (ControllerListener *)listener->next;
    }
    return res;
}

int Controller::JSCall(const char *funcname, int argc, const char *format, ...)
{
    int res;
    jsval *argv;
    va_list args;
    ControllerListener *listener = listeners.begin();
    va_start(args, format);
    va_end(args);
    while (listener) {
        void *markp = NULL;
        //JS_SetContextThread(listener->context());
        //JS_BeginRequest(listener->context());
        argv = JS_PushArgumentsVA(listener->context(), &markp, format, args);
        //JS_EndRequest(listener->context());
        //JS_ClearContextThread(listener->context());
        // TODO - unregister listener if returns false
        if (listener->call(funcname, argc, argv))
            res++;
        listener = (ControllerListener *)listener->next;
    }
    return res;
}

// ControllerListener

ControllerListener::ControllerListener(JSContext *cx, JSObject *obj)
{
    jsContext = cx;
    jsObject = obj;
    frameFunc = NULL;
}

ControllerListener::~ControllerListener()
{
    
}

bool ControllerListener::frame()
{
    jsval ret = JSVAL_VOID;
    JSBool res;
    
    JS_SetContextThread(jsContext);
    //JS_BeginRequest(jsContext);
    
    if (!frameFunc) {
        res = JS_GetProperty(jsContext, jsObject, "frame", &frameFunc);
        if(!res || JSVAL_IS_VOID(frameFunc)) {
            error("method frame not found in TriggerController");
            JS_ClearContextThread(jsContext);
            //JS_EndRequest(jsContext);
            return false;
        }
    }
    res = JS_CallFunctionValue(jsContext, jsObject, frameFunc, 0, NULL, &ret);
    //JS_EndRequest(jsContext);
    JS_ClearContextThread(jsContext);
    if (res == JS_FALSE) {
        error("trigger call frame() failed, deactivate ctrl");
        //active = false;
        return false;
    }
    return true;
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
bool ControllerListener::call(const char *funcname, int argc, const char *format, ...) {
    va_list ap;
    jsval fval = JSVAL_VOID;
    jsval ret = JSVAL_VOID;
    
    func("%s try calling method %s.%s(argc:%i)", __func__, name, funcname, argc);
    JS_SetContextThread(jsContext);
    //JS_BeginRequest(jsContext);
    int res = JS_GetProperty(jsContext, jsObject, funcname, &fval);
    
    if(JSVAL_IS_VOID(fval)) {
        warning("method unresolved by JS_GetProperty");
    } else {
        jsval *argv;
        void *markp;
        
        va_start(ap, format);
        argv = JS_PushArgumentsVA(jsContext, &markp, format, ap);
        va_end(ap);
        
        res = JS_CallFunctionValue(jsContext, jsObject, fval, argc, argv, &ret);
        JS_PopArguments(jsContext, &markp);
        
        if (res) {
            if(!JSVAL_IS_VOID(ret)) {
                JSBool ok;
                JS_ValueToBoolean(jsContext, ret, &ok);
                if (ok) // JSfunc returned 'true', so event is done
                {
                    //JS_EndRequest(jsContext);
                    JS_ClearContextThread(jsContext);
                    return true;
                }
            }
        }
    }
    //JS_EndRequest(jsContext);
    JS_ClearContextThread(jsContext);
    return false; // no callback, redo on next controller
}

/* less bloat but this only works with 4 byte argv values
 */
bool ControllerListener::call(const char *funcname, int argc, jsval *argv) {
    jsval fval = JSVAL_VOID;
    jsval ret = JSVAL_VOID;
    JSBool res;
    
    func("calling js %s.%s()", name, funcname);
    JS_SetContextThread(jsContext);
    //JS_BeginRequest(jsContext);
    res = JS_GetProperty(jsContext, jsObject, funcname, &fval);
    if(!res || JSVAL_IS_VOID(fval)) {
        // using func() instead of error() because this is not a real error condition.
        // controller could ask for unregistered functions ...
        // for instance in the case of a keyboardcontroller which propagates keystrokes 
        // for unregistered keys 
        func("method %s not found in %s controller", funcname, name);
        //JS_EndRequest(jsContext);
        JS_ClearContextThread(jsContext);
        return(false);
    }
    
    res = JS_CallFunctionValue(jsContext, jsObject, fval, argc, argv, &ret);
    //JS_EndRequest(jsContext);
    JS_ClearContextThread(jsContext);

    if(res == JS_FALSE) {
        error("%s : failed call", __PRETTY_FUNCTION__);
        return(false);
    }
    
    return(true);
}

JSContext *ControllerListener::context()
{
    return jsContext;
}

JSObject *ControllerListener::object()
{
    return jsObject;
}
