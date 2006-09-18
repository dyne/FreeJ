/*  FreeJ
 *  (c) Copyright 2001-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <jutils.h>
#include <context.h>
#include <goom_layer.h>
#include <config.h>

static short int audio[2][512];

GoomLayer::GoomLayer()
  :Layer() {
  set_name("Goom");
  buffer = NULL;
}

GoomLayer::~GoomLayer() {
  close();
}

bool GoomLayer::init(int width, int height) {
  func("GoomLayer::init()");

  _init(width,height);
  
  goom = ::goom_init(geo.w, geo.h);

  buffer = malloc(geo.size);

//  audio = (short int**) malloc(sizeof(uint16_t*)*2);
//  audio[0] = (short int*)malloc(512*sizeof(uint16_t));
//  audio[1] = (short int*)malloc(512*sizeof(uint16_t));
//  audio[0] = audio_l;
//  audio[1] = audio_r;
  
  goom_set_screenbuffer(goom, buffer);

  return(true);
}

bool GoomLayer::open(char *file) {
  return true;
};

void GoomLayer::close() {
  if(buffer)
    free(buffer);

}

void *GoomLayer::feed() {
  goom_update(goom, audio, 0, -1, NULL, NULL);
  return buffer;
}

bool GoomLayer::keypress(char key) {
  return false;
}
