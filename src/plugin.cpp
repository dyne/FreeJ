#include <string.h>
#include <dlfcn.h>

#include <plugin.h>

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
  if(point==NULL)
    warning("Plugin[%s] %s",sym,dlerror());
  return(point);
}

static inline int dummy_kbd_input(SDL_keysym *keysym) { return(0); }

bool Plugin::open(const char *path) {
  getch *getstr = NULL;
  getint *getver = NULL;

  _handle = dlopen(path,RTLD_NOW);
  if(!_handle) {
    warning("can't open plugin %s: %s",path,dlerror());
    return(false);
  }

  func("Plugin opened: %s handle[%p]",path,_handle);
  
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
    
  getstr = (getch*) (*this)["getname"];
  if(getstr) _name = (*getstr)();

  getstr = (getch*) (*this)["getauthor"];
  if(getstr) _author = (*getstr)();

  getstr = (getch*) (*this)["getinfo"];
  if(getstr) _info = (*getstr)();

  getver = (getint*) (*this)["getversion"];
  if(getver) _version = (*getver)();
  else _version = 0;

  /*
  getver = (getint*) (*this)["getbpp"];
  if(getver) _bpp = (*getver)();
  else _bpp = 0;
  */

  _path = strdup(path);    
  return(true);
}

void Plugin::_delete() {
  if(__clean) clean();
  if(_handle) dlclose(_handle);

  if(_path) jfree(_path);
}

Plugin::~Plugin(void) {
  _delete();
}
