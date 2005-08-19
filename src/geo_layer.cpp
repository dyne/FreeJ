/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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


// should make this shared soon or later
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


GeoLayer::GeoLayer() {
  surf = NULL;
  
  set_name("GEO");
  set_filename("/geometrical layer");
}

GeoLayer::~GeoLayer() {
  if(surf) SDL_FreeSurface(surf);
}

bool GeoLayer::init(Context *screen) {
  // internal initialization
  _init(screen, screen->screen->w, screen->screen->h,32);
  
  surf = SDL_CreateRGBSurface(SDL_HWSURFACE,
			      geo.w,geo.h,32,
			      rmask,gmask,bmask,amask);
  if(!surf) {
    error("can't allocate GeoLayer memory surface");
    return(false);
  } else
    func("Geometry surface initialized");

  return(true);
}

bool GeoLayer::open(char *file) {
  /* we don't need this */
  return true;
}

void GeoLayer::close() {
  /* neither this */
  return;
}


bool GeoLayer::keypress(char key) {
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

int GeoLayer::clear(uint32_t color) {
  SDL_FillRect(surf,NULL,color);
}

int GeoLayer::pixel(int16_t x, int16_t y, uint32_t color) {
  pixelColor(surf, x, y, color);
}

int GeoLayer::hline(int16_t x1, int16_t x2, int16_t y, uint32_t color) {
  hlineColor(surf, x1, x2, y, color);
}

int GeoLayer::vline(int16_t x, int16_t y1, int16_t y2, uint32_t color) {
  vlineColor(surf, x, y1, y2, color);
}

int GeoLayer::rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) {
  rectangleColor(surf, x1, y1, x2, y2, color);
}

int GeoLayer::rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) {
  boxColor(surf, x1, y1, x2, y2, color);
}

int GeoLayer::line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) {
  lineColor(surf, x1, y1, x2, y2, color);
}

int GeoLayer::aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t color) {
  aalineColor(surf, x1, y1, x2, y2, color);
}

int GeoLayer::circle(int16_t x, int16_t y, int16_t r, uint32_t color) {
  circleColor(surf, x, y, r, color);
}

int GeoLayer::aacircle(int16_t x, int16_t y, int16_t r, uint32_t color) {
  aacircleColor(surf, x, y, r, color);
}

int GeoLayer::circle_fill(int16_t x, int16_t y, int16_t r, uint32_t color) {
  filledCircleColor(surf, x, y, r, color);
}

int GeoLayer::ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t color) {
  ellipseColor(surf, x, y, rx, ry, color);
}

int GeoLayer::aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t color) {
  aaellipseColor(surf, x, y, rx, ry, color);
}

int GeoLayer::ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t color) {
  filledEllipseColor(surf, x, y, rx, ry, color);
}

////

int GeoLayer::pie(uint16_t x, uint16_t y, uint16_t rad,
		  uint16_t start, uint16_t end, uint32_t color) {
  pieColor(surf, x, y, rad, start, end, color);
}

int GeoLayer::pie_fill(uint16_t x, uint16_t y, uint16_t rad,
		       uint16_t start, uint16_t end, uint32_t color) {
  filledPieColor(surf, x, y, rad, start, end, color);
}

////

int GeoLayer::trigon(int16_t x1, int16_t y1,
	   int16_t x2, int16_t y2,
	   int16_t x3, int16_t y3, uint32_t color) {
  trigonColor(surf, x1, y1, x2, y2, x3, y3, color);
}

int GeoLayer::aatrigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t color) {
  aatrigonColor(surf, x1, y1, x2, y2, x3, y3, color);
}

int GeoLayer::trigon_fill(int16_t x1, int16_t y1,
		int16_t x2, int16_t y2,
		int16_t x3, int16_t y3, uint32_t color) {
  filledTrigonColor(surf, x1, y1, x2, y2, x3, y3, color);
}

////

int GeoLayer::polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t color) {
  polygonColor(surf, vx, vy, num_vertex, color);
}

int GeoLayer::aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t color) {
  aapolygonColor(surf, vx, vy, num_vertex, color);
}

int GeoLayer::polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t color) {
  filledPolygonColor(surf, vx, vy, num_vertex, color);
}

int GeoLayer::bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t color) {
  bezierColor(surf, vx, vy, num_vertex, steps, color);
}
