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
#include FT_FREETYPE_H /* bha' */
#include FT_GLYPH_H /* wtf? */

#include <ctype.h>
#include <SDL/SDL.h>
#define MAX_GLYPHS 512
#define MAX_CHUNK 512
#define SPEED 5
#define MAX_FONTS 256

typedef struct TGlyph_
{
     FT_UInt    glyph_index;    // glyph index
     FT_Vector  baseline_position;      // glyph origin on the baseline
     FT_Glyph   image;    // glyph image

} TGlyph, *PGlyph;

class TxtLayer: public Layer {
 private:
  int fd;
  
  /* handle to library     */
  FT_Library library;   
  
  /* handle to face object */
  FT_Face face;      

  /* handle to glyph image */
  FT_Glyph glyph;

  /* Transformation matrix for FT_Glyph_Transform()*/
  FT_Matrix matrix; /* neo sux */
  
  /* Translation vector for FT_Glyph_To_Bitmap() */
  FT_UInt num_glyphs;
  int x,y;

  /* glyphs table */
  TGlyph glyphs[ MAX_GLYPHS ]; 

  /* current glyph in table */
  PGlyph glyph_current; 
  FT_UInt glyphs_numbers;

  char chunk[MAX_CHUNK+256]; // safe bound
  char word[MAX_CHUNK+256];
  int chunk_len;
  char *pword, *punt;
  int word_len;

  bool change_word;
  bool clear_screen;
  bool onscreen;
  bool blinking;
  bool use_kerning;
  int onscreen_blink;
  int offscreen_blink;
  int blink;
  
  int text_dimension;
  
  /* image buffer */
  void *buf;
  
  int word_ff(int pos);
  int read_next_chunk();
  bool draw_character(FT_BitmapGlyph bitmap, int left_side_bearing, int top_side_bearing,uint8_t *dstp);

  //  int word_rw(int pos);

  int scanfonts(char *path);
  char *fonts[MAX_FONTS];
  int num_fonts;
  int sel_font;

public:
  TxtLayer();
  ~TxtLayer();
  
  bool init(Context *screen);
  bool open(char *file);
  void *feed();
  void *get_buffer();
  void close();
  bool set_character_size(int _text_dimension);
  bool keypress(char key);
  void compute_string_bbox( FT_BBox  *abbox,FT_Glyph image );
};

#endif
