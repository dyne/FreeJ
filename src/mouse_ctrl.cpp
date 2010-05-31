/*  FreeJ - Mouse controller 
 *
 *  (c) Copyright 2008 Christoph Rudorff <goil@dyne.org>
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

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Controller, MouseController, MouseController, sdl);

JS(js_mouse_ctrl_constructor);

DECLARE_CLASS("MouseController",js_mouse_ctrl_class, js_mouse_ctrl_constructor);

JSBool js_add_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("add prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
	return JS_TRUE;
}
JSBool js_del_p (JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
	func("del prop: %s %s", JS_GetStringBytes(JS_ValueToString(cx, id)), JS_GetStringBytes(JS_ValueToString(cx, *vp)));
	return JS_TRUE;
}

// JSClass js_mouse_ctrl_class = {
// 	"MouseController", JSCLASS_HAS_PRIVATE,
// 	//	js_add_p,  js_del_p, // add, del
// 	JS_PropertyStub,  JS_PropertyStub,

// 	JS_PropertyStub,  JS_PropertyStub,
// 	JS_EnumerateStub, JS_ResolveStub,
// 	JS_ConvertStub,   js_ctrl_gc,
// 	NULL,   NULL,
// 	js_mouse_ctrl_constructor
// };
//static JSClass *jsclass_s = &js_mouse_ctrl_class;

JSFunctionSpec js_mouse_ctrl_methods[] = {
	{"grab",	js_mouse_grab,	1},
	{0}
};

JS(js_mouse_grab) {
	JS_CHECK_ARGC(1);

	jsint state = js_get_int(argv[0]);

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
    :SdlController()
{
    set_name("Mouse");
    indestructible = true;
}

MouseController::~MouseController() {
  active = false; // ungrab ... ;)
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
    jsval ret = JSVAL_VOID;

    jsval js_data[] = {
        INT_TO_JSVAL(state),
        INT_TO_JSVAL(x), INT_TO_JSVAL(y),
        INT_TO_JSVAL(xrel), INT_TO_JSVAL(yrel)
    };
    if (!JSCall("motion", 5, js_data)) {
        error("Can't call method motion() on mouse listeners");
        return(0);
    }
    return(1);
}

int MouseController::button(int button, int state, int x, int y) {
    jsval ret = JSVAL_VOID;

    jsval js_data[] = {
        INT_TO_JSVAL(button),
        INT_TO_JSVAL(state),
        INT_TO_JSVAL(x),
        INT_TO_JSVAL(y)
    };
    if (!JSCall("button", 4, js_data)) {
        error("Can't call method button() on mouse listeners");
        return(0);
    }
    return(1);
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

    MouseController *mouse = (MouseController *)Factory<Controller>::get_instance( "MouseController" );

    // initialize with javascript context
    if(! mouse->init(global_environment) ) {
            error("failed initializing mouse controller");
            delete mouse; return JS_FALSE;
    }

    // assign instance into javascript object
    if( ! JS_SetPrivate(cx, obj, (void*)mouse) ) {
            error("failed assigning mouse controller to javascript");
            delete mouse; return JS_FALSE;
    }

    mouse->add_listener(cx, obj);
    *rval = OBJECT_TO_JSVAL(obj);
    return JS_TRUE;
}
