/*  FreeJ
 *  (c) Copyright 2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: layer.h 670 2005-08-21 18:49:32Z jaromil $"
 *
 */

#ifndef __TEXT_LAYER_H__
#define __TEXT_LAYER_H__

#include <config.h>
#ifdef WITH_FT2

#include <SDL.h>
#include <SDL_ttf.h>
#include <layer.h>


class TextLayer: public Layer {

 public:
  TextLayer();
  ~TextLayer();

  
  bool init(Context *freej);
  bool init(Context *freej, int w, int h) { return init(freej); };

  
  bool open(const char *file);
  void *feed();
  void close();

  void calculate_string_size(char *text, int *w, int *h);
  // calculates the width and height of a string if it would be printed with
  // current settings

  void print_text(const char *str);
  void set_fgcolor(int r, int g, int b);
  void set_bgcolor(int r, int g, int b);
  bool set_font(const char *path, int sz);
  bool set_font(const char *path);
  bool set_fontsize(int sz);

  bool keypress(int key);

 private:
  SDL_Color bgcolor;
  SDL_Color fgcolor;
  int size;

  TTF_Font *font;
  char *fontfile;
  SDL_Surface *surf;

  int sel_font;

  void _display_text(SDL_Surface *newsurf);

};

#endif
#endif
