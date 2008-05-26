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

#ifndef __GEO_LAYER_H__
#define __GEO_LAYER_H__

#include <inttypes.h>
#include <SDL_gfxPrimitives.h>

#include <layer.h>

class GeoLayer: public Layer {
  
 public:
  GeoLayer();
  ~GeoLayer();
  
  bool init(Context *freej);
  bool init(Context *freej, int w, int h);

  bool open(const char *file);
  void *feed();
  bool keypress(int key);
  void close();

  // drawing functions
  int clear();
  
  int pixel(int16_t x, int16_t y, uint32_t col);
  int hline(int16_t x1, int16_t x2, int16_t y, uint32_t col);
  int vline(int16_t x, int16_t y1, int16_t y2, uint32_t col);

  int rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);

  int line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);

  int circle(int16_t x, int16_t y, int16_t r, uint32_t col);
  int aacircle(int16_t x, int16_t y, int16_t r, uint32_t col);
  int circle_fill(int16_t x, int16_t y, int16_t r, uint32_t col);

  int ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);
  int aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);
  int ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);

  int pie(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end, uint32_t col);
  int pie_fill(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end, uint32_t col);
  
  int trigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t col);
  int aatrigon(int16_t x1, int16_t y1,
	       int16_t x2, int16_t y2,
	       int16_t x3, int16_t y3, uint32_t col);
  int trigon_fill(int16_t x1, int16_t y1,
		  int16_t x2, int16_t y2,
		  int16_t x3, int16_t y3, uint32_t col);
  
  int polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);
  int aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);
  int polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);

  int bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t col);

  //  int character(int16_t x, int16_t y, char c, uint32_t color);
  //  int string(int16_t x, int16_t y, const char *c, uint32_t color);

  uint32_t color;

 private:
  SDL_Surface *surf;
  int res;

};

#endif
