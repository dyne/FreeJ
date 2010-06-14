/*  FreeJ - New Freior based Filter class
 *  Copyright (C) 2001-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo <xant@dyne.org>
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

$Id: $

 */

#include <config.h>
#include <layer.h>
#include <filter.h>

#include <frei0r_freej.h>
#include <freeframe_freej.h>

#include <jutils.h>

Filter::Filter() 
  : Entry()
{
  int i;

  initialized = false;
  active = false;
  inuse = false;

  bytesize = 0;
    
}

static const char *KnownFilters[] =
{
#ifdef WITH_FREI0R
    "Freior",
#endif
#ifdef WITH_COCOA
    "CoreImage",
#endif
    "FreeFrame"
};

Filter::~Filter() {

}

FilterInstance *Filter::new_instance() {
    FilterInstance *instance = Factory<FilterInstance>::new_instance("FilterInstance");
    if (instance)
        instance->init(this);
    return instance;
}

bool Filter::apply(Layer *lay, FilterInstance *instance)
{
    
    errno=0;
    instance->outframe = (uint32_t*) calloc(lay->geo.bytesize, 1);
    if(errno != 0) {
        error("calloc outframe failed (%i) applying filter %s",errno, name);
        error("Filter %s cannot be instantiated", name);
        delete instance;
        return NULL;
    }
    
    bytesize = lay->geo.bytesize;
    
    lay->filters.append(instance);
    
    act("initialized filter %s on layer %s", name, lay->name);
    
    instance->set_layer(lay);
    
    return true;    
}

FilterInstance *Filter::apply(Layer *lay) {

  FilterInstance *instance = new_instance();
  
  if (apply(lay, instance))
      return instance;
  delete instance;
  return NULL;
}

const char *Filter::description() {
    return "Unknown"; // TODO - use a more meaningful default
}

const char *Filter::author()  {
    return "Unknown";
}


int Filter::get_parameter_type(int i) {
  return -1; // this method must be extended by subclasses (perhaps it should be pure virtual?)
}

char *Filter::get_parameter_description(int i) {
  return (char *)"Unknown";
}

void Filter::destruct(FilterInstance *inst) {


}

void Filter::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {


}

