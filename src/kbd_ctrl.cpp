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
#include <jsparser_data.h>

#define SDL_REPEAT_DELAY	200
#define SDL_REPEAT_INTERVAL	20

#define SDL_KEYEVENTMASK (SDL_KEYDOWNMASK|SDL_KEYUPMASK)


/////// Javascript KeyboardController
JS(js_kbd_ctrl_constructor);

DECLARE_CLASS("KeyboardController",js_kbd_ctrl_class, js_kbd_ctrl_constructor);

JSFunctionSpec js_kbd_ctrl_methods[] = {
  CONTROLLER_METHODS,
  {0}
};


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
  int res;
  SDL_Event user_event;

  user_event.type=SDL_USEREVENT;
  user_event.user.code=42;
  SDL_PeepEvents(&user_event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);

  res = SDL_PeepEvents(&env->event, 1, SDL_GETEVENT, SDL_KEYEVENTMASK|SDL_EVENTMASK(SDL_USEREVENT));
  while (res>0) {
    int handled = poll(env);
    if (handled == 0)
        SDL_PeepEvents(&env->event, 1, SDL_ADDEVENT, SDL_ALLEVENTS);
    res = SDL_PeepEvents(&env->event, 1, SDL_GETEVENT, SDL_KEYEVENTMASK|SDL_EVENTMASK(SDL_USEREVENT));
    if (env->event.type == SDL_USEREVENT)
        res = 0;
  }
  return 1;
}

int KbdCtrl::checksym(SDLKey key, char *name) {
  jsval ret;
  if(keysym->sym == key) {
    strcat(keyname,name);
    func("keyboard controller detected key: %s",keyname);
    if(env->event.key.state == SDL_PRESSED)
      snprintf(funcname, 511, "pressed_%s", keyname);
    else // if(env->event.key.state == SDL_RELEASED)
      snprintf(funcname, 511, "released_%s", keyname);

    func("keyboard controller calling method %s()",funcname);
    return JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
/*
    {
      jsval js_data[] = {
        STRING_TO_JSVAL(keyname),
        DOUBLE_TO_JSVAL(key) };
      if(env->event.key.state == SDL_PRESSED)
        JS_CallFunctionName(jsenv, jsobj, "pressed", 2, js_data, &ret);
      else // if(env->event.key.state == SDL_RELEASED)
        JS_CallFunctionName(jsenv, jsobj, "released", 2, js_data, &ret);
    } */
  }
  return 0;
}


int KbdCtrl::poll(Context *env) {
  char tmp[8];

  jsval ret;

  if(env->event.key.state != SDL_PRESSED)
    if(env->event.key.state != SDL_RELEASED)
      return 0; // no key state change
  
  keysym = & env->event.key.keysym;
  
  memset(keyname, 0, sizeof(char)<<9);  // *512
  memset(funcname, 0, sizeof(char)<<9); // *512
  
  // check key modifiers
  if(keysym->mod & KMOD_SHIFT)
    strcat(keyname,"shift_");
  if(keysym->mod & KMOD_CTRL)
    strcat(keyname,"ctrl_");
  if(keysym->mod & KMOD_ALT)
    strcat(keyname,"alt_");
  
  // check normal alphabet and letters
  if( keysym->sym >= SDLK_0
      && keysym->sym <= SDLK_9) {
    tmp[0] = keysym->sym;
    tmp[1] = 0x0;
    strcat(keyname,tmp);
    if(env->event.key.state == SDL_PRESSED)
      sprintf(funcname,"pressed_%s",keyname);
    else //if(env->event.key.state != SDL_RELEASED)
      sprintf(funcname,"released_%s",keyname);
    func("keyboard controller calling method %s()",funcname);
    return JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
  }
  
  if( keysym->sym >= SDLK_a
      && keysym->sym <= SDLK_z) {
    
    tmp[0] = keysym->sym;
    tmp[1] = 0x0;
    strcat(keyname,tmp);
    if(env->event.key.state == SDL_PRESSED)
      sprintf(funcname,"pressed_%s",keyname);
    else //if(env->event.key.state != SDL_RELEASED)
      sprintf(funcname,"released_%s",keyname);
    func("keyboard controller calling method %s()",funcname);
    return JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
  }

  // check arrows
  checksym(SDLK_UP,        "up");
  checksym(SDLK_DOWN,      "down");
  checksym(SDLK_RIGHT,     "right");
  checksym(SDLK_LEFT,      "left");
  checksym(SDLK_INSERT,    "insert");
  checksym(SDLK_HOME,      "home");
  checksym(SDLK_END,       "end");
  checksym(SDLK_PAGEUP,    "pageup");
  checksym(SDLK_PAGEDOWN,  "pagedown");


  // check special keys
  checksym(SDLK_BACKSPACE, "backspace");
  checksym(SDLK_TAB,       "tab");
  checksym(SDLK_RETURN,    "return");
  checksym(SDLK_SPACE,     "space");
  checksym(SDLK_PLUS,      "plus");
  checksym(SDLK_MINUS,     "minus");
  checksym(SDLK_ESCAPE,    "esc");
  checksym(SDLK_LESS,      "less");
  checksym(SDLK_GREATER,   "greater");
  checksym(SDLK_EQUALS,    "equal");
  

  // check numeric keypad
  if(keysym->sym >= SDLK_KP0
     && keysym->sym <= SDLK_KP9) {
    tmp[0] = keysym->sym - SDLK_KP0 + 48;
    tmp[1] = 0x0;
    strcat(keyname,"num_");
    strcat(keyname,tmp);
    if(env->event.key.state == SDL_PRESSED)
      sprintf(funcname,"pressed_%s",keyname);
    else //if(env->event.key.state != SDL_RELEASED)
      sprintf(funcname,"released_%s",keyname);
    func("keyboard controller calling method %s()",funcname);
    return JS_CallFunctionName(jsenv, jsobj, funcname, 0, NULL, &ret);
  }
  checksym(SDLK_KP_PERIOD,   "num_period");
  checksym(SDLK_KP_DIVIDE,   "num_divide");
  checksym(SDLK_KP_MULTIPLY, "num_multiply");
  checksym(SDLK_KP_MINUS,    "num_minus");
  checksym(SDLK_KP_PLUS,     "num_plus");
  checksym(SDLK_KP_ENTER,    "num_enter");
  checksym(SDLK_KP_EQUALS,   "num_equals");
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
    
