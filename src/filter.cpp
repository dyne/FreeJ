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

$Id: $

 */

#include <config.h>
#include <layer.h>
#include <filter.h>

#include <frei0r_freej.h>

#include <jutils.h>


Parameter::Parameter(int param_type)
  : Entry() {
  switch(param_type) {
  case PARAM_BOOL:
    value = calloc(1, sizeof(double));
    break;
  case PARAM_NUMBER:
    value = calloc(1, sizeof(double));
    break;
  case PARAM_COLOR:
    value = calloc(3, sizeof(double));
    break;
  case PARAM_POSITION:
    value = calloc(2, sizeof(double));
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
    strncpy(param->name, f->param_infos[i].name, 255);

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
    error("calloc outframe failed (%i) applying filter %s",errno, name);
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

  switch(freior->param_infos[idx-1].type) {

    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:
    (*freior->f0r_set_param_value)(inst->core, new f0r_param_bool(value[0]), idx-1);
    break;

  case F0R_PARAM_DOUBLE:
    (*freior->f0r_set_param_value)(inst->core, new f0r_param_double(value[0]), idx-1);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      color->r = value[0]; color->g = value[1]; color->b = value[2];
      (*freior->f0r_set_param_value)(inst->core, color, idx-1);
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      position->x = value[0]; position->y = value[1];
      (*freior->f0r_set_param_value)(inst->core, position, idx-1);
    } break;

  default:

    error("Unrecognized parameter type %u for set_parameter_value",
	  freior->param_infos[idx].type);
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


FilterInstance::FilterInstance(Filter *fr)
  : Entry() {
  func("creating instance for filter %s",fr->name);

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

bool FilterInstance::set_parameter(int idx, void *val) {
  Parameter *param;
  param = (Parameter*)proto->parameters[idx];

  if( ! param) {
    error("parameter %s not found in filter %s", param->name, proto->name );
    return false;
  } else 
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);
  
  ////////////////////////////////////////
  if(param->type == PARAM_NUMBER) {
    
    ((double*)param->value)[0] = ((double*)val)[0];

    act("filter %s parameter %s set to: %.5f", name, param->name, ((double*)param->value)[0]);
    
    //////////////////////////////////////
  } else if(param->type == PARAM_BOOL) {
    
    ((double*)param->value)[0] = ((double*)val)[0];

    //    act("filter %s parameter %s set to: %e", name, param->name, (double*)param->value);
    //////////////////////////////////////
  } else if (param->type == PARAM_POSITION) {

    ((double*)param->value)[0] = ((double*)val)[0];
    ((double*)param->value)[1] = ((double*)val)[1];

    //////////////////////////////////////
  } else if (param->type==PARAM_COLOR) {

    ((double*)param->value)[0] = ((double*)val)[0];
    ((double*)param->value)[1] = ((double*)val)[1];
    ((double*)param->value)[2] = ((double*)val)[2];

    //////////////////////////////////////
  } else if (param->type==PARAM_STRING) {
    
    strcpy((char*)param->value, (char*)val);

  } else {
    error("attempt to set value for a parameter of unknown type: %u", param->type);
    return false;
  }
  
  // register parameter in frei0r
  proto->set_parameter_value(this, (double*)param->value, idx);

  return true;
}

    
