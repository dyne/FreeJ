/*  FreeJ
 *  (c) Copyright 2006 Denis Roio aka jaromil <jaromil@dyne.org>
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

 * Virtual controller class to be inherited by other controllers

 */


#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <config.h>
#include <linklist.h>
//#include <callbacks_js.h> // javascript
#include <SDL/SDL.h>
//#include <jstypes.h>
#include <cstdarg> // va_list
#include <jsapi.h> // spidermonkey header

class Context;
class JSContext;
class JSObject;

class Controller: public Entry {
  friend class Context;

 public:
  Controller();
  virtual ~Controller();

  char *get_name();

  // store the javascript environment!
  virtual bool init(JSContext *env, JSObject *obj) =0;
  // function called in main loop, 
  // handle your events here
  virtual int poll() = 0;
  // call this or by poll_sdlevents, return 0 = requeue, 1 = event handled
  virtual int dispatch() = 0;

  // helper function to filter and redispatch unhandled SDL_Events
  // calls dispatch() foreach event in eventmask
  void poll_sdlevents(Uint32 eventmask);

  bool initialized;
  bool active;
  virtual bool activate(bool);

  JSContext *jsenv;
  JSObject  *jsobj;
  SDL_Event event;
  jsval lol;

  // TODO: eliminate runtime resolution -> C++ overhead alert!
  int JSCall(const char *funcname, int argc, jsval *argv, JSBool *res);
  int JSCall(const char *funcname, int argc, const char *format, ...);
};

#endif
