#ifndef __BLITTER_H__
#define __BLITTER_H__

#include <SDL.h>
#include <linklist.h>
#include <screen.h>

typedef void (blit_f)(void *src, void *dst, int len);

class Layer;

class Blit: public Entry {
 public:

  Blit();
  ~Blit();

  char desc[512];   ///< long description
  int param;         ///< parameter value
  short kernel[256]; ///< convolution kernel
  
  blit_f *fun;
};



class Blitter {
 public:
  Blitter();
  ~Blitter();


  bool init(Layer *lay); ///< initialize the blitter


  /* ==== BLITS */
  void blit();
  bool set_blit(char *name); ///< set the active blit
  bool set_value(int val); ///< set the blit parameter
  bool set_kernel(short *krn); /// set the convolution kernel
  Linklist blitlist; ///< list of available blits


  /* ==== geometrical transformations */
  double x_scale;    ///< zoom factor on x axis
  double y_scale;    ///< zoom factor on y axis
  double rotation;   ///< rotation factor
  
  
  /* ==== CROP */
  /** @param view if NULL, default ViewPort is used */
  void crop(ViewPort *view);
  ///< crop to fit in the ViewPort
  
  Layer *layer; ///< the layer on which is applied the blitter
  
 private:
  blit_f *blit_function;
  uint32_t blit_x;
  uint32_t blit_y;
  uint32_t blit_width;
  uint32_t blit_height;
  uint32_t blit_pitch;
  uint32_t blit_offset;
  uint32_t *blit_coords;
  SDL_Rect rect;
  /* small vars used in blits? */
  int chan, c, cc;
  uint32_t *scr, *pscr, *off, *poff;
  uint8_t *alpha;
  uint32_t rmask,gmask,bmask,amask;
};

#endif
