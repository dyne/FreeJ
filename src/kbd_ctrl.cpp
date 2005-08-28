/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <iostream>

#include <context.h>
#include <video_encoder.h>
#include <jutils.h>
#include <filter.h>
#include <plugger.h>
#include <kbd_ctrl.h>
#include <config.h>
#include <sdlgl_screen.h>

#define SDL_REPEAT_DELAY	200
#define SDL_REPEAT_INTERVAL	20

#define DELAY 50 //100

KbdListener::KbdListener() {

}

KbdListener::~KbdListener() {

}

bool KbdListener::init() {

  active = true;

  /* enable key repeat */
  SDL_EnableKeyRepeat(SDL_REPEAT_DELAY, SDL_REPEAT_INTERVAL);

  return(true);
}

bool KbdListener::poll() {
  bool res;
  res = SDL_PollEvent(&event);
  keysym = &event.key.keysym; // just to type less
  return res;
}
