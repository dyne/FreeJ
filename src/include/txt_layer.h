/*  FreeJ text layer
 *  Silvano Galliani aka kysucix <silvano.galliani@milug.org>
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

#ifndef __txt_u__
#define __txt_h__

#include <stdio.h>
#include <layer.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <ctype.h>
#include <SDL/SDL.h>

class TxtLayer: public Layer {
 private:
  int fd;
  
  /* handle to library     */
  FT_Library   library;   
  
  /* handle to face object */
  FT_Face      face;      
  FT_GlyphSlot  slot;
  
  char line[512]; // safe bound
  int line_len;
  char *word, *punt;
  int word_len;

  bool change_word;
  bool clear_screen;
  int text_dimension, ascissa, letter;
  
  /* image buffer */
  void *buf;
  int center;
  
  int word_ff(int pos);
  //  int word_rw(int pos);

public:
  TxtLayer();
  ~TxtLayer();
  
  bool init(Context *screen=NULL);
  bool open(char *file);
  void *feed();
  void *get_buffer();
  void close();
  bool keypress(SDL_keysym *keysym);
};

#endif
