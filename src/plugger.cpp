#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <jutils.h>
#include <plugger.h>

Plugger::Plugger() {
  char *home;
  char temp[256];

  _searchpath = NULL;

  home = getenv("HOME");
  sprintf(temp,"%s/.freej/plugins",home);
  _addsearchdir(temp);
  _addsearchdir("/usr/local/lib/freej");
  _addsearchdir("/usr/lib/freej");
  for(int i=0;i<MAX_PLUGINS;i++) plugs[i] = NULL;
}

Plugger::~Plugger() {
  func("Plugger::~Plugger()");
  _delete();
}

int selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".so")) return(1);
  return(0);
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
    }
  } while((dir = strtok(NULL,":")));
  free(filelist);
  return 0;
}

Filter *Plugger::operator[](const int num) {
  if(!plugs[num]) return(NULL);
  if(plugs[num]->inuse) return(NULL);
  plugs[num]->inuse = true; return(plugs[num]);
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
  closedir(d);
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