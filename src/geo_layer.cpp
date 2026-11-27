/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <config.h>

#include <stdlib.h>

#include <jutils.h>
#include <context.h>
#include <geo_layer.h>
#include <cstring>
#include <SDL_gfxPrimitives.h>
#include "sdl_pixels.h"
#include <jsparser_data.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, GeoLayer, GeometryLayer, basic);

GeoLayer::GeoLayer()
  :Layer() {
  surf = NULL;
  color = 0xffffffff;
  set_name("GEO");
  set_filename("/geometrical layer");
  is_native_sdl_surface = true;
  jsclass = &geometry_layer_class;
}

GeoLayer::~GeoLayer() {
  stop();
  if(surf) SDL_DestroySurface(surf);
}

bool GeoLayer::_init() {

  surf = SDL_CreateSurface(geo.w, geo.h, freej_pixel_format());
  if(!surf) {
    error("can't allocate GeoLayer memory surface");
    return(false);
  }
  
  snapshot_.resize(geo.pixelsize);
  SDL_FillSurfaceRect(surf, nullptr, 0);

  func("Geometry surface initialized");
  opened = true;
  return(true);
}

bool GeoLayer::open(const char *file) {
  /* we don't need this */
  return true;
}

void GeoLayer::close() {
  /* neither this */
  return;
}


void *GeoLayer::feed() {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if (!surf)
    return nullptr;

  // Draw calls update the canvas. The worker owns this packed snapshot,
  // including any changes made by filters before publishing the frame.
  const auto *src = static_cast<const uint8_t *>(surf->pixels);
  const std::size_t row_bytes = std::size_t(surf->w) * sizeof(uint32_t);
  for (int row = 0; row < surf->h; ++row)
    std::memcpy(snapshot_.data() + std::size_t(row) * surf->w,
                src + std::size_t(row) * surf->pitch, row_bytes);
  return snapshot_.data();
}

int GeoLayer::clear() {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const auto rgba = get_color();
  const auto pixel = SDL_MapSurfaceRGBA(surf, rgba >> 24, rgba >> 16, rgba >> 8, rgba);
  const int res = SDL_FillSurfaceRect(surf, nullptr, pixel) ? 0 : -1;
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::pixel(int16_t x, int16_t y, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = pixelColor(surf, x, y, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::hline(int16_t x1, int16_t x2, int16_t y, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = hlineColor(surf, x1, x2, y, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::vline(int16_t x, int16_t y1, int16_t y2, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = vlineColor(surf, x, y1, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = rectangleColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = boxColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = lineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = aalineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::circle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = circleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aacircle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = aacircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::circle_fill(int16_t x, int16_t y, int16_t r, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = filledCircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s (%i, %i, %i, %u)",__PRETTY_FUNCTION__,x,y,r,col);
  return(res);
}

int GeoLayer::ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = ellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = aaellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = filledEllipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::pie(uint16_t x, uint16_t y, uint16_t rad,
		  uint16_t start, uint16_t end, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = pieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::pie_fill(uint16_t x, uint16_t y, uint16_t rad,
		       uint16_t start, uint16_t end, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = filledPieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::trigon(int16_t x1, int16_t y1,
	   int16_t x2, int16_t y2,
	   int16_t x3, int16_t y3, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = trigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aatrigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = aatrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::trigon_fill(int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		int16_t x3, int16_t y3, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = filledTrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = polygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = aapolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = filledPolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t col) {
  std::lock_guard<std::mutex> lock(drawing_mutex_);
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  const int res = bezierColor(surf, vx, vy, num_vertex, steps, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}
