/*  FreeJ text layer
 *
 *  (c) 2003 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
 *  with some contributions by Denis "jaromil" Rojo <jaromil@dyne.org>
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

#include <config.h>

#ifdef WITH_FT2

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <txt_layer.h>
#include <context.h>
#include <jutils.h>


TxtLayer::TxtLayer()
  :Layer() {
  buf = NULL;
  set_name("TXT");
  change_word=true;
  clear_screen=false;
  interactive_input=false;
  onscreen=false;
  blinking=false;
  offscreen_blink = onscreen_blink = SPEED;
  blink = 0;
  text_dimension=50;
  glyph_current=NULL;
  num_fonts=0;
  sel_font=0;
  x=0;
  y=0;

  int fontnum = 0;
  fontnum += scanfonts("/usr/X11R6/lib/X11/fonts/TTF");
  fontnum += scanfonts("/usr/X11R6/lib/X11/fonts/truetype");
  fontnum += scanfonts("/usr/share/fonts/truetype");
  fontnum += scanfonts("/usr/X11R6/lib/X11/fonts/TrueType");
  act("TxtLayer fonts %i",fontnum);
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

int dirent_ttf_selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".ttf")) return(1);
  if(strstr(dir->d_name,".TTF")) return(1);
  return(0);
}
int TxtLayer::scanfonts(char *path) {
  /* add to the list of available fonts */
  struct dirent **filelist;
  char temp[256];
  int found;
  int num_before = num_fonts;
  found = scandir(path,&filelist,dirent_ttf_selector,alphasort);
  if(found<0) {
    error("no fonts found in %s : %s",path, strerror(errno)); return(false); }
  while(found--) {
    if(num_fonts>=MAX_FONTS) break;
    snprintf(temp,255,"%s/%s",path,filelist[found]->d_name);
    fonts[num_fonts] = strdup(temp);
    num_fonts++;
  }
  func("scanfont found %i fonts in %s",num_fonts-num_before,path);
  return(num_fonts - num_before);
}

bool TxtLayer::init(Context *scr) {

     if(text_dimension < 0) {
	  error("TxtLayer::init() - Character size must be positive dude!");
	  return(false);
     }

     /* Initialize the freetype library */
     if(FT_Init_FreeType( &library )) {
	  error("TxtLayer::Couldn't initialize freetype" );
	  return(false);
     }


     /* Create face object */
     if(FT_New_Face( library, fonts[sel_font], 0, &face ))  {
       error("Can't load font %s",fonts[sel_font]);
       return(false);
     }

     /* Set Character size */
     if(!set_character_size(text_dimension))
	  return(false);

     use_kerning=FT_HAS_KERNING(face);

     if(scr) freej = scr;
     // initialize to the size of the screen
     _init(freej, 0, 0, 0);
     

     buf = jalloc(buf,freej->screen->size);

     /* get the first word */
     chunk_len = 0;  
     punt = pword = chunk;

     x=(geo.w/2)*(geo.bpp/8);
     y=(geo.h/2)+50;
     return(true);
}

bool TxtLayer::print(char *s) {
  int len = strlen(s);
  if(!len) return false;
  if(len>512) len = 512;

  strncpy(word,s,len);
  word_len = len;
  
  interactive_input = true;
  clear_screen = true;
  change_word = false;
  func("TxtLayer::print : %s",word);
  return true;
}

