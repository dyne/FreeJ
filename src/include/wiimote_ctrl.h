/*  FreeJ WiiMote controller
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

#ifndef __WII_CTRL_H__
#define __WII_CTRL_H__

#include <controller.h>

extern "C" {
#include <wiimote.h>
#include <extension.h>
}

#include <stdlib.h>

class Context;

class WiiController: public Controller {

 public:
  WiiController();
  ~WiiController();

  bool init(JSContext *env, JSObject *obj);
  int dispatch();
  int poll();

  bool connect(char *hwaddr);

  Context *freej;

  /* TODO: support for multiple wiimotes
     can be done, but i don't need it ATM
     if needed: ask. -jrml */

 private:

  wm_conn_t connection;
  wm_wiimote_t wiimote;
  bdaddr_t wiimote_bdaddr;

  // input structures
  wm_input_t *input;
  wm_nunchuk_t nunchuk;
  wm_classic_t classic;

};

#endif
