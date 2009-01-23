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
 * "$Id: mouse_ctrl.cpp 881 2008-02-22 01:06:11Z mrgoil $"
 *
 */

#include <config.h>
#include <string.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h> // javascript
#include <jsparser_data.h>
#include <mouse_ctrl.h>

JS(js_mouse_ctrl_constructor);

//DECLARE_CLASS_GC("MouseController",js_mouse_ctrl_class, js_mouse_ctrl_constructor,js_ctrl_gc);

JSBool js_add_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("add prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
	return JS_TRUE;
}
JSBool js_del_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("del prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
	return JS_TRUE;
}

JSClass js_mouse_ctrl_class = {
	"MouseController", JSCLASS_HAS_PRIVATE,
	js_add_p,  js_del_p, // add, del
	JS_PropertyStub,  JS_PropertyStub,
	JS_EnumerateStub, JS_ResolveStub,
	JS_ConvertStub,   js_ctrl_gc,
	NULL,   NULL,
	js_mouse_ctrl_constructor
};
static JSClass *jsclass_s = &js_mouse_ctrl_class;

JSFunctionSpec js_mouse_ctrl_methods[] = {
	{"grab",	js_mouse_grab,	1},
	{0}
};

JS(js_mouse_grab) {
	JS_CHECK_ARGC(1);
	JS_ARG_NUMBER(state,0);

	MouseController *mouse = (MouseController *) JS_GetPrivate(cx,obj);
	if(!mouse) {
		error("%u:%s:%s :: Mouse core data is NULL",
		__LINE__,__FILE__,__FUNCTION__);
		return JS_FALSE;
	}

	mouse->grab(state);

	return JS_TRUE;
}

MouseController::MouseController()
	:Controller() {
	set_name("Mouse");
}

MouseController::~MouseController() {
  active = false; // ungrab ... ;)
}

bool MouseController::init(JSContext *env, JSObject *obj) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	jsenv = env;
	jsobj = obj;

	initialized = true;
	return(true);
}

// activate is removed from controller
// bool active is operated directly
// this solves a problem with swig and virtual inheritance...
// hopefully removing the flush data here won't hurt!

// bool MouseController::activate(bool state) {
// 	bool old = active;
// 	active = state;
// 	if (state == false) {
// 		SDL_ShowCursor(1);
// 		SDL_WM_GrabInput(SDL_GRAB_OFF);
// 	}
// 	return old;
// }

int MouseController::poll() {
	poll_sdlevents(SDL_MOUSEEVENTMASK); // calls dispatch() foreach SDL_Event
	return 0;
}

/*
typedef struct{
  Uint8 type;  SDL_MOUSEMOTION
  Uint8 state; SDL_PRESSED or SDL_RELEASED
  Uint16 x, y;
  Sint16 xrel, yrel;
} SDL_MouseMotionEvent;

typedef struct{
  Uint8 type;  SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
  Uint8 button; 1 - ...
  Uint8 state; SDL_PRESSED or SDL_RELEASED
  Uint16 x, y;
} SDL_MouseButtonEvent;
*/

int MouseController::motion(int state, int x, int y, int xrel, int yrel) {
	JSBool ret= JS_TRUE;
	jsval js_data[] = {state, x, y, xrel, yrel};
	return JSCall("motion", 5, js_data, &ret);
}

int MouseController::button(int button, int state, int x, int y) {
	JSBool ret= JS_TRUE;
	jsval js_data[] = {button, state, x, y};
	return JSCall("button", 4, js_data, &ret);
}

int MouseController::dispatch() {
	if (event.type == SDL_MOUSEMOTION) {
		SDL_MouseMotionEvent mm = event.motion;
		return motion(mm.state, mm.x, mm.y, mm.xrel, mm.yrel);
	} else { // MOUSE_BUTTON
		SDL_MouseButtonEvent mb = event.button;
		return button(mb.button, mb.state, mb.x, mb.y);
	}
}

void MouseController::grab(bool state) {
	if (state) {
		SDL_ShowCursor(0);
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
}

JS(js_mouse_ctrl_constructor) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

	MouseController *mouse = new MouseController();

	// initialize with javascript context
	if(! mouse->init(cx, obj) ) {
		error("failed initializing mouse controller");
		delete mouse; return JS_FALSE;
	}

	// assign instance into javascript object
	if( ! JS_SetPrivate(cx, obj, (void*)mouse) ) {
		error("failed assigning mouse controller to javascript");
		delete mouse; return JS_FALSE;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
