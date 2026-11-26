#ifndef FREEJ_SDL_PIXELS_H
#define FREEJ_SDL_PIXELS_H

#include <screen.h>

inline SDL_PixelFormat freej_pixel_format(bool alpha = true)
{
  return SDL_GetPixelFormatForMasks(32, red_bitmask, green_bitmask,
                                    blue_bitmask, alpha ? alpha_bitmask : 0);
}

#endif
