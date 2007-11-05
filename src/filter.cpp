/*  FreeJ - New Freior based Filter class
 *  (c) Copyright 2007 Denis Rojo <jaromil@dyne.org>
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
 * "$Id: $"
 *
 */

#include <config.h>
#include <layer.h>
#include <filter.h>

#include <frei0r_freej.h>

#include <jutils.h>


Parameter::Parameter(int param_type) {
  switch(param_type) {
  case PARAM_BOOL:
    value = calloc(1, sizeof(bool));
    break;
  case PARAM_NUMBER:
    value = calloc(1, sizeof(float));
    break;
  case PARAM_COLOR:
    value = calloc(3, sizeof(float));
    break;
  case PARAM_POSITION:
    value = calloc(2, sizeof(float));
    break;
  case PARAM_STRING:
    value = calloc(512, sizeof(char));
    break;
  default:
    error("parameter initialized with unknown type: %u", param_type);
  }

  type = param_type;
}

Parameter::~Parameter() {
  free(value);
}

void *Parameter::get_value() {
  return value;
}

void Parameter::set_value(void *val) {
  if      (type==PARAM_NUMBER)
    *(float*)value = *(float*)val;
  else if (type==PARAM_BOOL)
    *(bool*)value = *(bool*)val;
  else if (type==PARAM_POSITION) {
    ((float*)value)[0] = ((float*)val)[0];
    ((float*)value)[1] = ((float*)val)[1];
  }
  else if (type==PARAM_COLOR) {
    ((float*)value)[0] = ((float*)val)[0];
    ((float*)value)[1] = ((float*)val)[1];
    ((float*)value)[2] = ((float*)val)[2];
  }
  else if (type==PARAM_STRING)
    strcpy((char*)value, (char*)val);
  else
    error("attempt to set value for a parameter of unknown type: %u", type);
}





Filter::Filter(Freior *f) 
  : Entry() {
  int i;

  initialized = false;
  active = false;
  inuse = false;

  // critical errors:
  if(!f) error("Filter constructor received a NULL Freior object");
  if(!f->opened) error("Filter constructor received a Freior object that is not open");


  (*f->f0r_init)();

  // Get the list of params.
  f->param_infos.resize(f->info.num_params);
  for (i = 0; i < f->info.num_params; ++i) {

    (*f->f0r_get_param_info)(&f->param_infos[i], i);

    Parameter *param = new Parameter(f->param_infos[i].type);
    param->name = f->param_infos[i].name;
    param->description = f->param_infos[i].explanation;
    parameters.append(param);
  }

  freior = f;

  if(get_debug()>2)
    freior->print_info();

  set_name((char*)f->info.name);

}

Filter::~Filter() {
  delete freior;
}

FilterInstance *Filter::apply(Layer *lay) {

  FilterInstance *instance;
  instance = new FilterInstance(this);
  instance->core = (void*)(*freior->f0r_construct)(lay->geo.w, lay->geo.h);

  if(!instance->core) {
    error("freior constructor returned NULL applying filter %s",name);
    delete instance;
    return NULL;
  }
  errno=0;
  instance->outframe = (uint32_t*) calloc(lay->geo.size, 1);
  if(errno != 0) {
    error("calloc outframe failed (%i) applying filter %s",name, errno);
    delete instance;
    return NULL;
  }
  
  lay->filters.append(instance);

  act("initialized filter %s on layer %s", name, lay->name);

  // here maybe keep a list of layers that possess instantiantions of this filter?

  return instance;

}

char *Filter::description() {
  return (char*)freior->info.explanation;
}

int Filter::get_parameter_type(int i) {
  return freior->param_infos[i].type;
}

char *Filter::get_parameter_description(int i) {
  return (char*)freior->param_infos[i].explanation;
}

bool Filter::set_parameter_value(FilterInstance *inst, double *value, int idx) {

  switch(freior->param_infos[idx].type) {

  case F0R_PARAM_BOOL:
    (*freior->f0r_set_param_value)(inst->core, new f0r_param_bool(value[0]), idx);
    break;

  case F0R_PARAM_DOUBLE:
    (*freior->f0r_set_param_value)(inst->core, new f0r_param_double(value[0]), idx);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      color->r = value[0]; color->g = value[1]; color->b = value[2];
      (*freior->f0r_set_param_value)(inst->core, color, idx);
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      position->x = value[0]; position->y = value[1];
      (*freior->f0r_set_param_value)(inst->core, position, idx);
    } break;

  default:
    error("Unrecognized parameter type for set_parameter_value");
  }

  return(true);

}

void Filter::destruct(FilterInstance *inst) {
  if(inst->core) {
    (*freior->f0r_destruct)((f0r_instance_t*)inst->core);
    inst->core = NULL;
  }
}

void Filter::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {
  (*freior->f0r_update)((f0r_instance_t*)inst->core, time, inframe, outframe);
}


FilterInstance::FilterInstance(Filter *fr)  {
  proto = fr;

  core = NULL;
  outframe = NULL;
  
  active = true;

  set_name(proto->name);
}

FilterInstance::~FilterInstance() {
  func("~FilterInstance");
  if(proto)
    if(core) proto->destruct(this);
  if(outframe) free(outframe);
  outframe=NULL;
}

uint32_t *FilterInstance::process(float fps, uint32_t *inframe) {
  if(!proto) {
    error("void filter instance was called for process: %p", this);
    return inframe;
  }
  proto->update(this, fps, inframe, outframe);
  return outframe;
}

