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


#include <string.h>

#include <mouse_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>


#include <callbacks_js.h> // javascript
#include <jsparser_data.h>

JS(js_mouse_ctrl_constructor);

//DECLARE_CLASS_GC("MouseController",js_mouse_ctrl_class, js_mouse_ctrl_constructor,js_ctrl_gc);

JSBool js_add_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("add prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
}
JSBool js_del_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("del prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
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
	if (state) {
		SDL_ShowCursor(0);
		SDL_WM_GrabInput(SDL_GRAB_ON);
	} else {
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
	}
	return JS_TRUE;
}

MouseCtrl::MouseCtrl()
	:Controller() {
	set_name("Mouse");
}

MouseCtrl::~MouseCtrl() {
	SDL_ShowCursor(1);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
}

bool MouseCtrl::init(JSContext *env, JSObject *obj) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	jsenv = env;
	jsobj = obj;

	initialized = true;
	return(true);
}

int MouseCtrl::peep(Context *env) {
	int res;
	SDL_Event user_event;
	res = SDL_PeepEvents(&env->event, 1, SDL_PEEKEVENT, SDL_MOUSEEVENTMASK);
	if (!res) return 1;

	user_event.type=SDL_USEREVENT;
	user_event.user.code=42;
	SDL_PeepEvents(&user_event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);

	res = SDL_PeepEvents(&env->event, 1, SDL_GETEVENT, SDL_MOUSEEVENTMASK|SDL_EVENTMASK(SDL_USEREVENT));
	while (res>0) {
		int handled = poll(env);
		if (handled == 0)
				SDL_PeepEvents(&env->event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);
		res = SDL_PeepEvents(&env->event, 1, SDL_GETEVENT, SDL_MOUSEEVENTMASK|SDL_EVENTMASK(SDL_USEREVENT));
		if (env->event.type == SDL_USEREVENT)
				res = 0;
	}
	return 1;
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

int MouseCtrl::poll(Context *env) {
	jsval fval = JSVAL_VOID;
	jsval ret = JSVAL_VOID;
	JSObject *objp;
	jsval res = JSVAL_TRUE;

	if (env->event.type == SDL_MOUSEMOTION) {
		JS_GetMethod(jsenv, jsobj, "motion", &objp, &fval);
		if(!JSVAL_IS_VOID(fval)) {
			SDL_MouseMotionEvent *mm = &env->event.motion;
			// MouseController.motion(buttonstate, x, y, xrel, yrel)
			jsval js_data[] = {
				INT_TO_JSVAL(mm->state),
				INT_TO_JSVAL(mm->x),
				INT_TO_JSVAL(mm->y),
				INT_TO_JSVAL(mm->xrel),
				INT_TO_JSVAL(mm->yrel),
			};
			res = JS_CallFunctionValue(jsenv, jsobj, fval, 5, js_data, &ret);
		}
	} else { // MOUSE_BUTTON
		JS_GetMethod(jsenv, jsobj, "button", &objp, &fval);
		if(!JSVAL_IS_VOID(fval)) {
			SDL_MouseButtonEvent *mb = &env->event.button;
			// MouseController.button(button, state, x, y)
			jsval js_data[] = {
				INT_TO_JSVAL(mb->button),
				INT_TO_JSVAL(mb->state),
				INT_TO_JSVAL(mb->x),
				INT_TO_JSVAL(mb->y),
			};
			res = JS_CallFunctionValue(jsenv, jsobj, fval, 4, js_data, &ret);
		}
	}
	if (JSVAL_IS_NULL(res)) {
		error("MouseController call failed, deactivate ctrl");
		active = false;
		// just in case ;)
		SDL_ShowCursor(1);
		SDL_WM_GrabInput(SDL_GRAB_OFF);
		return 0;
	}
	if (res) {
		if(!JSVAL_IS_VOID(ret)) {
			JSBool ok;
			JS_ValueToBoolean(jsenv, ret, &ok);
			if (!ok) // JSfunc returned 'false', so redo the event
				return 0;
		}
	}
	return 1;
}

JS(js_mouse_ctrl_constructor) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

	MouseCtrl *mouse = new MouseCtrl();

	// assign instance into javascript object
	if( ! JS_SetPrivate(cx, obj, (void*)mouse) ) {
		error("failed assigning mouse controller to javascript");
		delete mouse; return JS_FALSE;
	}

	// initialize with javascript context
	if(! mouse->init(cx, obj) ) {
		error("failed initializing mouse controller");
		delete mouse; return JS_FALSE;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	mouse->data = (void*)*rval;
	return JS_TRUE;
}
