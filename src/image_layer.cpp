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
#include <factory.h>

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
  using Surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>;
  Surface input(IMG_Load(file), SDL_FreeSurface);
  if (!input) {
    error("ImageLayer::open() error: %s", file, IMG_GetError());
    return false;
  }

  if (input->format->BitsPerPixel != 32)
    input.reset(SDL_DisplayFormat(input.get()));
  if (!input) {
    error("ImageLayer::open() could not convert %s: %s", file, SDL_GetError());
    return false;
  }

  Surface converted(SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
      input->w, input->h, 32, red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask),
      SDL_FreeSurface);
  if (!converted) {
    error("ImageLayer::open() error creating SDL surface");
    return false;
  }
  // Copy color and alpha without blending the input over the new surface.
  SDL_SetAlpha(input.get(), 0, 0);
  if (SDL_BlitSurface(input.get(), NULL, converted.get(), NULL) < 0)
    return false;
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
  SDL_FreeSurface(image);
  SDL_FreeSurface(surf);
  free(black_image);
  image = surf = nullptr;
  black_image = nullptr;
  opened = false;
  tmpImage = NULL;
}
