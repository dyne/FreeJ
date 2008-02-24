/*  FreeJ OSC controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
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
 * "$Id: $"
 *
 */

#ifndef __OSC_CTRL_H__
#define __OSC_CTRL_H__

#include <controller.h>

#include <lo/lo.h>

#include <stdlib.h>


class Context;

class OscCommand: public Entry {
 public:
  // osc_cmd is Entry::name
  char proto_cmd[128];
  char js_cmd[512];
};

class JsCommand: public Entry {
 public:
  // name is function
  jsval function;
  int argc;
  jsval *argv;

};

class OscController: public Controller {

 public:
  OscController();
  ~OscController();

  bool init(JSContext *env, JSObject *obj);
  int dispatch();
  int poll();

  bool start(int port);
  void stop();

  int peep(Context *env);
  int poll(Context *env);


  Context *freej;

  lo_server_thread srv;

  char port[64];

  Linklist commands_handled;
  Linklist commands_pending;

};

#endif
