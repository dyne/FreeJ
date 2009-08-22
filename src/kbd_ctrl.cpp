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

#include <config.h>
#include <kbd_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <jsparser.h>
#include <callbacks_js.h> // javascript
#include <jsparser_data.h>
#include <factory.h>

#define SDL_REPEAT_DELAY	200
#define SDL_REPEAT_INTERVAL	20

#define SDL_KEYEVENTMASK (SDL_KEYDOWNMASK|SDL_KEYUPMASK)

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(controller, KbdController, KeyboardController, sdl);

KbdController::KbdController()
  :Controller() {
  set_name("Keyboard");
  func("%s this=%p",__PRETTY_FUNCTION__, this);
}

KbdController::~KbdController() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);

}

bool KbdController::init(Context *freej) {
  func("%s",__PRETTY_FUNCTION__);

  /* enable key repeat */
  SDL_EnableKeyRepeat(SDL_REPEAT_DELAY, SDL_REPEAT_INTERVAL);


  env = freej;
  jsenv = freej->js->global_context;
  
  initialized = true;
  
  SDL_EnableUNICODE(1);
  
  return(true);
}

int KbdController::poll() {
	poll_sdlevents(SDL_KEYEVENTMASK); // calls dispatch() foreach SDL_Event
	return 1;
}

int KbdController::key_event(const char *state, bool shift, bool ctrl, bool alt, bool num, const char *keyname) {

  JSBool res;

  Uint16 uni[] = {keysym->unicode, 0};
  //#snprintf(uni, 2, "X %s X", (char*)&keysym->unicode);
  // universal call
  if (JSCall("key", 7, "buusWuu",
  		event.key.state,
  		keysym->scancode,
  		keysym->sym,
  		SDL_GetKeyName(keysym->sym),
  		uni,
  		keysym->mod,
  		event.key.which
  		) )
  	return 1; // returned true, we are done!
  
  //Uint16 keysym->unicode
  //char * SDL_GetKeyName(keysym->sym);
  //func("KB u: %u / ks: %s", keysym->unicode, SDL_GetKeyName(keysym->sym));

  // funcname = "state_[shift_][ctrl_][alt_][num_]keyname"
  if(strlen(keyname)) {
	  sprintf(funcname, "%s_%s%s%s%s%s",
			  state,
			  (shift? "shift_" : ""),
			  (ctrl?  "ctrl_"  : ""),
			  (alt?   "alt_"   : ""),
			  (num?   "num_"   : ""),
			  keyname);

	  func("%s calling method %s()", __func__, funcname);
	  jsval fval = JSVAL_VOID;
	  return JSCall(funcname, 0, &fval);
  }

  return 0;
}

int KbdController::dispatch() {
  char tmp[8];
  const char *state;
  bool shift = false, ctrl = false, alt = false, num = false;

  if(event.key.state == SDL_PRESSED)
  	state = "pressed";
  else if(event.key.state == SDL_RELEASED)
  	state = "released";
  else
  	return 0; // no key state change

  keysym = &event.key.keysym;

  memset(keyname, 0, sizeof(char)<<9);  // *512
  memset(funcname, 0, sizeof(char)<<9); // *512

  // check key modifiers
  if(keysym->mod & KMOD_SHIFT)
    shift = true;
  if(keysym->mod & KMOD_CTRL)
    ctrl = true;
  if(keysym->mod & KMOD_ALT)
    alt = true;
  
  // check normal alphabet and letters
  if( (keysym->sym >= SDLK_0 && keysym->sym <= SDLK_9)
		  || (keysym->sym >= SDLK_a && keysym->sym <= SDLK_z) ){
	  tmp[0] = keysym->sym;
	  tmp[1] = 0x0;
	  sprintf(keyname,"%s", tmp);
  }
  // check numeric keypad
  else if(keysym->sym >= SDLK_KP0 && keysym->sym <= SDLK_KP9) {
	  tmp[0] = keysym->sym - SDLK_KP0 + 48;
	  tmp[1] = 0x0;
	  num = true;
	  sprintf(keyname, "%s", tmp);
  } else {
	  switch(keysym->sym) {
		  // check arrows
		  case SDLK_UP:        sprintf(keyname,        "up"); break;
		  case SDLK_DOWN:      sprintf(keyname,      "down"); break;
		  case SDLK_RIGHT:     sprintf(keyname,     "right"); break;
		  case SDLK_LEFT:      sprintf(keyname,      "left"); break;
		  case SDLK_INSERT:    sprintf(keyname,    "insert"); break;
		  case SDLK_HOME:      sprintf(keyname,      "home"); break;
		  case SDLK_END:       sprintf(keyname,       "end"); break;
		  case SDLK_PAGEUP:    sprintf(keyname,    "pageup"); break;
		  case SDLK_PAGEDOWN:  sprintf(keyname,  "pagedown"); break;
		  // check special keys
		  case SDLK_BACKSPACE: sprintf(keyname, "backspace"); break;
		  case SDLK_TAB:       sprintf(keyname,       "tab"); break;
		  case SDLK_RETURN:    sprintf(keyname,    "return"); break;
		  case SDLK_SPACE:     sprintf(keyname,     "space"); break;
		  case SDLK_PLUS:      sprintf(keyname,      "plus"); break;
		  case SDLK_MINUS:     sprintf(keyname,     "minus"); break;
		  case SDLK_ESCAPE:    sprintf(keyname,       "esc"); break;
		  case SDLK_LESS:      sprintf(keyname,      "less"); break;
		  case SDLK_GREATER:   sprintf(keyname,   "greater"); break;
		  case SDLK_EQUALS:    sprintf(keyname,     "equal"); break;
		  // check numeric keypad special keys
		  case SDLK_KP_PERIOD:   num = true; sprintf(keyname,   "period"); break;
		  case SDLK_KP_DIVIDE:   num = true; sprintf(keyname,   "divide"); break;
		  case SDLK_KP_MULTIPLY: num = true; sprintf(keyname, "multiply"); break;
		  case SDLK_KP_MINUS:    num = true; sprintf(keyname,    "minus"); break;
		  case SDLK_KP_PLUS:     num = true; sprintf(keyname,     "plus"); break;
		  case SDLK_KP_ENTER:    num = true; sprintf(keyname,    "enter"); break;
		  case SDLK_KP_EQUALS:   num = true; sprintf(keyname,   "equals"); break;
		  default: break;
	  }

  }
  return key_event(state, shift, ctrl, alt, num, keyname);
}

