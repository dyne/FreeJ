/*  FreeJ - FilterInstance base class
 *
 *  Copyright (C) 2001-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo <xant@dyne.org>
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

$Id:$

 */

#include <config.h>
#include <layer.h>
#include <filter.h>

#include <jutils.h>

FACTORY_REGISTER_INSTANTIATOR(FilterInstance, FilterInstance, FilterInstance, core);


FilterInstance::FilterInstance()
 : Entry() 
{
  core = NULL;
  intcore = 0;
  outframe = NULL;
  active = false;
  layer = NULL;
}

FilterInstance::FilterInstance(Filter *fr)
	: Entry()
{
  core = NULL;
  intcore = 0;
  outframe = NULL;
  active = false;
  layer = NULL;
  init(fr);
}

FilterInstance::~FilterInstance() {
  func("~FilterInstance");

  if(proto)
    proto->destruct(this);

  if(outframe)
    free(outframe);

}

void FilterInstance::init(Filter *fr)
{
  func("initializing instance for filter %s",fr->name);
  proto = fr;
  set_name(proto->name);
  active = true;
}

uint32_t *FilterInstance::process(float fps, uint32_t *inframe) {
  if(!proto) {
    error("void filter instance was called for process: %p", this);
    return inframe;
  }
  proto->update(this, fps, inframe, outframe);
  return outframe;
}

bool FilterInstance::set_parameter(int idx) {
  Parameter *param;
  param = (Parameter*)proto->parameters[idx];

  if( ! param) {
    error("parameter [%u] not found in filter %s", idx, proto->name );
    return false;
  } else {
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);
  }

  if(!param->filter_set_f) {
    error("no filter callback function registered in this parameter");
    return false;
  }

  (*param->filter_set_f)(this, param, idx);

  return true;
}

bool FilterInstance::get_parameter(int idx) {
  Parameter *param;
  param = (Parameter*)proto->parameters[idx];

  if( ! param) {
    error("parameter %s not found in filter %s", param->name, proto->name );
    return false;
  } else {
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);
  }

  if(!param->filter_get_f) {
    error("no filter callback function registered in this parameter");
    return false;
  }

  (*param->filter_get_f)(this, param, idx);

  return true;
}

bool FilterInstance::apply(Layer *lay)
{
    bool ret = false;
    if (!layer && proto)
        ret = proto->apply(lay, this);
    return ret;
}
 
void FilterInstance::set_layer(Layer *lay)
{
    layer = lay;
}

Layer *FilterInstance::get_layer()
{
    return layer;
}

bool FilterInstance::inuse()
{
    if (layer)
        return true;
    return false;
}