void TxtLayer::render() {
  int ret;
  int origin_x=0;
  int origin_y=0;
  int previous=0;
  int string_width,string_height;

  glyph_current=glyphs;
  
  /*  Convert the character string into a series of glyph indices. */
  for ( int n = 0; n < word_len; n++ ) {
    glyph_current->glyph_index = FT_Get_Char_Index( face, word[n] );
    if ( use_kerning && previous && glyph_current->glyph_index ) {
      FT_Vector kerning_vector;
      FT_Get_Kerning( face, previous, glyph_current->glyph_index, ft_kerning_unfitted, &kerning_vector );
      /*  Place the pen to the cursor position. */
      origin_x += kerning_vector.x;
      origin_y = 0;
    }
    /* store current pen position */
    glyph_current->baseline_position.x = origin_x;
    glyph_current->baseline_position.y = origin_y;
    
    /* load the glyph image (in its native format); */
    ret = FT_Load_Glyph( face, glyph_current->glyph_index, FT_LOAD_DEFAULT );
    if (ret) continue; // ignore errors, jump to next glyph */
    
    /* extract glyph image and store it in our table */
    ret = FT_Get_Glyph( face->glyph, &glyph_current->image );
    if (ret) continue;
    
    /* increment pen position */
    origin_x += face->glyph->advance.x <<2;
    
    /* record current glyph index in previous (4 kerning) */
    previous = glyph_current->glyph_index;
    
    /* increment the current pointer*/
    glyph_current++;
  }
  /* count number of glyphs loaded.. */
  num_glyphs = glyph_current - glyphs;
  
  /* get bbox of original glyph sequence */
  
  FT_BBox string_bbox;
  compute_string_bbox( &string_bbox, glyph_current->image);
  
  /* compute string dimensions in integer pixels */
  string_width  = (string_bbox.xMax - string_bbox.xMin)<<6;
  string_height = (string_bbox.yMax - string_bbox.yMin)<<6;
  
  /* set up position in 26.6 cartesian space */
  
  FT_Vector vector;
  vector.x = (x<<6)-(( origin_x /2) & -64);
  vector.y = (geo.h-y)<<6;
  
  
  /* set up transform (a rotation here) */
  //	  matrix.xx = (FT_Fixed)( cos(angle)*0x10000L);
  //	  matrix.xy = (FT_Fixed)(-sin(angle)*0x10000L);
  //	  matrix.yx = (FT_Fixed)( sin(angle)*0x10000L);
  //	  matrix.yy = (FT_Fixed)( cos(angle)*0x10000L);
  
  for ( unsigned int m = 0; m < num_glyphs; m++ )
    {
      FT_Glyph  image;
      FT_BBox   bbox;
      FT_Vector translate_vector;
      
      translate_vector=glyphs[m].baseline_position;
      translate_vector.x+=vector.x;
      translate_vector.y+=vector.y;
      
      /* create a copy of the original glyph */
      ret = FT_Glyph_Copy( glyphs[m].image, &image );
      if (ret)  {
	error("TxtLayer::FT_Glyph_Copy()");
	continue;
      }
      /* transform copy (this will also translate it to the correct position */
      FT_Glyph_Transform( image, 0, &translate_vector );
      
      /* check bounding box, if the transformed glyph image 
       * is not in our target surface, we can avoid rendering it 
       */
      FT_Glyph_Get_CBox( image, ft_glyph_bbox_pixels, &bbox );
      if ( bbox.xMax <= 0
	   || bbox.xMin >= (geo.w<<2)
	   || bbox.yMax <= 0
	   || bbox.yMin >= (geo.h<<2) ) {
	func("TxtLayer::glyph out of the box");
	continue;
      }
      /* convert glyph image to bitmap (destroy the glyph) */
      ret = FT_Glyph_To_Bitmap( &image, ft_render_mode_normal, 0, 1);
      //	       ret = FT_Glyph_To_Bitmap( &image,FT_RENDER_MODE_LCD, 0, 1);
      if (!ret) {
	FT_BitmapGlyph  bit = (FT_BitmapGlyph)image;
	draw_character( bit, bit->left,bit->top,(uint8_t *)buf);
	FT_Done_Glyph( image );
      }
    }
}

