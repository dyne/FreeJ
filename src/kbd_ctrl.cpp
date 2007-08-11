/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 */

#include <kbd_ctrl.h>

#include <config.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h> // javascript

#define SDL_REPEAT_DELAY	200
#define SDL_REPEAT_INTERVAL	20

/////// Javascript KeyboardController
JS(js_kbd_ctrl_constructor);

DECLARE_CLASS("KeyboardController",js_kbd_ctrl_class, js_kbd_ctrl_constructor);

JSFunctionSpec js_kbd_ctrl_methods[] = { {0} }; // all dynamic methods


KbdCtrl::KbdCtrl()
  :Controller() {
  set_name("Keyboard");
}

KbdCtrl::~KbdCtrl() {

}

bool KbdCtrl::init(JSContext *env, JSObject *obj) {

  /* enable key repeat */
  SDL_EnableKeyRepeat(SDL_REPEAT_DELAY, SDL_REPEAT_INTERVAL);

  jsenv = env;
  jsobj = obj;
  
  initialized = true;
  return(true);
}

int KbdCtrl::peep(Context *env) {

  int res = 1;
  int c = 0;

  while(res > 0) {

    res = SDL_PeepEvents(&env->event, 1, SDL_PEEKEVENT, SDL_KEYEVENTMASK);

    if(res<=0) break;

    SDL_PeepEvents(&env->event, 1, SDL_GETEVENT, SDL_KEYEVENTMASK);
    poll(env);
    
    c++;
    
  }

  return c;
}

int KbdCtrl::poll(Context *env) {
  char tmp[8];
  char funcname[512];

  jsval ret;

  //  if(env->event.type != SDL_KEYDOWN) return 0;
  //  if(env->event.type != SDL_KEYUP)   return 0;  

  if(env->event.key.state == SDL_PRESSED)
    strcpy(funcname,"pressed_");
  else if(env->event.key.state == SDL_RELEASED)
    strcpy(funcname,"released_");
  else // no key state change
    return 0;
  
  keysym = & env->event.key.keysym;
  

  // check key modifiers
  if(keysym->mod & KMOD_CTRL)
    strcat(funcname,"ctrl_");

  if(keysym->mod & KMOD_ALT)
    strcat(funcname,"alt_");

  // check normal alphabet and letters
  if( keysym->sym >= SDLK_0
      && keysym->sym <= SDLK_9) {
    tmp[0] = keysym->sym;
    tmp[1] = 0x0;
    strcat(funcname,tmp);
    func("keyboard controller calling method %s()",funcname);
    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);

    return 1;
  }
  
  if( keysym->sym >= SDLK_a
      && keysym->sym <= SDLK_z) {
    
    // shift modifier only for letters
    if(keysym->mod & KMOD_SHIFT)
      strcat(funcname,"shift_");
    
    tmp[0] = keysym->sym;
    tmp[1] = 0x0;
    strcat(funcname,tmp);
    func("keyboard controller calling method %s()",funcname);
    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);

    return 1;
  }

  // check arrows
  CHECKSYM(SDLK_UP,        "up");
  CHECKSYM(SDLK_DOWN,      "down");
  CHECKSYM(SDLK_RIGHT,     "right");
  CHECKSYM(SDLK_LEFT,      "left");
  CHECKSYM(SDLK_INSERT,    "insert");
  CHECKSYM(SDLK_HOME,      "home");
  CHECKSYM(SDLK_END,       "end");
  CHECKSYM(SDLK_PAGEUP,    "pageup");
  CHECKSYM(SDLK_PAGEDOWN,  "pagedown");


  // check special keys
  CHECKSYM(SDLK_BACKSPACE, "backspace");
  CHECKSYM(SDLK_TAB,       "tab");
  CHECKSYM(SDLK_RETURN,    "return");
  CHECKSYM(SDLK_SPACE,     "space");
  CHECKSYM(SDLK_PLUS,      "plus");
  CHECKSYM(SDLK_MINUS,     "minus");
  CHECKSYM(SDLK_ESCAPE,    "esc");
  CHECKSYM(SDLK_LESS,      "less");
  CHECKSYM(SDLK_GREATER,   "greater");
  CHECKSYM(SDLK_EQUALS,    "equal");
  

  // check numeric keypad
  if(keysym->sym >= SDLK_KP0
     && keysym->sym <= SDLK_KP9) {
    tmp[0] = keysym->sym - SDLK_KP0 + 48;
    tmp[1] = 0x0;
    strcat(funcname,"num_");
    strcat(funcname,tmp);
    func("keyboard controller calling method %s()",funcname);
    JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
    return 1;
  }
  CHECKSYM(SDLK_KP_PERIOD,   "num_period");
  CHECKSYM(SDLK_KP_DIVIDE,   "num_divide");
  CHECKSYM(SDLK_KP_MULTIPLY, "num_multiply");
  CHECKSYM(SDLK_KP_MINUS,    "num_minus");
  CHECKSYM(SDLK_KP_PLUS,     "num_plus");
  CHECKSYM(SDLK_KP_ENTER,    "num_enter");
  CHECKSYM(SDLK_KP_EQUALS,   "num_equals");
  //////

  return 0;
}


JS(js_kbd_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  KbdCtrl *kbd = new KbdCtrl();

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)kbd) ) {
    error("failed assigning keyboard controller to javascript");
    delete kbd; return JS_FALSE;
  }

  // initialize with javascript context
  if(! kbd->init(cx, obj) ) {
    error("failed initializing keyboard controller");
    delete kbd; return JS_FALSE;
  }

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}
    
