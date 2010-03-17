#include <string.h>
#include <dlfcn.h>

#include <plugin.h>
#include <config.h>
#include <stdio.h>

typedef char* (getch)(void);
typedef int (getint)(void);

Plugin::Plugin() {
  _name = _info = _author = _path = NULL;
  _handle = NULL;
  _version = 0;
  __init = NULL;
  __clean = NULL;
  __process = NULL;
}

void *Plugin::operator[](const char *sym) {
  if(!_handle) return NULL;
  void *point;  
  point = dlsym(_handle, sym);
#ifdef HAVE_DARWIN
/* darwin prepends an _ before plugin symbol names */
  if(!point) {
    char tmp[256];
    snprintf(tmp,256,"_%s",sym);
    point = dlsym(_handle, tmp);
  }
#endif
  if(!point)
    warning("Plugin::%s[%s] %s",_name,sym,dlerror());
  return(point);
}

static inline int dummy_kbd_input(int key) { return(0); }

bool Plugin::open(const char *path) {
  getch *getstr = NULL;
  getint *getver = NULL;

  dlerror(); //clear up previous errors 
#if 0
  if (!dlopen_preflight(path)) {
    warning("plugin '%s' failed: %s", path, dlerror());
    return false;
  }
#endif
  _handle = dlopen(path,RTLD_LAZY);
  if(!_handle) {
    warning("can't open plugin: %s",dlerror());
    return(false);
  }

  getstr = (getch*) (*this)["getname"];
  if(getstr) _name = (*getstr)();

  getstr = (getch*) (*this)["getauthor"];
  if(getstr) _author = (*getstr)();

  getstr = (getch*) (*this)["getinfo"];
  if(getstr) _info = (*getstr)();

  getver = (getint*) (*this)["getversion"];
  if(getver) _version = (*getver)();
  else _version = 0;

  func("Opened plugin %s from %s with handle %p",
       _name,path,_handle);
  
  __init = (t_init*)(*this)["init"];
  __clean = (t_clean*)(*this)["clean"];
  __process = (t_process*)(*this)["process"];
  __kbd_input = (t_kbdin*)(*this)["kbd_input"];
  if(!__kbd_input) __kbd_input = dummy_kbd_input;
  if(!__init||!__clean||!__process) {
    warning("invalid plugin %s",path);
    dlclose(_handle);
    return(false);
  }

  _path = strdup(path);    
  return(true);
}

void Plugin::_delete() {
  //  if(__clean) clean();
  if(_handle) dlclose(_handle);

  if(_path) free(_path);
}

Plugin::~Plugin(void) {
  _delete();
}
