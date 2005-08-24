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

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <config.h>

// KeyboardController methods
JS(kbd_ctrl_constructor);
JS(kbd_ctrl_poll);
//JS(execute);

DECLARE_CLASS("KeyboardController", kbd_ctrl_class, kbd_ctrl_constructor);



JSFunctionSpec kbd_ctrl_methods[] = {
  { "poll",     kbd_ctrl_poll,  0 },
  //  { "execute",  kbd_ctrl_execute, 0 },
  {0}
};

static SDL_Event event;
static SDL_keysym *keysym;

JS(kbd_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  SDL_EnableKeyRepeat(200, 20);

//   if( ! JS_SetPrivate(cx, obj, (void*)kbd) ) {
//     error("JS::%s : can't set the private value");
//     delete kbd; return JS_FALSE;
//   }
  
  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;
}

JS(kbd_ctrl_poll) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  char tmp[256];
  jsval ret;

  if(!SDL_PollEvent(&event)) return JS_TRUE;

  if(event.type == SDL_QUIT) {
    notice("QUIT! QUIT!");
  }

  if(event.key.state != SDL_PRESSED) return JS_TRUE;
  
  keysym = &event.key.keysym; /* just to type less */
  
  notice("pressed %c",keysym->sym);

  snprintf(tmp,255,"%c",keysym->sym);
  notice("calling method %s()",tmp);
  JS_CallFunctionName(cx, obj, tmp, 0, NULL, &ret);

  return JS_TRUE;

}
