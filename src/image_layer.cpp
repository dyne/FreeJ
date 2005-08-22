/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani aka kysucix <kysucix@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * "$Id$"
 *
 */



#include <SDL_image.h>
#include <context.h>
#include <jutils.h>
#include <image_layer.h>
#include <config.h>

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
static uint32_t rmask = 0x00ff0000;
static uint8_t rchan = 1;
static uint32_t gmask = 0x0000ff00;
static uint8_t gchan = 2;
static uint32_t bmask = 0x000000ff;
static uint8_t bchan = 3;
static uint32_t amask = 0xff000000;
static uint8_t achan = 0;
#else
static uint32_t rmask = 0x000000ff;
static uint8_t rchan = 2;
static uint32_t gmask = 0x0000ff00;
static uint8_t gchan = 1;
static uint32_t bmask = 0x00ff0000;
static uint8_t bchan = 0;
static uint32_t amask = 0xff000000;
static uint8_t achan = 3;
#endif


ImageLayer::ImageLayer()
  :Layer() {

  surf = NULL;
  image = NULL;
  black_image = NULL;
  
  subliminal = 0;
  blinking = false;
  count = 0;
  set_name("IMG");
  is_native_sdl_surface = true;
}

ImageLayer::~ImageLayer() {
  close();
}

bool ImageLayer::open(char *file) {

  if(image) SDL_FreeSurface(image);

  image = IMG_Load(file);
  if(!image) {
    error("ImageLayer::open() error: %s", file,IMG_GetError());
    return false;
  }

  /**
   * Convert to display pixel format if the images is not 32 bit per pixel
   */
  if(image->format->BitsPerPixel != 32) {
    image = SDL_DisplayFormat(image);
  }

  if(surf) {
    SDL_FillRect(surf, NULL, 0x0);
    SDL_BlitSurface(image,NULL,surf,NULL);
  }

  return true;
}

bool ImageLayer::init(int width, int height) {
  func("ImageLayer::init");

  if((!width || !height) && image)
    _init(image->w, image->h);
  else
    _init(width, height);

  notice("ImageLayer :: w[%u] h[%u] (%u bytes)",
	 geo.w, geo.h, geo.size);

  surf = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
			      geo.w, geo.h, 32,
			      rmask, gmask, bmask, amask);
  SDL_FillRect(surf, NULL, 0x0);

  if(image)
    SDL_BlitSurface(image,NULL,surf,NULL);

  /** allocate memory for the black image */
  black_image = jalloc(black_image,geo.size);
  
  // paint it black!
  black_image = memset(black_image,0,geo.size);

  return true;
}
void *ImageLayer::feed() {
  count++;
  // threads problems !! XXX TODO 
  //    if(count==freej->fps_speed)
  //	count=0;
  if(blinking && count%2!=0) {
    return black_image;
  }
  // subliminal mode
  if(subliminal!=0 && count>= subliminal)
    return black_image;
  
  return surf->pixels;
}

bool ImageLayer::keypress(char key) {
  bool res = true;
  
  switch(key) {
  case 'o':
    subliminal++;
    act("ImageLayer::subliminal count %i",subliminal);
    break;
  case 'p':
    subliminal=0;
    act("ImageLayer::subliminal reset");
    break;
  case 'b':
    if(blinking)
      blinking=false;
    else 
      blinking=true;
    act("ImageLayer::blinking %s",(blinking)?"ON":"OFF");
    break;
    
  default:
    res = false;
    break;
  }
  return res;
}


void ImageLayer::close() {
  func("ImageLayer::close()");
  SDL_FreeSurface(image);
  SDL_FreeSurface(surf);
  free(black_image);
}
