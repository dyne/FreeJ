/*  FreeJ
 *  Frei0r particle generator layer
 *  (c) Copyright 2007 Denis Roio aka jaromil <jaromil@dyne.org>

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

$Id: gen_layer.cpp 845 2007-04-03 07:04:47Z jaromil $

 */

#include <stdlib.h>
#include <gen_f0r_layer.h>
#include <frei0r_freej.h>
#include <jutils.h>
#include <context.h>
#include <config.h>


GenF0rLayer::GenF0rLayer()
  :Layer() {

  generator = NULL;

  type = F0R_GENERATOR_LAYER;
  set_name("F0R");
  //  set_filename("/particle generator");

}

GenF0rLayer::~GenF0rLayer() {
  close();
}

bool GenF0rLayer::open(char *file) {
  /* we don't need this */
  int idx;
  Filter *proto;

  proto = (Filter*) env->generators.search(file, &idx);
  if(!proto) {
    error("generator not found: %s", file);
    return(false);
  }

  close();

  generator = new FilterInstance((Filter*)this);
  generator->core = (void*)(*proto->freior->f0r_construct)(geo.w, geo.h);
  if(!generator->core) {
    error("freior constructor returned NULL instantiating generator %s",file);
    delete generator;
    generator = NULL;
    return false;
  }
  generator->outframe = (uint32_t*) calloc(geo.size, 1);
  
  set_filename(file);
  opened = true;
  return true;
}

void GenF0rLayer::close() {
  if(generator) {
    delete generator;
    generator = NULL;
  }
  opened = false;
}

bool GenF0rLayer::init(Context *freej) {

  
  int width  = freej->screen->w;
  int height = freej->screen->h;
 

  /* internal initalization */
  _init(width,height);

  open("lissajous0r");
  env = freej;
  
  return(true);
}

void *GenF0rLayer::feed() {
  return generator->process(env->fps, NULL);
}

bool GenF0rLayer::keypress(int key) {
  return(false);
}
    
