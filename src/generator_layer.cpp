/*  FreeJ
 *  Frei0r/FreeFrame genereator layer
 *  (c) Copyright 2007 - 2009 Denis Roio <jaromil@dyne.org>
 *
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
 */

#include <stdlib.h>

#include <config.h>

#include <generator_layer.h>
#include <frei0r_freej.h>
#include <freeframe_freej.h>
#include <fps.h>

#include <jutils.h>
#include <context.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, GeneratorLayer, GeneratorLayer, ff_f0r);

GeneratorLayer::GeneratorLayer()
  :Layer() {

  generator = NULL;
  generators = NULL;

  type = Layer::GENERATOR;
  set_name("GEN");
  //jsclass = &gen0r_layer_class;
  //  set_filename("/particle generator");
  swap_buffer = NULL;
}

GeneratorLayer::~GeneratorLayer() {
  stop();
  close();
  if (swap_buffer)
    free(swap_buffer);
}

/// set_parameter callback for generator layers
// TODO
static void set_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
#ifdef WITH_FREI0R
static void get_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
  Freior::get_parameter(static_cast<GeneratorLayer *>(lay)->generator, param, idx);
}
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
  Freior::set_parameter(static_cast<GeneratorLayer *>(lay)->generator, param, idx);
}
#endif

void GeneratorLayer::register_generators(Linklist<Filter> *gens) {
  generators = gens;
  act("%u generators available", gens->len());
}

bool GeneratorLayer::open(const char *file) {
  func("%s - %s",__PRETTY_FUNCTION__, file);
  int idx;
  Filter *proto;

  if(!generators) {
    error("No generators registered");
    return false;  }

  proto = (Filter*) generators->search(file, &idx);
  if(!proto) {
    error("generator not found: %s", file);
    return(false);
  }

  close();

  generator = Factory<FilterInstance>::new_instance("FilterInstance");
  if (!generator) return false;
  generator->init(proto);
  
#ifdef WITH_FREI0R
  if(proto->type() == Filter::FREIOR) {
    if (!static_cast<Freior *>(proto)->construct(generator, geo.w, geo.h)) {
      error("freior constructor returned NULL instantiating generator %s",file);
      delete generator;
      generator = NULL;
      return false;
    }
    parameters = &generator->parameters;
    
    Parameter *p = (Parameter*)parameters->begin();
    while(p) {
      p->layer_set_f = p->filter_set_f ? set_frei0r_layer_parameter : nullptr;
      p->layer_get_f = p->filter_get_f ? get_frei0r_layer_parameter : nullptr;
      p = (Parameter*)p->next;
    }
  }
#endif

  if(proto->type() == Filter::FREEFRAME) {
    VideoInfoStruct vidinfo;
    vidinfo.frameWidth = geo.w;
    vidinfo.frameHeight = geo.h;
    vidinfo.orientation = 1;
    vidinfo.bitDepth = FF_CAP_32BITVIDEO;
    generator->intcore = ((Freeframe *)proto)->plugmain(FF_INSTANTIATE, &vidinfo, 0).ivalue;
    if(generator->intcore == FF_FAIL) {
      error("Freeframe generator %s cannot be instantiated", name);
      delete generator;
      generator = NULL;
      return false;
    }
    // todo: parameters in freeframe
    generator->freeframe_active_ = true;
    parameters = &generator->parameters;
    Parameter *p = (Parameter*)parameters->begin();
    while(p) {
      p->layer_set_f = set_freeframe_layer_parameter;
      p->layer_get_f = get_freeframe_layer_parameter;
      p = (Parameter*)p->next;
    }
  }

  if (!generator->allocate_output(geo.bytesize)) {
    close();
    return false;
  }
  set_filename(file);
  opened = true;
  return true;
}

void GeneratorLayer::close() {
  if(generator) {
    parameters = nullptr; // owned by the generator instance
    delete generator;
    generator = NULL;
  }
  opened = false;
}

bool GeneratorLayer::_init() {
  
  //  open("lissajous0r");
  //  open("ising0r");

  if (!swap_buffer)
    swap_buffer = malloc(geo.bytesize);
  else { // if changing context ensure we can handle its resolution
    swap_buffer = realloc(swap_buffer, geo.bytesize);
  }
  return(true);
}

void *GeneratorLayer::feed() {
  void *res;
  if (generator) {
      res = generator->process(fps.get(), NULL);
      jmemcpy(swap_buffer, res, geo.bytesize);
  }
  return swap_buffer;
}

    
