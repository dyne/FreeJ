#ifndef __Bitcolor__
#define __Bitcolor__

#include <filter.h>

class Bitcolor: public Filter {
 private:
  unsigned short ccodio;
  
 public:
  Bitcolor();
  ~Bitcolor() { _delete(); };

  void *process(void *buffo);
  bool init();
  void _delete();

  bool kbd_input(SDL_keysym *keysym);
  void inc_bitmask() { if(ccodio<8) ccodio++; };
  void dec_bitmask() { if(ccodio>0) ccodio--; };
  void *procbuf;
};

#endif
