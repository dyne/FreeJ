/*  FreeJ
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#include <string.h>
#include <context.h>
#include <plugger.h>
#include <jutils.h>
#include <joy_ctrl.h>

#include <config.h>

#define DELAY 50

JoyControl::JoyControl() {
  quit = false;
  env = NULL;
  num = 0;
}

JoyControl::~JoyControl() {
}

bool JoyControl::init(Context *context) {
  func("JoyControl::init(%p)",context);

  int found = 0;
  int c, cc;
  if(!context) return false;

  num = SDL_NumJoysticks();
  func("num joysticks %i",num);
  for(c=0;c<num;c++) {
    joy[found] = SDL_JoystickOpen(c);
    if(joy[found]) {
      if(strstr(SDL_JoystickName(c),"Keyboard")) {
	/* this is not a joystick! it happens on MacOSX
	   to have "Apple Extended USB Keyboard" recognized as joystick */
	SDL_JoystickClose(joy[found]);
	continue;
      }
      notice("Joystick: %s",SDL_JoystickName(c));
      axes = SDL_JoystickNumAxes(joy[found]);
      buttons = SDL_JoystickNumButtons(joy[found]);
      balls = SDL_JoystickNumBalls(joy[found]);
      hats = SDL_JoystickNumHats(joy[found]);
      act("%i axes, %i balls, %i hats, %i buttons",
	  axes, balls, hats, buttons);
      found++;
    } else {
      error("error opening %s",SDL_JoystickName(c));
    }
  }

  num = found;

  if(!num) {
    notice("no joystick found");
    return(false);
  }
  
  this->env = context;
  start();
  return(true);
}

void JoyControl::run() {
  int c;
  SDL_Event event;

  while(!quit) {

    SDL_Delay(DELAY);
    
    /* trap joystick events */
    if(!SDL_PollEvent(&event)) continue;
    if(!(event.type & (SDL_JOYAXISMOTION|SDL_JOYBUTTONDOWN))) continue;
    debug();
  }

  for(c=0;c<num;c++)
    SDL_JoystickClose(joy[c]);

}

void JoyControl::debug() {
  int j,c;
  for(j=0;j<num;j++) {
    func("action on %s",SDL_JoystickName(j));
    /* print out axes */
    for(c=0;c<axes;c++)
      func("axis %i position %i",c,SDL_JoystickGetAxis(joy[j],c));
    for(c=0;c<buttons;c++)
      func("button %i position %i",c,SDL_JoystickGetButton(joy[j],c));
  }
}
