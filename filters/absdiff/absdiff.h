#ifndef __Absdiff__
#define __Absdiff__

#include <filter.h>

class Absdiff : public Filter {
 private:
  int threshold_value;
  void *lastimage;

 public:
  Absdiff();
  ~Absdiff() { _delete(); };

  void *process(void *buffo);
  bool init();
  void _delete();

  void *procbuf;

};

#endif
