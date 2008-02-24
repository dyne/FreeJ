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
	jsenv = NULL;
	jsobj = NULL;
}

Controller::~Controller() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	rem();
	if (jsobj)
		JS_SetPrivate(jsenv, jsobj, NULL);
	jsobj = NULL;
}

char *Controller::get_name() {
  return name;
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
	  ctrl->activate(var);
  }
  return JS_TRUE;
}

bool Controller::activate(bool state) {
	bool old = active;
	active = state;
	return old;
}

void Controller::poll_sdlevents(Uint32 eventmask) {
	int res;
	SDL_Event user_event;

	res = SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, eventmask);
	if (!res) return;

	user_event.type=SDL_USEREVENT;
	user_event.user.code=42;
	SDL_PeepEvents(&user_event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);

	res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, eventmask|SDL_EVENTMASK(SDL_USEREVENT));
	while (res>0) {
		int handled = dispatch(); // <<< virtual
		if (handled == 0)
				SDL_PeepEvents(&event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);
		res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, eventmask|SDL_EVENTMASK(SDL_USEREVENT));
		if (event.type == SDL_USEREVENT)
				res = 0;
	}
	//return 1;
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
		func("JSvalcmp(%s): %p / %p ctrl: %p", jc->name, OBJECT_TO_JSVAL(obj), ctrl->jsobj, ctrl);
		notice("JSgc: deleting %s Controller %s", jc->name, ctrl->name);
		delete ctrl;
	} else {
		func("Mh, object(%s) has no private data", jc->name);
	}
}
