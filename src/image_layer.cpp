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


#include <SDL3_image/SDL_image.h>
#include <context.h>
#include <blitter.h>

#include <jutils.h>
#include <image_layer.h>
#include <config.h>

#include <jsparser_data.h>
#include <factory.h>
#include "sdl_pixels.h"

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, ImageLayer, ImageLayer, sdl);

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
  ImageLayer::close();
}

bool ImageLayer::open(const char *file) {
  using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)>;
  Surface input(IMG_Load(file), SDL_DestroySurface);
  if (!input) {
    error("ImageLayer::open() error opening %s: %s", file, SDL_GetError());
    return false;
  }

  Surface converted(SDL_ConvertSurface(input.get(), freej_pixel_format()), SDL_DestroySurface);
  if (!converted) {
    error("ImageLayer::open() could not convert %s: %s", file, SDL_GetError());
    return false;
  }
  Geometry layout;
  layout.init(converted->w, converted->h, 32);
  std::unique_ptr<void, decltype(&free)> black(calloc(1, layout.bytesize), free);
  if (!black)
    return false;

  const bool restart = is_running();
  close();
  std::lock_guard<Layer> lock(*this);
  image = input.release();
  surf = converted.release();
  black_image = black.release();
  geo.init(layout.w, layout.h, 32);
  need_crop = true;
  opened = true;
  set_filename(file);
  notice("ImageLayer opened %s :: w[%u] h[%u] (%u bytes)",
         file, geo.w, geo.h, geo.bytesize);
  return !restart || start() == 0;
}

void *ImageLayer::feed() {
  return ( (surf) ? surf->pixels : NULL );
}

void ImageLayer::close() {
  func("ImageLayer::close()");
  stop();
  std::lock_guard<Layer> lock(*this);
  clear_frame();
  SDL_DestroySurface(image);
  SDL_DestroySurface(surf);
  free(black_image);
  image = surf = nullptr;
  black_image = nullptr;
  opened = false;
  tmpImage = NULL;
}
