/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#include <config.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#ifdef WITH_FREI0R
#include <frei0r.h>
#endif

#include <plugger.h>
#include <context.h>
#include <jutils.h>
#include <frei0r_freej.h>
#include <freeframe_freej.h>

#ifdef HAVE_DARWIN
#include <Carbon/Carbon.h>
#endif

#ifdef WITH_COCOA
#include <CVFilter.h>
#endif

Plugger::Plugger() {
  char temp[256];

  _searchpath = NULL;

  addsearchdir(PACKAGE_LIB_DIR);

  sprintf(temp,"%s/.freej/plugins",getenv("HOME"));
  addsearchdir(temp);

  addsearchdir("/usr/lib/FreeFrame");
  addsearchdir("/usr/local/lib/FreeFrame");
#ifdef WITH_FREI0R
  addsearchdir("/usr/lib/frei0r-1");
  addsearchdir("/usr/lib64/frei0r-1");
  addsearchdir("/usr/local/lib/frei0r-1");
  addsearchdir("/opt/local/lib/frei0r-1");
#endif

//   addsearchdir("/usr/lib/freej");
//   addsearchdir("/usr/local/lib/freej");
//   addsearchdir("/opt/video/lib/freej");
#ifdef HAVE_DARWIN
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    // 238 == 256 - strlen("/Contents/Plugins") - 1
    FSRefMakePath(&location, (UInt8 *)temp, 238);
    strcat(temp, "/Contents/Plugins");
    addsearchdir(temp);
    sprintf(temp, "%s/Library/Application Support/FreeJ", getenv("HOME"));
    addsearchdir(temp);
    sprintf(temp, "%s/Library/Application Support/FreeFrame", getenv("HOME"));
    addsearchdir(temp);
    addsearchdir("/Library/Application Support/FreeJ");
    addsearchdir("/Library/Application Support/FreeFrame");
#endif

}

Plugger::~Plugger() {

  func("Plugger::~Plugger()");
  if(_searchpath) free(_searchpath);

}

int selector(const struct dirent *dir)
{
  if(strstr(dir->d_name,".so")) return(1);
  else if(strstr(dir->d_name,".frf")) return(1);
#ifdef HAVE_DARWIN
  else if(strstr(dir->d_name,".dylib")) return(1);
#endif
  return(0);
}


int Plugger::refresh(Context *env) {

  char *dir;
  struct dirent **filelist;
  int found;
  char *path = _getsearchpath();

#ifdef WITH_COCOA
    CVFilter::listFilters(env->filters);
#endif
  if(path) {

      notice("serching available plugins in %s", path);

      dir = strtok(path,":");

      // scan for all available effects
      do {
        func("scanning %s",dir);

          found = scandir(dir,&filelist,selector,alphasort);
          if(found<0) { error("Plugger::scandir"); return(-1); };
          /* .so files found, check if they are plugins */
          
          
          while(found--) {
        
        char temp[256];
        
        snprintf(temp,255,"%s/%s",dir,filelist[found]->d_name);
        free(filelist[found]);

#ifdef WITH_FREI0R
        {
              Freior *fr = (Freior *)Factory<Filter>::new_instance("Frei0rFilter");
          if( !fr || !fr->open(temp) ) {
            delete fr;
          } else { // freior effect found
            // check what kind of plugin is and place it
            if(fr->info.plugin_type == F0R_PLUGIN_TYPE_FILTER) {
                  env->filters.append(fr);
              
              func("found frei0r filter: %s (%p)", fr->name, fr);
              continue;

            } else if(fr->info.plugin_type == F0R_PLUGIN_TYPE_SOURCE) {	      
              env->generators.append(fr);
              func("found frei0r generator: %s (%p)", fr->name, fr);
              continue;

            } else if(fr->info.plugin_type == F0R_PLUGIN_TYPE_MIXER2) {
              func("frei0r plugin of type MIXER2 not supported (yet)",
               fr->info.plugin_type);
              delete fr;
              continue;
            } else if(fr->info.plugin_type == F0R_PLUGIN_TYPE_MIXER3) {
              func("frei0r plugin of type MIXER3 not supported (yet)",
               fr->info.plugin_type);
              delete fr;
              continue;
            }
          }
        }
#endif
#ifdef WITH_FREEFRAME
        {
              Freeframe *fr = (Freeframe *)Factory<Filter>::new_instance("FreeframeFilter");
          if( ! fr->open(temp) ) {
            delete fr;
          } else { // freeframe effect found
            // check what kind of plugin is and place it
            if(fr->info->pluginType == FF_EFFECT) {
              
              env->filters.append(fr);
              
              func("found freeframe filter: %s (%p)",
               fr->info->pluginName, fr);
              continue;

            } else if(fr->info->pluginType == FF_SOURCE) {
              
              env->generators.append(fr);
              
              func("found freeframe generator: %s (%p)",
               fr->info->pluginName, fr);
              continue;

            }
          }
        }
#endif
        if(found<0)
              break;
          }

         free(filelist);
      } while((dir = strtok(NULL,":")));
  } else {
      warning("can't find any valid plugger directory");
      return(-1);
  }
  act("filters found: %u", env->filters.len());
  act("generators found: %u", env->generators.len());

  return 0;
}


void Plugger::addsearchdir(const char *dir) {
  char temp[1024];
  if(!dircheck(dir))
      return;
  if(_searchpath) {
    snprintf(temp,1024,"%s:%s",_searchpath,dir);
    free(_searchpath);
    _searchpath = strdup(temp);
  } else {
      _searchpath = strdup(dir);
  }
}
