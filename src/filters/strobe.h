#ifndef __StrobeFilter__
#define __StrobeFilter__

#include <filter.h>

class Strobe: public Filter {
 private:
  int frame_count;
  int delay;
  
 public:
  Strobe();
  ~Strobe() { _delete(); };

  void *process(void *buffo);
  bool init();
  void _delete();
  
  /* shows one frame every num frames */
  void set_delay(int num);

  void *storebuf;
};

#endif
