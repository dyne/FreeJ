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

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <txt_layer.h>
#include <context.h>
#include <lubrify.h>
#include <jutils.h>
#include <config.h>


TxtLayer::TxtLayer()
  :Layer() {
  buf = NULL;
  setname("TXT");
  change_word=true;
  text_dimension=0;
  clear_screen=false;
}

TxtLayer::~TxtLayer() {
  close();
}

bool TxtLayer::open(char *file) {
  func("TxtLayer::open(%s)",file);
  
  fd = ::open(file,O_RDONLY|O_NONBLOCK);
  if(fd<0) {
    error("TxtLayer::open(%s) - %s",
	  file, strerror(errno));
    return (false);
  }
  
  set_filename(file);
  
  return(true);
}

bool TxtLayer::init(Context *scr) {

  text_dimension=64;
  
  /* Initialize the freetype library */
  if(FT_Init_FreeType( &library )) {
    error("TxtLayer::Couldn't initialize freetype" );
    return(false);
  }
  if(FT_New_Face( library, "/usr/X11R6/lib/X11/fonts/truetype/arial.ttf", 0, &face ))  {
    error("TxtLayer::Couldn't load ttf font");
    return(false);
  }
  
  /* Guess?? */
  FT_Set_Pixel_Sizes(face, 0, text_dimension);
     
  /* Comfortable */
  slot = face->glyph; 
  
  if(scr) screen = scr;
  _init(screen, screen->w, screen->h, screen->bpp);
  
  buf = jalloc(buf,screen->size);

  /* get the first word */
  line_len = 0;  
  punt = word = line;
  
  /* TODO: fetch some more data about the chars and use it
     to center them better */
  center = (int)(geo.pitch*(geo.h/2.5)) + geo.pitch/2;
  return(true);
}

void *TxtLayer::feed() {
  int ret,z,i,c;
  
  /* One word */
  if(change_word && !clear_screen) {
    int n=0;
    word_len = word_ff(1);
    /* load glyph image into the slot */
    ret = FT_Load_Char( face, word[n], FT_LOAD_RENDER );
    letter=0;
    

    /* first inner loop:
       draw the whole word */
    for(i=0 ; i<word_len ; i++) {
      
      Uint8 *src = slot->bitmap.buffer ;
      Uint8 *dst= (Uint8 *) buf;
      
      /* optimized to not use any multiply or division
	 === now ascissa became word_len (which is a more explicative variable name)
	 ascissa was lately used in the inner loop as
	 (ascissa/2)*text_dimension
	 so i made text_dimension a power of 2 (128) and it comes
	 (ascissa>>1)<<7 which can be semplified to a shift left of 6 bits
	 [jrml 19may03] */
      dst += center + (letter << 5) - (word_len<<5);
      /* dirty fix for the lineup of the 'l' and some other letters 
	 TODO : implement a decent alignement method for letters */
      if( word[n] == 'l'
	  || word[n] == 't'
	  || word[n] == 'f'
	  || word[n] == 'i'
	  || word[n] == 'd'
	  || word[n] == 'b'
	  || word[n] == 'I'
	  || word[n] == 'h')
	dst -= (geo.pitch<<3) + (geo.pitch<<2);


      /* Second inner loop: draw a letter;
	 they come around but they never come close to 
	 TODO: this has to be fixed indeed */
      for(z=0 ; z<(slot->bitmap.rows) ; z++) {

	for(c = slot->bitmap.width; c>0 ; c--) {
	  //	  *dst++ = *src;
	  *dst++ = *src++;
	  // dst[1] = *src;
	  // dst[2] = *src;
	  // dst[3] = (alpha_blit) ? 0x00 : 0xff;
	}
	// memcpy( dst, src, slot->bitmap.pitch );
	// src += slot->bitmap.pitch;
	dst += geo.pitch - slot->bitmap.pitch;
      }
      
      n++;
      letter+=2;
      ret = FT_Load_Char( face, word[n], FT_LOAD_RENDER );
    }
    
    change_word=false;
  }
  /* now guess */
  else if(clear_screen) {
    bzero( buf, geo.size);
    clear_screen=false;
  }
  
  return buf;
}

void TxtLayer::close() {
  func("TxtLayer::close()");
  FT_Done_FreeType( library );
  jfree(buf);
  ::close(fd);
}

int TxtLayer::word_ff(int pos) {
  int c;
  for(c=pos ; c>0 ; c--) {

    /* if end of line is reached then suck more */
    if(punt-line >= line_len) {
      line_len = read(fd,line,512);
      func("succhiato %i",line_len);
      /* if nothing more to read, seek to the beginning */
      if(line_len <= 0) {
	lseek(fd,0,SEEK_SET);
	line_len = read(fd,line,512);
      }
      line[line_len] = '\0';
      func("read new line: %s",line);
      punt = line;
    }
    
    /* go forward until it meets a word */
    while( (punt-line) < line_len 
	   && !isgraph(*punt) ) 
      punt++;

    word = punt;

    /* reaches the end of that word */
    while(    (punt-line) < line_len
	      && isgraph(*punt) )
	      /*	      && *punt != ' '
			      && *punt != '\0'
			      && *punt != '\n'
			      && *punt != '\r'
			      && *punt != '\t') */
      punt++;
    *punt = '\0';
  }
  func("word_ff(%i) got \"%s\" string of %i chars",
       pos, word, punt-word);
  return(punt-word);
}

bool TxtLayer::keypress(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_RIGHT: 
    change_word=true;
    clear_screen=true;
    break;

  case SDLK_LEFT:
    change_word=true;
    clear_screen=true;
    word_ff(1);
    break;
    
  default: 
    res = 0; 
    break;
  }

  return res;
}

