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


#ifndef __JOY_CTRL_H__
#define __JOY_CTRL_H__

#include <sdl_controller.h>

#include <SDL2/SDL.h>

#include <config.h>

#ifdef HAVE_LINUX
  // joystick rumble
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#endif

class JoyController : public SdlController {

 public:
  JoyController();
  virtual ~JoyController();
  
  bool init(Context *freej);
  int  poll();
  virtual int dispatch();
  
  virtual int axismotion(int device, int axis, int value);
  virtual int ballmotion(int device, int ball, int xrel, int yrel);
  virtual int hatmotion(int device, int hat, int value);
  virtual int button_down(int device, int button);
  virtual int button_up(int device, int button);

#ifdef HAVE_LINUX
  // joystick rumble

#define N_EFFECTS 6
  bool init_rumble(char *devpath);
  bool rumble(int intensity);
  int rumble_fd;
  struct ff_effect effects[N_EFFECTS];
  struct input_event play, stop;
#endif

 private:
  SDL_Joystick *joy[4];
  int num;
  int axes;
  int buttons;
  int balls;
  int hats;


};

#endif
