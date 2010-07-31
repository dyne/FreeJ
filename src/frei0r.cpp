/*  FreeJ - Frei0r wrapper
 *
 *  Copyright (C) 2007-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo   <xant@xant.net>
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

#ifdef WITH_FREI0R

#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h> // for snprintf()

#include <cstdlib>
#include <string>

#include <frei0r_freej.h>
#include <layer.h>
#include <jutils.h>

FACTORY_REGISTER_INSTANTIATOR(Filter, Freior, Frei0rFilter, core);

/// frei0r parameter callbacks
static void get_frei0r_parameter(FilterInstance *filt, Parameter *param, int idx) {
    Freior *f = (Freior *)filt->proto;
    
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
    
    Freior *f = (Freior *)filt->proto;
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
// end of parameter callbacks

Freior::Freior() 
  : Filter() 
{ 
  handle = NULL;
  opened = false;
           
  set_name((char *)"Unknown");
}

Freior::~Freior() {

  if(handle)
    dlclose(handle);

}

int Freior::open(char *file) {
  void *sym;
#ifdef HAVE_FREEBSD
  const char *err;
#else
  char *err;
#endif

  if(opened) {
    error("Freior object %p has already opened file %s",this, filename);
    return 0;
  }

  // clear up the errors if there were any
  dlerror();

#if 0
  if (!dlopen_preflight(file)) {
    warning("plugin '%s' failed: %s", file, dlerror());
    return 0;
  }
#endif
  // Create dynamic handle to the library file.
  handle = dlopen(file, RTLD_LAZY);
  if(!handle) {
    warning("can't dlopen plugin: %s", file);
    return 0;
  }

  // try the frei0r symbol
  sym = dlsym(handle, "f0r_init");

  // return if error
  if( dlerror() ) {
    //    func("not a valid frei0r plugin: %s", file);
    // don't forget to close
    dlclose(handle);
    handle = NULL;
    return 0;
  }

  // Get interface function pointers
  *(void**) (&f0r_init) = sym; // dlsym(handle, "f0r_init"); 
  *(void**) (&f0r_get_plugin_info) = dlsym(handle, "f0r_get_plugin_info");
  err = dlerror();
  if (err)
    warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_get_param_info) = dlsym(handle, "f0r_get_param_info");
  err = dlerror();
  if (err)
    warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_construct) = dlsym(handle, "f0r_construct");
  err = dlerror();
  if (err)
    warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_destruct) = dlsym(handle, "f0r_destruct");
  err = dlerror();
  if (err)
    warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_set_param_value) = dlsym(handle, "f0r_set_param_value");
  err = dlerror();
    if (err)
      warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_get_param_value) = dlsym(handle, "f0r_get_param_value");
  err = dlerror();
    if (err)
      warning("%s in frei0r plugin %s", err, file);
  *(void**) (&f0r_update) = dlsym(handle, "f0r_update");
  // XXX - don't output an error since f0r_update() is exported only by generators
  //       and it won't be found when opening filters
  //err = dlerror(); if (err) warning("%s in frei0r plugin %s", err, file);
  
  // get the info on the plugin
  (*f0r_get_plugin_info)(&info);

  opened = true;
  snprintf(filename,255,"%s",file);

  f0r_init();
  
  set_name((char*)info.name);
 
  // Get the list of params.
  param_infos.resize(info.num_params);
  for (int i = 0; i < info.num_params; ++i) {
      
      (f0r_get_param_info)(&param_infos[i], i);
      
      Parameter *param = new Parameter((Parameter::Type)param_infos[i].type);
      snprintf(param->name, 255, "%s", param_infos[i].name);
      func("registering parameter %s for filter %s\n", param->name, info.name);
      
      snprintf(param->description, 512, "%s", param_infos[i].explanation);
      param->filter_set_f = set_frei0r_parameter;
      param->filter_get_f = get_frei0r_parameter;
      parameters.append(param);
  }
  
  if(get_debug()>2)
      print_info();
  
  return 1;

}

void Freior::print_info() {
  notice("Name             : %s", info.name);
  act("%s",info.explanation);
  switch(info.plugin_type) {
  case F0R_PLUGIN_TYPE_FILTER: act("Type             : Filter"); break;
  case F0R_PLUGIN_TYPE_SOURCE: act("Type             : Source"); break;
  case F0R_PLUGIN_TYPE_MIXER2: act("Type             : Mixer2"); break;
  case F0R_PLUGIN_TYPE_MIXER3: act("Type             : Mixer3"); break;
  default: error("Unrecognized plugin type");
  }
  act("Author           : %s", info.author);
  act("Parameters [%i total]",info.num_params);
  for (int i=0; i<info.num_params; ++i) {
    char tmp[256];
    snprintf(tmp,255,"  [%i] %s ",i, param_infos[i].name);
    switch (param_infos[i].type) {
    case F0R_PARAM_BOOL:     act("%s (bool) %s",tmp, param_infos[i].explanation);     break;
    case F0R_PARAM_DOUBLE:   act("%s (double) %s",tmp, param_infos[i].explanation);    break;
    case F0R_PARAM_COLOR:    act("%s (color) %s",tmp, param_infos[i].explanation);    break;
    case F0R_PARAM_POSITION: act("%s (position) %s",tmp, param_infos[i].explanation); break;
    case F0R_PARAM_STRING:   act("%s (string) %s",tmp, param_infos[i].explanation); break;      
    default: error("%s Unrecognized info type.",tmp );
    }
  }
}

bool Freior::apply(Layer *lay, FilterInstance *instance) 
{
    instance->core = (*f0r_construct)(lay->geo.w, lay->geo.h);
    if (!instance->core)
        return false;
    return Filter::apply(lay, instance);
}

void Freior::destruct(FilterInstance *inst)
{
    if(inst->core) {
        f0r_destruct((f0r_instance_t*)inst->core);
        inst->core = NULL;
    }
}

void Freior::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {
    Filter::update(inst, time, inframe, outframe);
    f0r_update((f0r_instance_t*)inst->core, time, inframe, outframe);
}

const char *Freior::description()
{
    return info.explanation;
}

const char *Freior::author()
{
    return info.author;
}

int Freior::get_parameter_type(int i)
{
    return param_infos[i].type;
}

char *Freior::get_parameter_description(int i)
{
    return (char*)param_infos[i].explanation;
}

int Freior::type()
{
    return Filter::FREIOR;
}

#endif // WITH_FREI0R

