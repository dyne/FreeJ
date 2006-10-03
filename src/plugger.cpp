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
#include <jutils.h>
#include <plugger.h>
#include <config.h>

Plugger::Plugger() {
  char temp[256];

  _searchpath = NULL;

  sprintf(temp,"%s/.freej/plugins",getenv("HOME"));
  _addsearchdir(temp);
  _addsearchdir("/usr/lib/freej");
  _addsearchdir("/usr/local/lib/freej");
  _addsearchdir("/opt/video/lib/freej");
  for(int i=0;i<MAX_PLUGINS;i++) plugs[i] = NULL;
}

Plugger::~Plugger() {
  func("Plugger::~Plugger()");
  _delete();
}

Filter *Plugger::pick(char *name) {
  for(int c=0;c<MAX_PLUGINS;c++)
    if(plugs[c]) {
      if( strcasecmp( plugs[c]->getname() , name ) ==0 )
	return plugs[c];
    } else break;
  return(NULL);
}

/* lookup a string implementing completion:
   returns a list of chars, all possible completions
   return a NULL terminated char* array
   the return buffer is allocated in this function
   and must be freed by the caller, in any case
   also when no hits are found! */
char **Plugger::complete(char *str) {
  int c,len,found;
  char **res;
  char *tmp;
  len = strlen(str);
  found = 0;
  res = (char**)calloc(MAX_PLUGINS,sizeof(char*));
  /* first browse all filters and collect
     all those with the name starting with the
     first letter of our lookup string */
  for(c=0;c<MAX_PLUGINS;c++) {
    if(!plugs[c]) break;
    tmp = plugs[c]->getname();
    if(strncasecmp(str,tmp,len)==0) {
      res[found] = tmp; found++;
      continue;
    }
  }
  res[found] = NULL;

  return res;
}

int selector(const struct dirent *dir) {
  //  if(strstr(dir->d_name,".")) return(1);
  if(dir->d_name[0]=='.') return(0);
  if(strstr(dir->d_name,".la")) return(0);
  return(1);
}

/* searches into the lt_searchpath for valid modules */
int Plugger::refresh() {
  char *dir;
  struct dirent **filelist;
  int found;
  char *path = _getsearchpath();
  _delete();

  notice("loading available plugins");
  
  if(!path) { warning("can't find any valid plugin directory"); return(-1); }
  dir = strtok(path,":");
  do {
    found = scandir(dir,&filelist,selector,alphasort);
    if(found<0) { error("Plugger::scandir"); return(-1); };
    /* .so files found, check if they are plugins */
    while(found--) {
      char temp[256];
      snprintf(temp,255,"%s/%s",dir,filelist[found]->d_name);
      Filter *filt = new Filter;
      if(filt->open(temp)) {
	act("plugged: %s filter v%u by %s",
	    filt->getname(), filt->getversion(), filt->getauthor());
	_add_plug(filt);
      } else delete(filt);
      //      free(filelist[found]);
    }
    break;
    //    free(filelist);
  } while((dir = strtok(NULL,":")));

  return 0;
}

Filter *Plugger::operator[](const int num) {

  if(!plugs[num]) return(NULL);

  /* if the plugin is allready in use we can't instantiate it
     from the same DSO shared object */
  if(plugs[num]->inuse) return(NULL);
  /* this is handled by the keyboard class
     plugs[num]->inuse = true; */
  return(plugs[num]);
}

int Plugger::_delete() {
  func("Plugger::_delete");
  for(int c=0;c<MAX_PLUGINS;c++) if(plugs[c]) delete(plugs[c]);
  return 0;
}

bool Plugger::_add_plug(Filter *f) {
  for(int c=0;c<MAX_PLUGINS;c++)
    if(!plugs[c]) {
      plugs[c]=f;
      return(true);
    }
  return(false);
}

bool Plugger::_filecheck(const char *file) {
  bool res = true;
  FILE *f = fopen(file,"r");
  if(!f) res = false;
  fclose(f);
  return(res);
}

bool Plugger::_dircheck(const char *dir) {
  bool res = true;
  DIR *d = opendir(dir);
  if(!d) res = false;
  else closedir(d);
  return(res);
}

void Plugger::_addsearchdir(const char *dir) {
  char temp[1024];
  if(!_dircheck(dir)) return;
  if(_searchpath) {
    snprintf(temp,1024,"%s:%s",_searchpath,dir);
    jfree(_searchpath);
    _searchpath = strdup(temp);
  } else _searchpath = strdup(dir);
}
