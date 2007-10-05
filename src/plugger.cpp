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
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include <frei0r.h>

#include <config.h>


#include <plugger.h>
#include <context.h>
#include <jutils.h>
#include <frei0r_freej.h>



Plugger::Plugger() {
  char temp[256];

  _searchpath = NULL;

  sprintf(temp,"%s/.freej/plugins",getenv("HOME"));
  addsearchdir(temp);
  addsearchdir("/usr/lib/freej");
  addsearchdir("/usr/local/lib/freej");
  addsearchdir("/opt/video/lib/freej");

}

Plugger::~Plugger() {

  func("Plugger::~Plugger()");

}

int selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".so")) return(1);
  return(0);
}


int Plugger::refresh(Context *env) {

  char *dir;
  struct dirent **filelist;
  int found;
  char *path = _getsearchpath();

  Freior *fr;

  notice("serching available plugins");
 
  if(!path) { warning("can't find any valid plugger directory"); return(-1); }

  dir = strtok(path,":");

  do {

      found = scandir(dir,&filelist,selector,alphasort);
      if(found<0) { error("Plugger::scandir"); return(-1); };
      /* .so files found, check if they are plugins */
      
      
      while(found--) {
	
	char temp[256];
	
	snprintf(temp,255,"%s/%s",dir,filelist[found]->d_name);

	fr = new Freior();
	if( ! fr->open(temp) ) {
	  delete fr; continue;
	}

	func("found frei0r plugin: %s (%p)", temp, fr);

	// check what kind of plugin is and place it
	if(fr->info.plugin_type == F0R_PLUGIN_TYPE_FILTER) {
	  //	case F0R_PLUGIN_TYPE_FILTER:
	  Filter *filt = new Filter(fr);
	  env->filters.append(filt);
	} else {
	  func("frei0r plugin of type %i not supported (yet)",
	       fr->info.plugin_type);
	}

	if(found<0) break;
      }
      
  } while((dir = strtok(NULL,":")));

  act("filters found: %u", env->filters.len());
  
  return 0;
}


void Plugger::addsearchdir(char *dir) {
  char temp[1024];
  if(!dircheck(dir)) return;
  if(_searchpath) {
    snprintf(temp,1024,"%s:%s",_searchpath,dir);
    jfree(_searchpath);
    _searchpath = strdup(temp);
  } else _searchpath = strdup(dir);
}
