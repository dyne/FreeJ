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
 * "$Id$"
 *
 */

#include <string.h>

#include <joy_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>


#include <callbacks_js.h> // javascript

/////// Javascript JoystickController
JS(js_joy_ctrl_constructor);

DECLARE_CLASS("JoystickController",js_joy_ctrl_class, js_joy_ctrl_constructor);

JSFunctionSpec js_joy_ctrl_methods[] = { {0} }; // all dynamic methods



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

int JoyCtrl::poll(Context *env) {
  int c;
  char funcname[512];
  jsval ret;
  /*
  if(! (env->event.type
	& (SDL_JOYAXISMOTION
	   |SDL_JOYBALLMOTION
	   |SDL_JOYHATMOTION
	   |SDL_JOYBUTTONDOWN
	   |SDL_JOYBUTTONUP   )))
    return 0;
  */
  switch(env->event.type) {
    
  case SDL_JOYAXISMOTION:

    snprintf(funcname,512,"axis_%u(%i)", env->event.jaxis.axis);

    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);

    break;

  case SDL_JOYBALLMOTION:


    snprintf(funcname,512,"ball_%u", env->event.jball.ball);

    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
    
    break;

  case SDL_JOYHATMOTION:

    snprintf(funcname,512,"hat_%u", env->event.jhat.hat);

    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
    
    break;

  case SDL_JOYBUTTONDOWN:

    snprintf(funcname,512,"button_%u_down", env->event.jbutton.button);

    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
    
    break;

  case SDL_JOYBUTTONUP:

    snprintf(funcname,512,"button_%u_up", env->event.jbutton.button);

    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);

    break;

  default: return 0;
    
  }

  {
    int j,c;
    for(j=0;j<num;j++) {
      func("action on %s",SDL_JoystickName(j));
      /* print out axes */
      for(c=0;c<axes;c++)
	func("axis %i position %i",c,SDL_JoystickGetAxis(joy[j],c));
      for(c=0;c<buttons;c++)
	func("button %i position %i",c,SDL_JoystickGetButton(joy[j],c));
      

    }
  }

  func("joystick controller called method %s()",funcname);
    
  return(1);
    
}

JS(js_joy_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  JoyCtrl *joy = new JoyCtrl();


  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)joy) ) {
    error("failed assigning keyboard controller to javascript");
    delete joy; return JS_FALSE;
  }

  // initialize with javascript context
  if(! joy->init(cx, obj) ) {
    error("failed initializing keyboard controller");
    delete joy; return JS_FALSE;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}
