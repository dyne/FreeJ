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

#include <factory.h>

class GeoLayer: public Layer {
  
 public:
  GeoLayer();
  ~GeoLayer();
  
  bool init(Context *freej);
  bool init(Context *freej, int w, int h);

  bool open(const char *file);
  void *feed();
  void close();

  void set_color(uint32_t hex) { color = hex; }
  void set_color(uint16_t r, uint16_t g, uint16_t b) { set_color(r, g, b, 0xff); }
  void set_color(uint16_t r, uint16_t g, uint16_t b, uint16_t a) {
	if(SDL_BYTEORDER == SDL_LIL_ENDIAN)
		color = a|(b<<8)|(g<<16)|(r<<24);
	else
		color = a|(r<<8)|(g<<16)|(b<<24);
  }
  uint32_t get_color() { return color; }

  // drawing functions
  int clear();
  
  int pixel(int16_t x, int16_t y, uint32_t col);
  int pixel(int16_t x, int16_t y) { return pixel(x, y, color); }
  int hline(int16_t x1, int16_t x2, int16_t y, uint32_t col);
  int hline(int16_t x1, int16_t x2, int16_t y) { return hline(x1, x2, y, color); }
  int vline(int16_t x, int16_t y1, int16_t y2, uint32_t col);
  int vline(int16_t x, int16_t y1, int16_t y2) { return vline(x, y1, y2, color); }

  int rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int rectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
  		{ return rectangle(x1, y1, x2, y2, color); }
  int rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int rectangle_fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
  		{ return rectangle_fill(x1, y1, x2, y2, color); }

  int line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int line(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
  		{ return line(x1, y1, x2, y2, color); }
  int aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint32_t col);
  int aaline(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
  		{ return aaline(x1, y1, x2, y2, color); }

  int circle(int16_t x, int16_t y, int16_t r, uint32_t col);
  int circle(int16_t x, int16_t y, int16_t r) { return circle(x, y, r, color); }
  int aacircle(int16_t x, int16_t y, int16_t r, uint32_t col);
  int aacircle(int16_t x, int16_t y, int16_t r) { return aacircle(x, y, r, color); }
  int circle_fill(int16_t x, int16_t y, int16_t r, uint32_t col);
  int circle_fill(int16_t x, int16_t y, int16_t r)
  		{ return circle_fill(x, y, r, color); }

  int ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);
  int ellipse(int16_t x, int16_t y, int16_t rx, int16_t ry)
  		{ return ellipse(x, y, rx, ry, color); }
  int aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);
  int aaellipse(int16_t x, int16_t y, int16_t rx, int16_t ry)
  		{ return aaellipse(x, y, rx, ry, color); }
  int ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry, uint32_t col);
  int ellipse_fill(int16_t x, int16_t y, int16_t rx, int16_t ry)
  		{ return ellipse_fill(x, y, rx, ry, color); }

  int pie(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end, uint32_t col);
  int pie(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end)
  		{ return pie(x, y, rad, start, end, color); }
  int pie_fill(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end, uint32_t col);
  int pie_fill(uint16_t x, uint16_t y, uint16_t rad, uint16_t start, uint16_t end)
  		{ return pie_fill(x, y, rad, start, end, color); }
  
  int trigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3, uint32_t col);
  int trigon(int16_t x1, int16_t y1,
	     int16_t x2, int16_t y2,
	     int16_t x3, int16_t y3) { return trigon(x1, y1, x2, y2, x3, y3, color); }
  int aatrigon(int16_t x1, int16_t y1,
	       int16_t x2, int16_t y2,
	       int16_t x3, int16_t y3, uint32_t col);
  int aatrigon(int16_t x1, int16_t y1,
	       int16_t x2, int16_t y2,
	       int16_t x3, int16_t y3)
	       { return aatrigon(x1, y1, x2, y2, x3, y3, color); }
  int trigon_fill(int16_t x1, int16_t y1,
		  int16_t x2, int16_t y2,
		  int16_t x3, int16_t y3, uint32_t col);
  int trigon_fill(int16_t x1, int16_t y1,
		  int16_t x2, int16_t y2,
		  int16_t x3, int16_t y3)
		  { return trigon_fill(x1, y1, x2, y2, x3, y3, color); }
  
  int polygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);
  int polygon(int16_t *vx, int16_t *vy, int num_vertex)
  		{ return polygon(vx, vy, num_vertex, color); }
  int aapolygon(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);
  int aapolygon(int16_t *vx, int16_t *vy, int num_vertex)
  		{ return aapolygon(vx, vy, num_vertex, color); }
  int polygon_fill(int16_t *vx, int16_t *vy, int num_vertex, uint32_t col);
  int polygon_fill(int16_t *vx, int16_t *vy, int num_vertex)
  		{ return polygon_fill(vx, vy, num_vertex, color); }

  int bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps, uint32_t col);
  int bezier(int16_t *vx, int16_t *vy, int num_vertex, int steps)
  		{ return bezier(vx, vy, num_vertex, steps, color); }

  //  int character(int16_t x, int16_t y, char c, uint32_t color);
  //  int string(int16_t x, int16_t y, const char *c, uint32_t color);

 private:
  SDL_Surface *surf;
  uint32_t *fsurf[2];
  int doublebuf;

  int res;

  uint32_t color;

  // allow to use Factory on this class
  FACTORY_ALLOWED
};

#endif
