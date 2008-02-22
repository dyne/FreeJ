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

#include <linklist.h>

class Context;
class JSContext;
class JSObject;

class Controller: public Entry {
  friend class Context;

 public:
  Controller();
  virtual ~Controller();

  char *get_name();

  virtual int peep(Context *env) =0;
  ///< virtual function to be implemented, peeps interesting events in the queue and calls poll

  virtual bool init(JSContext *env, JSObject *obj) =0;
  ///< virtual function to be implemented, receives the javascript environment

  virtual int poll(Context *env)  =0;
  ///< virtual function called in main loop, receives global freej environment

  bool initialized;
  bool active;
  virtual bool activate(bool);

  JSContext *jsenv;
  JSObject  *jsobj;

};

#endif
