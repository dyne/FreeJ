/*  FreeJ
 *  (c) Copyright 2006 Denis Roio aka jaromil <jaromil@dyne.org>
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

#ifndef __TBT_LAYER_H__
#define __TBT_LAYER_H__

#include <SDL.h>
#include <SDL_ttf.h>

#include <tbt.h>

// use the abstract text console functions in SLW
#include <abs_text_console.h>

#include <layer.h>

class TBTLayer;

class TBTConsole: public TextConsole {

  public:
  TBTConsole();
  ~TBTConsole();
  
  int putnch(CHAR *str, int x, int y, int nchars);

  void blank();

  TBTLayer *layer;
  
};

class TBTLayer: public Layer {

 public:
  TBTLayer();
  ~TBTLayer();

  bool init(Context *freej);
  
  bool open(char *file);
  void *feed();
  void close();
  
  bool keypress(int key);

  TBTConsole console;

  SDL_Color bgcolor;
  SDL_Color fgcolor;
  int size;
  
  TTF_Font *font;

  SDL_Surface *surf;

 private:

  //  SDL_Surface *letter;

  TBT *tbt;
  bool interactive;
  
  int sel_font;
};

#endif

