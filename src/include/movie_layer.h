#ifndef __MOVIE_LAYER__
#define __MOVIE_LAYER__

extern "C" {
#include <mlt/framework/mlt.h>
}

#include <layer.h>


class MovieLayer: public Layer {

 public:
  MovieLayer();
  ~MovieLayer();

  bool init(Context *freej);
  
  bool open(char *file);
  void *feed();
  void close();

  bool keypress(int k);

  
  mlt_producer real_producer;
  mlt_properties properties;
  mlt_service service;

 private:
  uint8_t *buffer;
  

};

#endif