void *TxtLayer::feed() {

     /* clear_screen switch serves to confirm that the screen
	has been cleaned, serving the purpose to not waste
	screen cleanups (the memset)
	same for change_word which is the next word switch
     */


     if(clear_screen) {

       jmemset(buf,0,geo.size);
       clear_screen = false; 
       onscreen = false;

     } else if(interactive_input) {

       render();
       interactive_input=false;
       onscreen=true;

     } else if(change_word) {
      
       word_len = word_ff(1);       
       render();
       change_word=false;
       onscreen=true;

     }       


     if(blinking) {
       blink++;
       if(onscreen) {
	 if(blink>onscreen_blink) {
	   clear_screen = true;
	   blink = 0;
	 }
       } else {
	 if(blink>offscreen_blink) {
	   change_word = true;
	   blink = 0;
	 }
       }
     }
	          
     return buf;
}

void TxtLayer::close() {
  int c;
  func("TxtLayer::close()");
  
  /* The library doesn't keeps a list of all allocated glyph objects */
  for(c=0;c<num_fonts;c++)
    free(fonts[c]);
  FT_Done_Face(face);
  FT_Done_FreeType( library );
  jfree(buf);
  ::close(fd);
}

int TxtLayer::read_next_chunk() {
  chunk_len = ::read(fd,chunk,MAX_CHUNK);
  /* if nothing more to read, seek to the beginning */
  if(chunk_len == 0) {
    lseek(fd,0,SEEK_SET);
    chunk_len = ::read(fd,chunk,MAX_CHUNK);
  }
  chunk[chunk_len+1] = '\0';
  pword = punt = chunk;
  return chunk_len;
}

int TxtLayer::word_ff(int pos) {
  int c;
  word_len = 0;
  for(c=pos ; c>0 ; c--) {
    /* if end of chunk is reached then suck more */
    if(punt-chunk >= chunk_len) {
      func("punt - chunk = %i and chunk_len %i",punt-chunk,chunk_len);
      read_next_chunk();
    }
    /* go forward until it meets a word */
    while(!isgraph(*punt)) {
      if((punt-chunk)==chunk_len) /* end of chunk reached */
	read_next_chunk();
      else
	punt++;
    }

    pword = punt;

    /* reaches the end of that word */
    while(   isgraph(*punt)
	     && *punt != ' '
	     && *punt != '\0'
	     && *punt != '\n'
	     && *punt != '\r'
	     && *punt != '\t') {
      if((punt-chunk)==chunk_len && (punt-pword)) { /* end of chunk reached */
	word_len += punt - pword;
	if(word_len>=512) return 512;
	strncpy(word,pword,word_len);
	read_next_chunk();
      }
      punt++;
    }
    *punt = '\0';
    if(!strlen(pword)) word_ff(1); // recursion
    if(!word_len) // it was in the same chunk 
      strncpy(word,pword,512);
    else
      strncpy(&word[word_len],pword,512-word_len);
  }
  word_len += punt - pword;
  func("word_ff(%i) got \"%s\" string of %i chars",
       pos, word, word_len);
  
  return(word_len);
}

bool TxtLayer::set_font(int c) {

  if (c>=num_fonts) 
    sel_font = num_fonts;
  else if
    (c<1) sel_font = 1;
  else
    sel_font = c;
  
  /* Create face object */
  if(FT_New_Face( library, fonts[sel_font], 0, &face )) {
    error("Can't load font %s",fonts[sel_font]);
    return false;
  }
  use_kerning = FT_HAS_KERNING(face);
  if(!set_character_size(text_dimension)) {
    error("Can't set character size for font %s",fonts[sel_font]);
    return false;
  }
    
  act("TxtLayer::font %i/%i face %s size %i",
      sel_font,num_fonts,fonts[sel_font],text_dimension);
  return true;
}

