#ifndef __Rotozoom__
#define __Rotozoom__

#include <filter.h>

class Rotozoom: public Filter {
 private:
  void init_tables();
  int theta;
  int h_cos [360];
  int h_sin [360];
  int xi [360];
  int yi [360];
  int xj [360];
  int yj [360];
 public:
  Rotozoom();
  ~Rotozoom() { _delete(); };

  void *process(void *buffo);
  bool init();
  void _delete();
  
  void *procbuf;
};

#endif
