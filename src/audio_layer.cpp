/*  FreeJ
 *  (c) Copyright 2008 Robin Gareus <robin@gareus.org>
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


#include <config.h>

#define WITH_AUDIO // XXX
#ifdef WITH_AUDIO

#include <audio_layer.h>
#include <jutils.h>

AudioLayer::AudioLayer()
  :Layer() {
  notice("Audio layer initialized.");
}

AudioLayer::~AudioLayer() {
  close();
}


bool AudioLayer::open(const char *file) {
 opened=true;
 return(0);
}

bool AudioLayer::init(Context *freej) {
  return init(freej, freej->screen->w, freej->screen->h);
}

bool AudioLayer::init(Context *freej, int width, int height) {
  _init(width, height);
  return true;
}

void *AudioLayer::feed() {
  return NULL;
}


void AudioLayer::close() {
  if(!opened) return;
}

#endif
