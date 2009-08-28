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
 */

/**
   @file image_layer.h
   @brief FreeJ Layer class to show images (SDL_image)
*/


#include <SDL_image.h>
#include <context.h>
#include <blitter.h>

#include <jutils.h>
#include <image_layer.h>
#include <config.h>

#include <jsparser_data.h>

ImageLayer::ImageLayer()
  :Layer() {

  surf = NULL;
  image = NULL;
  black_image = NULL;

  set_name("IMG");
  is_native_sdl_surface = true;
  jsclass = &image_layer_class;

  opened = false;
}

ImageLayer::~ImageLayer() {
  func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
  close();
}

bool ImageLayer::open(const char *file) {

  if(image) SDL_FreeSurface(image);
  if(surf) SDL_FreeSurface(surf);


  image = IMG_Load(file);
  if(!image) {
    error("ImageLayer::open() error: %s", file,IMG_GetError());
    return false;
  }
  set_filename(file);

  /**
   * Convert to display pixel format if the images is not 32 bit per pixel
   */
  if(image->format->BitsPerPixel != 32) {
    image = SDL_DisplayFormat(image);
  }

  // allocate the hw accelerated surface
  surf = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
			      image->w, image->h, 32,
			      red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask);
  if(!surf) {
    error("ImageLayer::open() error creating SDL surface");
    return false;
  }

  //SDL_FillRect(surf, NULL, 0x0);


  geo.init(image->w, image->h, 32);

  
  notice("ImageLayer opened %s :: w[%u] h[%u] (%u bytes)",
	 file, geo.w, geo.h, geo.bytesize);


  /** allocate memory for the black image */
  if(black_image) {
    free(black_image);
    black_image = NULL;
  }

  black_image = malloc(geo.bytesize);
  // paint it black!
  black_image = memset(black_image,0,geo.bytesize);

  // do not apply the mask,
  // copy image+alpha to surf
  SDL_SetAlpha( image, 0, 0 );

  //SDL_FillRect(surf, NULL, alpha_bitmask);
  SDL_BlitSurface(image,NULL,surf,NULL);

  opened = true;

  return true;
}

void *ImageLayer::feed() {
  return ( (surf) ? surf->pixels : NULL );
}

void ImageLayer::close() {
  func("ImageLayer::close()");
  SDL_FreeSurface(image);
  SDL_FreeSurface(surf);
  free(black_image);
  tmpImage = NULL;
}
