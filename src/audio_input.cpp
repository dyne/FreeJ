/*  FreeJ audio input
 *
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *                
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
 */

#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <audio_input.h>
#include <jutils.h>


AudioInput::AudioInput() {
  func("creating audio input");

  //  set_format(1, 44100); // default mono 44khz

  audio = NULL;

  initialized = false;
  started = false;
}

AudioInput::~AudioInput() {

  if(audio) delete audio;

}

bool AudioInput::init() {
  notice("Initializing audio input jack");

  audio = new AudioCollector(name, 2048, 44100);

  if(!audio) return(false);

  act("audio initialization succesful");
  initialized = true;

  return true;
  
}

  

