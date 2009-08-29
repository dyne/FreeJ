/*  FreeJ
 *  (c) Copyright 2006-2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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

/**
   @file controller.h
   @brief FreeJ generic Controller interface
*/

#ifndef __CONTROLLER_H__
#define __CONTROLLER_H__

#include <config.h>
#include <cstdarg> // va_list
#include <jsapi.h> // spidermonkey header

#include <linklist.h>

class Context;
class JSContext;
class JSObject;


class Controller: public Entry {
  friend class Context;

 public:
  Controller();
  virtual ~Controller();

  // we need a pointer to context because controllers aren't depending from javascript
  // those who need it will retreive JSContext/Object from freej->js->global_context/object
  virtual bool init(Context *freej); ///< initialize the controller to be used on the Context

  // function called in main loop, 
  virtual int poll() = 0; ///< poll() is called internally at every frame processed,
  ///< returns 0 = requeue, 1 = event handled

  virtual int dispatch() = 0; ///< dispatch() is implemented by the specific controller 
  ///< distributes the signals to listeners, can be overrided in python

 
  bool initialized; ///< is this class initialized on a context?
  bool active; ///< is this class active?

  bool indestructible; ///< set to true if you want the controller to survive a reset

  bool javascript; ///< was this controller created by javascript?

  Context *env; ///< pointer to the Context

  JSContext *jsenv; ///< javascript environment
  JSObject  *jsobj; ///< this javascript object

  // TODO: eliminate runtime resolution -> C++ overhead alert!
  int JSCall(const char *funcname, int argc, jsval *argv);
  int JSCall(const char *funcname, int argc, const char *format, ...);
};

#endif
