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
 */

#ifndef __png_h__
#define __png_h__

#include <stdio.h>
#include <SDL/SDL.h>

#include <png.h>
#include <layer.h>

class PngLayer: public Layer {
private:
  /* core internal structure */
  png_structp core;
  
  /* png image informations */
  png_infop info;

  /* file signature to check for validation */
  unsigned char sig[8];

  /* row pointers */
  png_bytep *row_pointers;

  /* black image */
  void *black_image;

  void *png_image;

  /** how many times show image  when in subliminal mode */
  int subliminal;

  bool blinking;

  int count;

  FILE *fp;

public:
  PngLayer();
  ~PngLayer();
  
  bool init(Context *screen);
  bool open(char *file);
  void *feed();

  void close();
  
  bool keypress(char key);
};

#endif
