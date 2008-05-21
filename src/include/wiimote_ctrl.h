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
#include <cwiid.h>
}

class Context;

class WiiController: public Controller {

 public:
  WiiController();
  ~WiiController();

  bool init(JSContext *env, JSObject *obj);
  int dispatch();
  int poll();

  bool connect(char *hwaddr);

  void accel(uint8_t nx, uint8_t ny, uint8_t nz);
  void button(uint16_t buttons);

  Context *freej;

  int get_battery();

  int print_state(); // debug

  cwiid_wiimote_t  *wiimote;
  int  x,  y,  z;
  cwiid_state state;

 private:

  int nx, ny, nz;

  uint16_t newbutt;
  uint16_t oldbutt;

  // todo nunchuk_state and classic_state extensions

  bdaddr_t bdaddr;

};

#endif
