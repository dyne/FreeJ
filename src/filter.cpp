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
#include <freeframe_freej.h>

#include <jutils.h>

#ifdef WITH_FREI0R
/// frei0r parameter callbacks
static void get_frei0r_parameter(FilterInstance *filt, Parameter *param, int idx) {
  Freior *f = filt->proto->freior;

  switch(f->param_infos[idx-1].type) {

    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:
    (*f->f0r_get_param_value)(filt->core, (f0r_param_t)param->value, idx-1);
    func("bool value is %s",(*(bool*)param->value==true) ? "true" : "false");
    break;

  case F0R_PARAM_DOUBLE:
    (*f->f0r_get_param_value)(filt->core, (f0r_param_t)param->value, idx-1);
    func("number value is %g",*(double*)param->value);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      (*f->f0r_get_param_value)(filt->core, (f0r_param_t)color, idx-1);
      ((double*)param->value)[0] = (double)color->r;
      ((double*)param->value)[1] = (double)color->g;
      ((double*)param->value)[2] = (double)color->b;
      delete color;
    } break;

  case F0R_PARAM_POSITION:
    { f0r_param_position *position = new f0r_param_position;
      (*f->f0r_get_param_value)(filt->core, (f0r_param_t)position, idx-1);
      ((double*)param->value)[0] = (double)position->x;
      ((double*)param->value)[1] = (double)position->y;
      delete position;
    } break;

  default:

    error("Unrecognized parameter type %u for get_parameter_value",
	  f->param_infos[idx].type);
  }  
}

