#ifndef __FREEJ_H__
#define __FREEJ_H__

#include <SDL/SDL.h>

typedef struct {
  Sint16 x, y;
  Uint16 w, h;
  Uint8 bpp;
  Uint16 pitch;
  Uint32 size;
  float fps;
} ScreenGeometry;

#endif
