/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __KBD_CTRL_H__
#define __KBD_CTRL_H__

#include <sdl_controller.h>
#include <factory.h>

#include <SDL.h>

class KbdController: public SdlController {

 public:
  KbdController();
  virtual ~KbdController();

  bool init(Context *freej);
  int  poll();
  virtual int dispatch();
  virtual int key_event(const char *state, bool shift, bool ctrl, bool alt, bool num, const char *keyname);

 private:  
  SDL_keysym *keysym;
  char keyname[512];
  char funcname[512];
  // allow to use Factory on this class
  FACTORY_ALLOWED
};

#endif
