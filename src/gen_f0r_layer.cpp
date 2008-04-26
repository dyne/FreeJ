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
//#include <jsparser_data.h>


GenF0rLayer::GenF0rLayer()
  :Layer() {

  generator = NULL;

  type = F0R_GENERATOR_LAYER;
  set_name("F0R");
  //jsclass = &gen0r_layer_class;
  //  set_filename("/particle generator");

}

GenF0rLayer::~GenF0rLayer() {
  close();
}

/// set_parameter callback
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
  GenF0rLayer *layer = (GenF0rLayer*)lay;

  Freior *f = layer->generator->proto->freior;
  bool *val = (bool*)param->value;

  switch(f->param_infos[idx-1].type) {
    
    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:
    (*f->f0r_set_param_value)(layer->generator->core, new f0r_param_bool(val[0]), idx-1);
    break;
    
  case F0R_PARAM_DOUBLE:
    (*f->f0r_set_param_value)(layer->generator->core, new f0r_param_double(val[0]), idx-1);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      color->r = val[0];
      color->g = val[1];
      color->b = val[2];
      (*f->f0r_set_param_value)(layer->generator->core, color, idx-1);
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      position->x = val[0];
      position->y = val[1];
      (*f->f0r_set_param_value)(layer->generator->core, position, idx-1);
    } break;

  default:

    error("Unrecognized parameter type %u for set_parameter_value",
	  f->param_infos[idx].type);
  }
}

bool GenF0rLayer::open(char *file) {
  int idx;
  Filter *proto;

  proto = (Filter*) env->generators.search(file, &idx);
  if(!proto) {
    error("generator not found: %s", file);
    return(false);
  }

  close();

  generator = new FilterInstance((Filter*)proto);
  generator->core = (void*)(*proto->freior->f0r_construct)(geo.w, geo.h);
  if(!generator->core) {
    error("freior constructor returned NULL instantiating generator %s",file);
    delete generator;
    generator = NULL;
    return false;
  }
  generator->outframe = (uint32_t*) calloc(geo.size, 1);
  parameters = &proto->parameters;

  Parameter *p = (Parameter*)parameters->begin();
  while(p) {
    p->layer_func = set_frei0r_layer_parameter;
    p = (Parameter*)p->next;
  }

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

  env = freej; 

  /* internal initalization */
  _init(width,height);

  //  open("lissajous0r");
  open("ising0r");

  
  return(true);
}

void *GenF0rLayer::feed() {
  return generator->process(env->fps_speed, NULL);
}

bool GenF0rLayer::keypress(int key) {
  return(false);
}
    
