/*  FreeJ - FreeFrame videoFX wrapper
 *
 *  (c) Copyright 2008 Denis Roio <jaromil@dyne.org>
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

#include <cstdlib>
#include <string>

#include <config.h>

#include <freeframe_freej.h>

#include <jutils.h>

#ifdef HAVE_DARWIN
#include <CoreFoundation/CoreFoundation.h>
#endif

Freeframe::Freeframe() 
  : Entry() { 

  handle = NULL;
  opened = false;

}

Freeframe::~Freeframe() {

  if(handle) dlclose(handle);

}

int Freeframe::open(char *file) {
  plugMainType *plugmain;

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
      plugmain = (plugMainUnion (*)(DWORD, void*, DWORD))CFBundleGetFunctionPointerForName(bundle, CFSTR("plugMain"));
      CFRelease(filestring);
      CFRelease(filepath);
#endif
  } else {
      handle = dlopen(file, RTLD_NOW);
      if(!handle) {
        warning("can't dlopen plugin: %s", file);
        return 0;
      }

      // try the freeframe symbol
      plugmain = (plugMainType *) dlsym(handle, "plugMain");
      if(!plugmain) {
        func("%s not a valid freeframe plugin: %s", file, dlerror());
        // don't forget to close
        dlclose(handle);
        handle = NULL;
        return 0;
      }

  }
  
  /// WARNING:  if  compiled  without  -freg-struct-return  this  will
  /// return an invalid address ...
  PlugInfoStruct *pis = (plugmain(FF_GETINFO,NULL, 0)).PISvalue;

  //  func("freeframe plugin: %s",pis->pluginName);
  // ... and here will segfault
  if ((plugmain(FF_GETPLUGINCAPS,
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
  //   (plugmain(FF_INITIALISE, NULL, 0)).ivalue {

  main = plugmain;
  info = pis;
  //  extinfo = plugmain(FF_GETEXTENDEDINFO, NULL, 0)

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
  act("Parameters [%i total]", main(FF_GETNUMPARAMETERS, NULL, 0).ivalue);
}

