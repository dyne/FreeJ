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

#include <jutils.h>
#include <context.h>
#include <geo_layer.h>
#include <config.h>

#include <jsparser_data.h>

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
  if(surf) SDL_FreeSurface(surf);
}

bool GeoLayer::init(Context *freej) {
  int width  = freej->screen->w;
  int height = freej->screen->h;
  return init(freej, width, height);
}
bool GeoLayer::init(Context *freej, int width, int height) {

  // internal initialization
  _init(width, height);
  
  surf = SDL_CreateRGBSurface(SDL_HWSURFACE,
			      geo.w,geo.h,32,
			      red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask);
  if(!surf) {
    error("can't allocate GeoLayer memory surface");
    return(false);
  } else
    func("Geometry surface initialized");

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


bool GeoLayer::keypress(int key) {
  /* neither this */
  return(true);
}

void *GeoLayer::feed() {
  /* TODO: check synchronisation here
     we might want to use a double buffer and do a memcpy here
     or a locking system
     or a queue of operations
   */
  return(surf->pixels);
}

int GeoLayer::clear() {
  res = SDL_FillRect(surf,NULL,color);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::pixel(int16_t x, int16_t y, uint32_t col) {
  res = pixelColor(surf, x, y, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::hline(int16_t x1, int16_t x2, int16_t y, uint32_t col) {
  res = hlineColor(surf, x1, x2, y, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::vline(int16_t x, int16_t y1, int16_t y2, uint32_t col) {
  res = vlineColor(surf, x, y1, y2, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  res = rectangleColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  res = boxColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  res = lineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col) {
  res = aalineColor(surf, x1, y1, x2, y2, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::circle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  res = circleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::aacircle(int16_t x, int16_t y, int16_t r, uint32_t col) {
  res = aacircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::circle_fill(int16_t x, int16_t y, int16_t r, uint32_t col) {
  res = filledCircleColor(surf, x, y, r, col);
  if(res<0) error("error in %s (%i, %i, %i, %u)",__FUNCTION__,x,y,r,col);
  return(res);
}

int GeoLayer::ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  res = ellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  res = aaellipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col) {
  res = filledEllipseColor(surf, x, y, rx, ry, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

////

int GeoLayer::pie(uint16_t x, uint16_t y, uint16_t rad,
		  uint16_t start, uint16_t end, uint32_t col) {
  res = pieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::pie_fill(uint16_t x, uint16_t y, uint16_t rad,
		       uint16_t start, uint16_t end, uint32_t col) {
  res = filledPieColor(surf, x, y, rad, start, end, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

////

int GeoLayer::trigon(int16_t x1, int16_t y1,
	   int16_t x2, int16_t y2,
	   int16_t x3, int16_t y3, uint32_t col) {
  res = trigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::aatrigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t col) {
  res = aatrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::trigon_fill(int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		int16_t x3, int16_t y3, uint32_t col) {
  res = filledTrigonColor(surf, x1, y1, x2, y2, x3, y3, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

////

int GeoLayer::polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  res = polygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  res = aapolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col) {
  res = filledPolygonColor(surf, vx, vy, num_vertex, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}

int GeoLayer::bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t col) {
  res = bezierColor(surf, vx, vy, num_vertex, steps, col);
  if(res<0) error("error in %s",__FUNCTION__);
  return(res);
}
