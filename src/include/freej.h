#ifndef __FREEJ_H__
#define __FREEJ_H__

#include <inttypes.h>

typedef struct {
  int16_t x, y;
  uint16_t w, h;
  uint8_t bpp;
  uint16_t pitch;
  uint32_t size;
  float fps;
} ScreenGeometry;

#endif
