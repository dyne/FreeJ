/*  FreeJ
 *  (c) Copyright 2006-2007 Denis Roio aka jaromil <jaromil@dyne.org>
 *   fixed at CCC camp 2007 by MrGoil
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
 * "$Id$"
 *
 */

#include <string.h>

#include <joy_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>


#include <callbacks_js.h> // javascript
#include <jsparser_data.h>

/////// Javascript JoystickController
JS(js_joy_ctrl_constructor);

DECLARE_CLASS_GC("JoystickController",js_joy_ctrl_class, js_joy_ctrl_constructor,js_ctrl_gc);

JSFunctionSpec js_joy_ctrl_methods[] = {
  {0}
};



JoyCtrl::JoyCtrl()
  :Controller() {
    
    set_name("Joystick");
    
    num = 0;
}

JoyCtrl::~JoyCtrl() {
  int c;

  for(c=0;c<num;c++)
    SDL_JoystickClose(joy[c]);

}

bool JoyCtrl::init(JSContext *env, JSObject *obj) {
  //bool JoyCtrl::init(Context *context) {
  func("JoyCtrl::init()");
 
  int found = 0;
  int c;

  num = SDL_NumJoysticks();
  if(num>4) num = 4; // we support maximum 4 joysticks

  func("num joysticks %i",num);
  for(c=0;c<num;c++) {
    joy[found] = SDL_JoystickOpen(c);
    if(joy[found]) {
      if(strstr(SDL_JoystickName(c),"Keyboard")) {
	/* this is not a joystick! it happens on MacOSX
	   to have "Apple Extended USB Keyboard" recognized as joystick */
	SDL_JoystickClose(joy[found]);
	continue;
      }
      notice("Joystick: %s",SDL_JoystickName(c));
      axes = SDL_JoystickNumAxes(joy[found]);
      buttons = SDL_JoystickNumButtons(joy[found]);
      balls = SDL_JoystickNumBalls(joy[found]);
      hats = SDL_JoystickNumHats(joy[found]);
      act("%i axes, %i balls, %i hats, %i buttons",
	  axes, balls, hats, buttons);
      found++;
    } else {
      error("error opening %s",SDL_JoystickName(c));
    }
  }
  
  num = found;
  
  if(!num) {
    notice("no joystick found");
    return(false);
  } else
    SDL_JoystickEventState(SDL_ENABLE);
  
  jsenv = env;
  jsobj = obj;
  
  initialized = true;

  return(true);
}

int JoyCtrl::poll() {
	poll_sdlevents(SDL_JOYEVENTMASK); // calls dispatch() 
	return 0;
}

int JoyCtrl::dispatch() {

	switch(event.type) {
		
	case SDL_JOYAXISMOTION:
		return JSCall("axismotion", 3, "uui", 
			event.jaxis.which, event.jaxis.axis, event.jaxis.value
		);
		break;

	case SDL_JOYBALLMOTION:
		return JSCall("ballmotion", 4, "uuii", 
			event.jball.which, event.jball.ball, 
			event.jball.xrel, event.jball.yrel
		);
		break;

	case SDL_JOYHATMOTION:
		return JSCall("hatmotion", 3, "uui", 
			event.jhat.which, event.jhat.hat, event.jhat.value
		);
		break;
		
	case SDL_JOYBUTTONDOWN:
		return JSCall("button", 3, "uuc", 
			event.jbutton.which, event.jbutton.button, 1
		);
		break;
		
	case SDL_JOYBUTTONUP:
		return JSCall("button", 3, "uuc", 
			event.jbutton.which, event.jbutton.button, 0
		);
		break;

	default: 
		return 0;
	}
}

JS(js_joy_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];

  JoyCtrl *joy = new JoyCtrl();

  // initialize with javascript context
  if(! joy->init(cx, obj) ) {
    sprintf(excp_msg, "failed initializing joystick controller");
    goto error;
  }

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)joy) ) {
    sprintf(excp_msg, "failed assigning joystick controller to javascript");
    goto error;
  }
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

error:
    JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
              JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
    //    cx->newborn[GCX_OBJECT] = NULL;
    // same omissis as in callbacks_js.h
    delete joy; return JS_FALSE;
}


