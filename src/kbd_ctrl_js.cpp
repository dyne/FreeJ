/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: freej.cpp 654 2005-08-18 16:52:47Z jaromil $"
 *
 */

#include <SDL/SDL.h>

#include <kbd_ctrl.h>
#include <callbacks_js.h>
#include <jsparser_data.h>
#include <config.h>

// KeyboardController methods
JS(kbd_ctrl_constructor);
JS(kbd_ctrl_poll);
//JS(execute);

DECLARE_CLASS("KeyboardController", kbd_ctrl_class, kbd_ctrl_constructor);


JSFunctionSpec kbd_ctrl_methods[] = {
  { "poll",     kbd_ctrl_poll,  1 },
  {0}
};

JS(kbd_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  KbdListener *kbd = new KbdListener();

  if( ! JS_SetPrivate(cx, obj, (void*)kbd) ) {
    error("JS::%s : can't set the private value");
    delete kbd; return JS_FALSE;
  }
  
  kbd->init();

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

#define CHECKSYM(key,name) \
  if(kbd->keysym->sym == key) { \
    strcat(funcname,name); \
    JS_CallFunctionName(cx, obj, funcname, 0, NULL, &ret); \
    return JS_TRUE; \
  }
    


JS(kbd_ctrl_poll) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  char tmp[8];
  char funcname[512];

  jsval ret;


  // get the core class out
  KbdListener *kbd = (KbdListener *) JS_GetPrivate(cx,obj);
  if(!kbd) {
    error("%u:%s:%s :: KeyboardController core data is NULL",
	  __LINE__,__FILE__,__FUNCTION__);
    return JS_FALSE;
  }

  // if we have no events we just go on
  if(!kbd->poll()) return JS_TRUE;


  // quit() callback
  /* the quit even in SDL is not concerning the keyboard
  if(kbd->event.type == SDL_QUIT) {
    notice("QUIT invoked from Javascript (TODO)");
    JS_CallFunctionName(cx, obj, "quit", 0, NULL, &ret);
    return JS_TRUE;
    } */

  // pointer to the function name being formed
  //  memset(funcname, 512, 0x0);

  if(kbd->event.key.state == SDL_PRESSED)
    strcpy(funcname,"pressed_");
  else if(kbd->event.key.state == SDL_RELEASED)
    strcpy(funcname,"released_");
  else // no key state change
    return JS_TRUE;

  // check key modifiers
  if(kbd->keysym->mod & KMOD_CTRL)
    strcat(funcname,"ctrl_");

  if(kbd->keysym->mod & KMOD_SHIFT)
    strcat(funcname,"shift_");

  if(kbd->keysym->mod & KMOD_ALT)
    strcat(funcname,"alt_");


  // check normal alphabet and letters
  if( kbd->keysym->sym >= SDLK_0
      && kbd->keysym->sym <= SDLK_9) {
    tmp[0] = kbd->keysym->sym;
    tmp[1] = 0x0;
    strcat(funcname,tmp);
    JS_CallFunctionName(cx, obj, funcname, 0, NULL, &ret);
    //    notice("calling method %s()",funcname);
    return JS_TRUE;
  }

  if( kbd->keysym->sym >= SDLK_a
      && kbd->keysym->sym <= SDLK_z) {
    tmp[0] = kbd->keysym->sym;
    tmp[1] = 0x0;
    strcat(funcname,tmp);
    JS_CallFunctionName(cx, obj, funcname, 0, NULL, &ret);
    //    notice("calling method %s()",funcname);
    return JS_TRUE;
  }    
  ///////

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
  if(kbd->keysym->sym >= SDLK_KP0
     && kbd->keysym->sym <= SDLK_KP9) {
    tmp[0] = kbd->keysym->sym - SDLK_KP0 + 48;
    tmp[1] = 0x0;
    strcat(funcname,"num_");
    strcat(funcname,tmp);
    JS_CallFunctionName(cx, obj, funcname, 0, NULL, &ret);
    return JS_TRUE;
  }
  CHECKSYM(SDLK_KP_PERIOD,   "num_period");
  CHECKSYM(SDLK_KP_DIVIDE,   "num_divide");
  CHECKSYM(SDLK_KP_MULTIPLY, "num_multiply");
  CHECKSYM(SDLK_KP_MINUS,    "num_minus");
  CHECKSYM(SDLK_KP_PLUS,     "num_plus");
  CHECKSYM(SDLK_KP_ENTER,    "num_enter");
  CHECKSYM(SDLK_KP_EQUALS,   "num_equals");
  //////
  
  return JS_TRUE;

}
