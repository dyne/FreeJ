#ifndef __Judenburg__
#define __Judenburg__

#include <filter.h>

class Judenburg: public Filter {
 private:
  Uint8 r,g,b,a;

 public:
  Judenburg();
  ~Judenburg() { _delete(); };
  
  void *process(void *buffo);
  bool init();
  void _delete();
  bool kbd_input(SDL_keysym *keysym);
  
  void *procbuf;
};

#endif
