#ifndef __DSO_PLUGIN_H__
#define __DSO_PLUGIN_H__

#include <freej.h>
#include <linklist.h>
#include <jutils.h>

class Plugin {
  typedef int (t_init)(ScreenGeometry*);
  typedef int (t_clean)(void);
  typedef void* (t_process)(void*);
  typedef int (t_kbdin)(SDL_keysym*);
 public:
  Plugin();
  ~Plugin();
  
  bool open(const char *path);
  void *operator[](const char *);

  void init(ScreenGeometry *sg) { 
    func("plugin %s::init",getname());
    (*__init)(sg); };

  void clean() {
    func("plugin %s::clean", getname());
    (*__clean)(); };

  void *process(void *buffo) { return (*__process)(buffo); };

  int kbd_input(SDL_keysym *keysym) { return (*__kbd_input)(keysym); };

  char *getname() { return _name; };
  char *getauthor() { return _author; };
  char *getinfo() { return _info; };
  int getversion() { return _version; };
  int getbpp(int bpp) { return ((bpp/8)<=_bpp); };

private:
  void _delete();
  
  t_init *__init;
  t_clean *__clean;
  t_process *__process;
  t_kbdin *__kbd_input;

  void *_handle;

  /* plugin informations */
  char *_name;
  char *_author;
  char *_info;
  int _version;
  int _bpp;
  char *_path;
};

#endif
