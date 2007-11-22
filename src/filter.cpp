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

  layer_func = NULL;
  filter_func = NULL;

}

Parameter::~Parameter() {
  free(value);
}

bool Parameter::set(void *val) {
  ////////////////////////////////////////
  if(type == PARAM_NUMBER) {
    
    ((double*)value)[0] = ((double*)val)[0];
    
    //////////////////////////////////////
  } else if(type == PARAM_BOOL) {
    
    ((double*)value)[0] = ((double*)val)[0];
    
    //    act("filter %s parameter %s set to: %e", name, param->name, (double*)value);
    //////////////////////////////////////
  } else if (type == PARAM_POSITION) {
    
    ((double*)value)[0] = ((double*)val)[0];
    ((double*)value)[1] = ((double*)val)[1];
    
    //////////////////////////////////////
  } else if (type==PARAM_COLOR) {

    ((double*)value)[0] = ((double*)val)[0];
    ((double*)value)[1] = ((double*)val)[1];
    ((double*)value)[2] = ((double*)val)[2];
    
    //////////////////////////////////////
  } else if (type==PARAM_STRING) {
    
    strcpy((char*)value, (char*)val);

  } else {
    error("attempt to set value for a parameter of unknown type: %u", type);
    return false;
  }
  
  return true;
}

bool Parameter::parse(char *p) {
  // parse the strings into value
  double *val;

  if(type == PARAM_NUMBER) {
    
    if( sscanf(p, "%le", (double*)value) < 1 ) {
      error("error parsing value [%s] for parameter %s", p, name);
      return false;
    }
    
    //////////////////////////////////////
  } else if(type == PARAM_BOOL) {
    
    if( sscanf(p, "%le", (double*)value) < 1 ) {
      error("error parsing value [%s] for parameter %s", p, name);
      return false;
    }
    
  } else if(type == PARAM_POSITION) {
    
    val = (double*)value;
    if( sscanf(p, "%le %le", &val[0], &val[1]) < 1 ) {
      error("error parsing position [%s] for parameter %s", p, name);
      return false;
    }
    
  } else if(type == PARAM_COLOR) {
    
    val = (double*)value;
    if( sscanf(p, "%le %le %le", &val[0], &val[1], &val[2]) < 1 ) {
      error("error parsing position [%s] for parameter %s", p, name);
      return false;
    }
    
  } else {
    error("attempt to set value for a parameter of unknown type: %u", type);
    return false;
  }
  
  return true;

}

/// set_parameter callback
static void set_frei0r_parameter(FilterInstance *filt, Parameter *param, int idx) {

  Freior *f = filt->proto->freior;
  double *val = (double*)param->value;

  switch(f->param_infos[idx-1].type) {
    
    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:
    (*f->f0r_set_param_value)(filt->core, new f0r_param_bool(val[0]), idx-1);
    break;
    
  case F0R_PARAM_DOUBLE:
    (*f->f0r_set_param_value)(filt->core, new f0r_param_double(val[0]), idx-1);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      color->r = val[0];
      color->g = val[1];
      color->b = val[2];
      (*f->f0r_set_param_value)(filt->core, color, idx-1);
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      position->x = val[0];
      position->y = val[1];
      (*f->f0r_set_param_value)(filt->core, position, idx-1);
    } break;

  default:

    error("Unrecognized parameter type %u for set_parameter_value",
	  f->param_infos[idx].type);

  }

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
    param->filter_func = set_frei0r_parameter;
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
    proto->destruct(this);
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

bool FilterInstance::set_parameter(int idx) {
  Parameter *param;
  param = (Parameter*)proto->parameters[idx];

  if( ! param) {
    error("parameter %s not found in filter %s", param->name, proto->name );
    return false;
  } else 
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);

  if(!param->filter_func) {
    error("no filter callback function registered in this parameter");
    return false;
  }

  (*param->filter_func)(this, param, idx);

  return true;
}

    
