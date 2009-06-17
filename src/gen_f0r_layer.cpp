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
#include <freeframe_freej.h>
#include <fps.h>

#include <jutils.h>
#include <context.h>
#include <config.h>
//#include <jsparser_data.h>


GenF0rLayer::GenF0rLayer()
  :Layer() {

  generator = NULL;

  type = Layer::F0R_GENERATOR;
  set_name("F0R");
  //jsclass = &gen0r_layer_class;
  //  set_filename("/particle generator");
  swap_buffer = NULL;
}

GenF0rLayer::~GenF0rLayer() {
  close();
  if (swap_buffer)
    free(swap_buffer);
}

/// set_parameter callback for generator layers
// TODO
static void set_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
  GenF0rLayer *layer = (GenF0rLayer*)lay;

  Freior *f = layer->generator->proto->freior;
  bool *val = (bool*)param->value;

  switch(f->param_infos[idx-1].type) {
    
    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:
    (*f->f0r_set_param_value)
      (layer->generator->core, new f0r_param_bool(val[0]), idx-1);
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
      // QUAAA: should we delete the new allocated object? -jrml
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      position->x = val[0];
      position->y = val[1];
      (*f->f0r_set_param_value)(layer->generator->core, position, idx-1);
      // QUAAA: should we delete the new allocated object? -jrml
    } break;

  default:

    error("Unrecognized parameter type %u for set_parameter_value",
	  f->param_infos[idx].type);
  }
}

bool GenF0rLayer::open(const char *file) {
  int idx;
  Filter *proto;

  proto = (Filter*) env->generators.search(file, &idx);
  if(!proto) {
    error("generator not found: %s", file);
    return(false);
  }

  close();

  generator = new FilterInstance((Filter*)proto);
  
  if(proto->freior) {
    generator->core = (void*)(*proto->freior->f0r_construct)(geo.w, geo.h);
    if(!generator->core) {
      error("freior constructor returned NULL instantiating generator %s",file);
      delete generator;
      generator = NULL;
      return false;
    }
    parameters = &proto->parameters;
    
    Parameter *p = (Parameter*)parameters->begin();
    while(p) {
      p->layer_set_f = set_frei0r_layer_parameter;
      p->layer_get_f = get_frei0r_layer_parameter;
      p = (Parameter*)p->next;
    }
  }

  if(proto->freeframe) {
    VideoInfoStruct vidinfo;
    vidinfo.frameWidth = geo.w;
    vidinfo.frameHeight = geo.h;
    vidinfo.orientation = 1;
    vidinfo.bitDepth = FF_CAP_32BITVIDEO;
    generator->intcore = proto->freeframe->main(FF_INSTANTIATE, &vidinfo, 0).ivalue;
    if(generator->intcore == FF_FAIL) {
      error("Freeframe generator %s cannot be instantiated", name);
      delete generator;
      generator = NULL;
      return false;
    }
    // todo: parameters in freeframe
    parameters = &proto->parameters;
    Parameter *p = (Parameter*)parameters->begin();
    while(p) {
      p->layer_set_f = set_freeframe_layer_parameter;
      p->layer_get_f = get_freeframe_layer_parameter;
      p = (Parameter*)p->next;
    }
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

  env = freej; 

  /* internal initalization */
  _init(width,height);

  //  open("lissajous0r");
  //  open("ising0r");

  if (!swap_buffer)
    swap_buffer = malloc(freej->screen->size);
  else { // if changing context ensure we can handle its resolution
    swap_buffer = realloc(swap_buffer, freej->screen->size);
  }
  return(true);
}

void *GenF0rLayer::feed() {
  void *res;
  if (env && generator) {
      res = generator->process(env->fps.get(), NULL);
      jmemcpy(swap_buffer, res, env->screen->size);
  }
  return swap_buffer;
}

    
