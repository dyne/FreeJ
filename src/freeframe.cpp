/*  FreeJ - FreeFrame videoFX wrapper
 *
 *  Copyright (C) 2008-2010 Denis Roio <jaromil@dyne.org>
 *  Copyright (C) 2010    Andrea Guzzo <xant@dyne.org>
 *
 *  studied on a  Pure Data Packet module
 *    by Tom Schouten <pdp@zzz.kotnet.org>
 *  to add support for Pete Warden's free video plugins
 *
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


#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h> // for snprintf()

#include <cstdlib>
#include <string>

#include <config.h>

#include <freeframe_freej.h>

#include <jutils.h>
#include <layer.h>

#ifdef HAVE_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

FACTORY_REGISTER_INSTANTIATOR(Filter, Freeframe, FreeframeFilter, core);

Freeframe::Freeframe() 
  : Filter() 
{ 

  handle = NULL;
  opened = false;


  set_name((char*)info->pluginName);

  // init freeframe filter
  if(plugmain(FF_INITIALISE, NULL, 0).ivalue == FF_FAIL)
    error("cannot initialise freeframe plugin %s",name);

  // TODO freeframe parameters

  if(get_debug()>2)
    print_info();

}

Freeframe::~Freeframe() {

  if(handle)
      dlclose(handle);

}

int Freeframe::open(char *file) {
  plugMainType *plgMain;

  if(opened) {
    error("Freeframe object %p has already opened file %s",this, filename);
    return 0;
  }

  // clear up the errors if there were any
  dlerror();

  // Create dynamic handle to the library file.
  if(strstr(file, ".frf")) {
#ifdef HAVE_DARWIN
      CFStringRef filestring = CFStringCreateWithCString(NULL, file, kCFStringEncodingUTF8);
      CFURLRef filepath = CFURLCreateWithFileSystemPath(NULL, filestring, kCFURLPOSIXPathStyle, 1);;
      CFBundleRef bundle = CFBundleCreate(NULL, filepath);
      plgMain = (plugMainUnion (*)(DWORD, void*, DWORD))CFBundleGetFunctionPointerForName(bundle, CFSTR("plugMain"));
      CFRelease(filestring);
      CFRelease(filepath);
#endif
  } else {
      dlerror(); //clear up previous errors 
#if 0
      if (!dlopen_preflight(file)) {
        warning("plugin '%s' failed: %s", file, dlerror());
        return 0;
      }
#endif
      handle = dlopen(file, RTLD_NOW);
      if(!handle) {
        warning("can't dlopen plugin: %s", file);
        return 0;
      }

      // try the freeframe symbol
      plgMain = (plugMainType *) dlsym(handle, "plugMain");
      if(!plgMain) {
        func("%s not a valid freeframe plugin: %s", file, dlerror());
        // don't forget to close
        dlclose(handle);
        handle = NULL;
        return 0;
      }

  }
  
  /// WARNING:  if  compiled  without  -freg-struct-return  this  will
  /// return an invalid address ...
  PlugInfoStruct *pis = (plgMain(FF_GETINFO,NULL, 0)).PISvalue;

  //  func("freeframe plugin: %s",pis->pluginName);
  // ... and here will segfault
  if ((plgMain(FF_GETPLUGINCAPS,
		(LPVOID) FF_CAP_32BITVIDEO, 0)).ivalue != FF_TRUE) {
    func("plugin %s: no 32 bit support", file);
    dlclose(handle);
    handle = NULL;
    return 0;
  }
 
  if (pis->APIMajorVersion < 1) {
    error("plugin %s: old api version", file);
    dlclose(handle);
    handle = NULL;
    return 0;
  }

  // init is called by Filter class
  //   (plgMain(FF_INITIALISE, NULL, 0)).ivalue {

  info = pis;
  //  extinfo = plgMain(FF_GETEXTENDEDINFO, NULL, 0)
  plugmain = plgMain;
  opened = true;
  snprintf(filename,255,"%s",file);

  return 1;

}

void Freeframe::print_info() {
  notice("Name             : %s", info->pluginName);
  switch(info->pluginType) {    
  case FF_EFFECT: act("Type             : Filter"); break;
  case FF_SOURCE: act("Type             : Source"); break;
  default: error("Unrecognized plugin type");
  }
  act("Parameters [%i total]", plugmain(FF_GETNUMPARAMETERS, NULL, 0).ivalue);
}

bool Freeframe::apply(Layer *lay, FilterInstance *instance) 
{
    VideoInfoStruct vidinfo;
    vidinfo.frameWidth = lay->geo.w;
    vidinfo.frameHeight = lay->geo.h;
    vidinfo.orientation = 1;
    vidinfo.bitDepth = FF_CAP_32BITVIDEO;
    instance->intcore = plugmain(FF_INSTANTIATE, &vidinfo, 0).ivalue;
    if(instance->intcore == FF_FAIL) {
        error("Filter %s cannot be instantiated", name);
        return false;
    }
    return Filter::apply(lay, instance);
}

void Freeframe::destruct(FilterInstance *inst) {
    plugmain(FF_DEINSTANTIATE, NULL, inst->intcore);
}

void Freeframe::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {
    Filter::update(inst, time, inframe, outframe);
    jmemcpy(outframe,inframe,bytesize);
    plugmain(FF_PROCESSFRAME, (void*)outframe, inst->intcore);
}

const char *Freeframe::description()
{
    // TODO freeframe has no extentedinfostruct returned!?
    return "freeframe VFX";
}

const char *Freeframe::author()
{
    // TODO freeframe has no extentedinfostruct returned!?
    return "freeframe authors";
}

int Freeframe::get_parameter_type(int i)
{
    // TODO
    return -1;
}

char *Freeframe::get_parameter_description(int i)
{
    // TODO
    return (char *)"Unknown";
}

int Freeframe::type()
{
    return Filter::FREEFRAME;
}