static void set_frei0r_parameter(FilterInstance *filt, Parameter *param, int idx) {

  func("set_frei0r_param callback on %s for parameter %s at pos %u",
       filt->proto->name, param->name, idx);

  Freior *f = filt->proto->freior;
  double *val = (double*)param->value;

  switch(f->param_infos[idx-1].type) {
    
    // idx-1 because frei0r's index starts from 0
  case F0R_PARAM_BOOL:

    func("bool value is %s",(*(bool*)param->value==true) ? "true" : "false");

    (*f->f0r_set_param_value)
      (filt->core, new f0r_param_bool(*(bool*)param->value), idx-1);

    break;
    
  case F0R_PARAM_DOUBLE:
    func("number value is %g",*(double*)param->value);
    (*f->f0r_set_param_value)(filt->core, new f0r_param_double( *(double*)param->value), idx-1);
    break;

  case F0R_PARAM_COLOR:
    { f0r_param_color *color = new f0r_param_color;
      color->r = val[0];
      color->g = val[1];
      color->b = val[2];
      (*f->f0r_set_param_value)(filt->core, color, idx-1);
      delete color;
      // QUAAA: should we delete the new allocated object? -jrml
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
#endif

Filter::Filter(Type type, void *filt) 
  : Entry() {
  int i;

  initialized = false;
  active = false;
  inuse = false;

#ifdef WITH_FREI0R
  freior = NULL;
#endif
  freeframe = NULL;

  bytesize = 0;

  // critical errors:
  if(!filt) error("Filter constructor received a NULL object");
  //  if(!filt->opened) error("Filter constructor received a Freior object that is not open");

  switch(type) {

#ifdef WITH_FREI0R
  case FREIOR:
    freior = (Freior*)filt;
    (*freior->f0r_init)();

    // Get the list of params.
    freior->param_infos.resize(freior->info.num_params);
    for (i = 0; i < freior->info.num_params; ++i) {
      
      (*freior->f0r_get_param_info)(&freior->param_infos[i], i);
      
      Parameter *param = new Parameter((Parameter::Type)freior->param_infos[i].type);
      strncpy(param->name, freior->param_infos[i].name, 255);
      
      strcpy(param->description, freior->param_infos[i].explanation);
      param->filter_set_f = set_frei0r_parameter;
      param->filter_get_f = get_frei0r_parameter;
      parameters.append(param);
    }

    if(get_debug()>2)
      freior->print_info();
    
    set_name((char*)freior->info.name);
    
    break;
#endif

  case FREEFRAME:
    freeframe = (Freeframe*)filt;

    set_name((char*)freeframe->info->pluginName);

    // init freeframe filter
    if(freeframe->main(FF_INITIALISE, NULL, 0).ivalue == FF_FAIL)
      error("cannot initialise freeframe plugin %s",name);
    
    // TODO freeframe parameters
    
    if(get_debug()>2)
      freeframe->print_info();
    
    break;

  default:
    error("filter type %u not supported",type);
    return;
  }

  backend = type;

}

Filter::~Filter() {
#ifdef WITH_FREI0R
  if(freior) delete freior;
#endif
  if(freeframe) delete freeframe;
}

FilterInstance *Filter::apply(Layer *lay) {

  FilterInstance *instance;
  instance = new FilterInstance(this);

#ifdef WITH_FREI0R
  if(freior) {
    instance->core = (void*)(*freior->f0r_construct)(lay->geo.w, lay->geo.h);
  }
#endif

  if(freeframe) {
    VideoInfoStruct vidinfo;
    vidinfo.frameWidth = lay->geo.w;
    vidinfo.frameHeight = lay->geo.h;
    vidinfo.orientation = 1;
    vidinfo.bitDepth = FF_CAP_32BITVIDEO;
    instance->intcore = freeframe->main(FF_INSTANTIATE, &vidinfo, 0).ivalue;
    if(instance->intcore == FF_FAIL) {
      error("Filter %s cannot be instantiated", name);
      delete instance;
      return NULL;
    }
  }

  
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

  // here maybe keep a list of layers that possess instantiantions of this filter?

  return instance;

}

const char *Filter::description() {
  const char *ret;
#ifdef WITH_FREI0R
  if(backend==FREIOR) {
    ret = freior->info.explanation;
  }
#endif
  if(backend==FREEFRAME) {
    // TODO freeframe has no extentedinfostruct returned!?
    ret = "freeframe VFX";
  }
  return ret;

}

int Filter::get_parameter_type(int i) {
  int ret;
#ifdef WITH_FREI0R
  if(backend==FREIOR) {
    ret =  freior->param_infos[i].type;
  }
#endif
  if(backend==FREEFRAME) {
    // TODO freeframe
  }
  return ret;

}

char *Filter::get_parameter_description(int i) {
  char *ret;
#ifdef WITH_FREI0R
  if(backend==FREIOR) {
    ret = (char*)freior->param_infos[i].explanation;
  }
#endif
  if(backend==FREEFRAME) {
    // TODO freeframe
  }
  return ret;
}

void Filter::destruct(FilterInstance *inst) {

#ifdef WITH_FREI0R
  if(backend==FREIOR) {

    if(inst->core) {
      (*freior->f0r_destruct)((f0r_instance_t*)inst->core);
      inst->core = NULL;
    }

  }
#endif
  if(backend==FREEFRAME) {

    freeframe->main(FF_DEINSTANTIATE, NULL, inst->intcore);

  }
}

void Filter::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {

#ifdef WITH_FREI0R
  if(backend==FREIOR) {

    (*freior->f0r_update)((f0r_instance_t*)inst->core, time, inframe, outframe);

  }
#endif
  if(backend==FREEFRAME) {

    jmemcpy(outframe,inframe,bytesize);

    freeframe->main(FF_PROCESSFRAME, (void*)outframe, inst->intcore);

  }

}


FilterInstance::FilterInstance(Filter *fr)
  : Entry() {
  func("creating instance for filter %s",fr->name);

  proto = fr;
  
  core = NULL;
  intcore = 0;
  outframe = NULL;
  
  active = true;

  set_name(proto->name);
}

FilterInstance::~FilterInstance() {
  func("~FilterInstance");

  if(proto)
    proto->destruct(this);

  if(outframe) free(outframe);

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
    error("parameter [%u]%s not found in filter %s", idx, param->name, proto->name );
    return false;
  } else 
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);

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
  } else 
    func("parameter %s found in filter %s at position %u",
	 param->name, proto->name, idx);

  if(!param->filter_get_f) {
    error("no filter callback function registered in this parameter");
    return false;
  }

  (*param->filter_get_f)(this, param, idx);

  return true;
}

    
    