bool TxtLayer::keypress(char key) {
     int res = 1;
     switch(key) {
     case 'l': 
       change_word=true;
       clear_screen=true;
       break;
       
     case 'b': 
       if(!blinking) {
	 blinking=true;
	 clear_screen=true;
       } else blinking=false;
       act("TxtLayer::blinking %s",
	   (blinking)?"ON":"OFF");
       break;
       
     case 'j':
       offscreen_blink++;
       act("TxtLayer::offscreen blink time %i",offscreen_blink);
       break;
     case 'n':
       offscreen_blink--;
       act("TxtLayer::offscreen blink time %i",offscreen_blink);
       break;
     case 'k':
       onscreen_blink++;
       act("TxtLayer::onscreen blink time %i",onscreen_blink);
       break;
     case 'm':
       onscreen_blink--;
       act("TxtLayer::onscreen blink time %i",onscreen_blink);
       break;
       
     case 'p': /* wider */ 
       text_dimension+=5;
       set_character_size(text_dimension);
       act("TxtLayer::font size %i",text_dimension);
       break;

     case 'o': /* smaller */
       text_dimension-=5;
       set_character_size(text_dimension);
       act("TxtLayer::font size %i",text_dimension);
       break;
	    
     case 'i':
       set_font( ++sel_font );
       break;

     case 'u':
       set_font( --sel_font );
       break;

     default: 
       res = 0; 
       break;
     }

     return res;
}
void TxtLayer::compute_string_bbox( FT_BBox  *abbox,FT_Glyph image ) {
  FT_BBox  bbox;
  unsigned int t;
     bbox.xMin = bbox.yMin =  32000;
     bbox.xMax = bbox.yMax = -32000;

     for ( t = 0; t < num_glyphs; t++ ) {
	  FT_BBox   glyph_bbox;

	  FT_Glyph_Get_CBox( glyphs[t].image, ft_glyph_bbox_gridfit, &glyph_bbox );

	  if (glyph_bbox.xMin < bbox.xMin)
	       bbox.xMin = glyph_bbox.xMin;

	  if (glyph_bbox.yMin < bbox.yMin)
	       bbox.yMin = glyph_bbox.yMin;

	  if (glyph_bbox.xMax > bbox.xMax)
	       bbox.xMax = glyph_bbox.xMax;

	  if (glyph_bbox.yMax > bbox.yMax)
	       bbox.yMax = glyph_bbox.yMax;
     }

     if ( bbox.xMin > bbox.xMax ) {
	  bbox.xMin = 0;
	  bbox.yMin = 0;
	  bbox.xMax = 0;
	  bbox.yMax = 0;
     }

     *abbox = bbox;
}
bool TxtLayer::draw_character(FT_BitmapGlyph bitmap,
			      int left_side_bearing, int top_side_bearing,
			      uint8_t *dstp) {
  int z,d;
     FT_Bitmap pixel_image=bitmap->bitmap;
     ////// ci ho provato a farlo a 32 bit ma ancora non mi e' uscito -jrml
     //     uint8_t *tsrc = (uint8_t*) bitmap->bitmap.buffer ;
     //     uint32_t *src;
     //     uint32_t *dst = (uint32_t*)dstp;
     //     dst += bitmap->left + (geo.h - bitmap->top)*geo.w;
     //////
     uint8_t *src = (uint8_t*) bitmap->bitmap.buffer;
     uint8_t *dst= dstp;
     dst += bitmap->left + (geo.h - bitmap->top)*geo.pitch;
     

     

     /* Second inner loop: draw a letter;
	they come around but they never come close to */
     for(z = pixel_image.rows ; z>0 ; z--) {
	  for(d = pixel_image.width ; d>0 ; d--)  {
	    //	    src = (uint32_t*)tsrc;
	    /** Fill up rgba with the same colors */
	    *dst++=*src;
	    *dst++=*src;
	    *dst++=*src;
	    *dst++=*src;
	    src++;
	    //	    *dst = ((*src<<3)|(*src<<2)|(*src<<1)|*src);
	    //	    dst++; tsrc++;
	  }
	  dst += (geo.pitch - pixel_image.pitch*4);
	  //	  dst += (geo.w - pixel_image.width);
     }
     return(true);
}
bool TxtLayer::set_character_size(int _text_dimension) {
  text_dimension = _text_dimension;
  int ret = FT_Set_Char_Size( face, 0, text_dimension*64, 0, 0);
  if(ret<0) {
    error("TxtLayer::Couldn't set character size");
    return(false);
  }
  return(true);
}
#endif
