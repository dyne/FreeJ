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
#include <SDL2/SDL.h>
#define MAX_GLYPHS 512
#define MAX_CHUNK 512
#define MAX_WORD 256
#define SPEED 5
#define MAX_FONTS 512

typedef struct TGlyph_
{
     FT_UInt    glyph_index;    // glyph index
     FT_Vector  baseline_position;      // glyph origin on the baseline
     FT_Glyph   image;    // glyph image

} TGlyph, *PGlyph;

typedef struct wordlist_t {
  char word[MAX_WORD];
  struct wordlist_t *next;
  struct wordlist_t *prev;
  int len;
};

class TxtLayer: public Layer {
 private:
  FILE *fd;
  
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


  // linked list of words
  Linklist words;
  Entry *current_word;

  char *chunk;
  int chunk_len;
  char *pword, *punt;
  
  int text_dimension;
  
  /* image buffer */
  void *buf;
  

  bool draw_character(FT_BitmapGlyph bitmap, int left_side_bearing, int top_side_bearing,uint8_t *dstp);

  //  int word_rw(int pos);

  int scanfonts(char *path, int depth);
  char *fonts[MAX_FONTS];
  int num_fonts;
  int sel_font;


  void render();

public:
  TxtLayer();
  ~TxtLayer();
  
  bool init(int width, int height);

  // operations on file 
  bool open(char *file);
  char *get_word(int num);
  int  wordcount();

  void *feed();
  void *get_buffer();
  void close();
  void advance();

  bool print(const char *s);
  int word_ff(int pos);


  bool set_character_size(int _text_dimension);
  bool set_font(int c);

  bool set_blink(int c);
  bool set_blink_off(int c);
  bool set_blink_on(int c);

  bool keypress(int key);
  void compute_string_bbox( FT_BBox  *abbox,FT_Glyph image );
  int string_width, string_height;

  bool next_word;
  bool inject_word;
  bool clear_screen;
  bool onscreen;
  bool blinking;
  bool use_kerning;
  int onscreen_blink;
  int offscreen_blink;
  int blink;

  uint32_t color;

};

#endif
