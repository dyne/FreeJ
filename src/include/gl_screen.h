/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __GL_SCREEN_H__
#define __GL_SCREEN_H__

#include <GL/gl.h>
#include <GL/glu.h>

#include <screen.h>

struct Vertex
{
    float tu, tv;
    float x, y, z;
};

class GlScreen : public ViewPort {
 public:
  GlScreen();
  ~GlScreen();

  bool _init(int widt, int height);

  fourcc get_pixel_format() { return RGBA32; };
  void *get_surface();
  void *coords(int x, int y);

  // whis is the main window surface
  SDL_Surface *surface;

  float x_translation;
  float y_translation;
  float x_rotation;
  float y_rotation;
  float rotation;
  float zoom;

  // opengl stuff
  void blit(Layer *layer);
  //  bool glblitX(Layer *layer);
  GLuint texturize(Layer *layer);
  Vertex g_quadVertices[4];
  void setup_blits(Layer *lay) {};


  //  bool lock();
  //  bool unlock();
 
 private:
  int setres(int wx, int hx);
  bool dbl;
  uint32_t sdl_flags;

  // check gl error and print it
  bool check_opengl_error();

  // allow to use Factory on this class
  FACTORY_ALLOWED

};

#endif 
