/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 *   based on Andreas Schiffler's FPSmanager framerate routines (SDL)
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
 *
 */

#include <config.h>
#include <SDL_framerate.h>
#include <fps.h>

FPS::FPS() {
  manager = NULL;
}

FPS::~FPS() {
  if(manager) free(manager);
}
void FPS::init(int rate) {
  if(manager) free(manager);
  manager = (void*)new FPSmanager();
  SDL_initFramerate((FPSmanager*)manager);
  set(rate);
}

void FPS::delay() {
  if(!manager) return;
  SDL_framerateDelay((FPSmanager*)manager);
}

int FPS::get() {
  if(!manager) return 0;
  return SDL_getFramerate((FPSmanager*)manager);
}

int FPS::set(int rate) {
  if(!manager) return 0;
  return SDL_setFramerate((FPSmanager*)manager, rate);
}
