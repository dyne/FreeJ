/*  FreeJ
 *  (c) Copyright 2001-2009 Denis Roio <jaromil@dyne.org>
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
 */


#ifndef __MOUSE_CTRL_H__
#define __MOUSE_CTRL_H__

#include <sdl_controller.h>
#include <factory.h>

class MouseController : public SdlController {
  
 public:
  MouseController();
  virtual ~MouseController();
  
  int  poll();
  virtual int dispatch();

  virtual int motion(int state, int x, int y, int xrel, int yrel);
  virtual int button(int button, int state, int x, int y);
  
  void grab(bool state);

 private:

  FACTORY_ALLOWED;
};

#endif
