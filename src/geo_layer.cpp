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
#include <jsparser_data.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, GeoLayer, GeometryLayer, basic);

GeoLayer::GeoLayer()
  :Layer() {
  surf = NULL;
  fsurf[0] = NULL;
  fsurf[1] = NULL;
  color = 0xffffffff;
  set_name("GEO");
  set_filename("/geometrical layer");
  is_native_sdl_surface = true;
  jsclass = &geometry_layer_class;
}

GeoLayer::~GeoLayer() {
  if(surf) SDL_FreeSurface(surf);
  if(fsurf[0]) free(fsurf[0]);
  if(fsurf[1]) free(fsurf[1]);

}

bool GeoLayer::_init() {

  surf = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_SRCALPHA,
			      geo.w,geo.h,geo.bpp,
			      red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask);
  if(!surf) {
    error("can't allocate GeoLayer memory surface");
    return(false);
  }
  
//   fsurf = SDL_CreateRGBSurface(SDL_HWSURFACE,
// 			      geo.w,geo.h,32,
// 			      red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask);
//   if (!fsurf) {
//     error("can't allocate GeoLayer memory surface");
//     return(false);
//   }

  fsurf[0] = (uint32_t*)malloc(geo.bytesize);
  fsurf[1] = (uint32_t*)malloc(geo.bytesize);

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
  /* TODO: check synchronisation here
     we might want to use a double buffer and do a memcpy here
     or a locking system
     or a queue of operations
   */
  if (!surf)
  	return NULL;

  return surf->pixels;

}

int GeoLayer::clear() {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = SDL_FillRect(surf,NULL,color);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::pixel(int16_t x, int16_t y, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = pixelColor(surf, x, y, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::hline(int16_t x1, int16_t x2, int16_t y, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = hlineColor(surf, x1, x2, y, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::vline(int16_t x, int16_t y1, int16_t y2, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = vlineColor(surf, x, y1, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = rectangleColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = boxColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = lineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = aalineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::circle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = circleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aacircle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = aacircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::circle_fill(int16_t x, int16_t y, int16_t r, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = filledCircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s (%i, %i, %i, %u)",__PRETTY_FUNCTION__,x,y,r,col);
  return(res);
}

int GeoLayer::ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = ellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = aaellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = filledEllipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::pie(uint16_t x, uint16_t y, uint16_t rad,
		  uint16_t start, uint16_t end, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = pieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::pie_fill(uint16_t x, uint16_t y, uint16_t rad,
		       uint16_t start, uint16_t end, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = filledPieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::trigon(int16_t x1, int16_t y1,
	   int16_t x2, int16_t y2,
	   int16_t x3, int16_t y3, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = trigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aatrigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = aatrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::trigon_fill(int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		int16_t x3, int16_t y3, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = filledTrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

////

int GeoLayer::polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = polygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = aapolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = filledPolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}

int GeoLayer::bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t col) {
  if(!surf) {
    error("%s can't run: layer not initialized", __PRETTY_FUNCTION__);
    return -1;
  }
  res = bezierColor(surf, vx, vy, num_vertex, steps, col);
  if(res<0) error("error in %s",__PRETTY_FUNCTION__);
  return(res);
}
