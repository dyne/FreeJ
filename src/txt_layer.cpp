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

// this layer deactivated by default
#undef WITH_FT2

#ifdef WITH_FT2

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

#include <jsparser_data.h>

TxtLayer::TxtLayer()
  :Layer() {
  buf = NULL;
  set_name("TXT");
  next_word=false;
  inject_word=false;
  clear_screen=false;
  onscreen=false;
  blinking=false;
  offscreen_blink = onscreen_blink = SPEED;
  blink = 0;
  fd = 0;
  text_dimension=50;
  glyph_current=NULL;
  num_fonts=0;
  sel_font=0;
  
  current_word = NULL;

  x=0;
  y=0;

  scanfonts("/usr/X11R6/lib/X11/fonts/TTF");
  scanfonts("/usr/X11R6/lib/X11/fonts/truetype");
  scanfonts("/usr/X11R6/lib/X11/fonts/TrueType");
  scanfonts("/usr/share/truetype");
  scanfonts("/usr/share/fonts/truetype/freefont");

  func("TxtLayer fonts %i",num_fonts);

  jsclass = &txt_layer_class;
}

TxtLayer::~TxtLayer() {
     close();
}

bool TxtLayer::open(char *file) {
  Entry *tmpw;

  func("TxtLayer::open(%s)",file);
  
  fd = ::fopen(file,"r");
  if(!fd) {
    error("TxtLayer::open(%s) - %s",
	  file, strerror(errno));
    return (false);
  }

  // read it all in memory
  fseek(fd,0,SEEK_END);
  chunk_len = ftell(fd);
  rewind(fd);
  chunk = (char*)calloc(chunk_len,sizeof(char));
  fread(chunk,chunk_len,1,fd);
  fclose(fd);

  punt = chunk;
  
  // now fill up the linklist

  while(punt-chunk<chunk_len) { // parse it all until the end

    while(!isgraph(*punt)) // goes forward until it meets a word
      if(punt-chunk>=chunk_len) // end of chunk reached
	return true;
      else punt++;

    // word found, now reach its end
    pword = punt;
    while(   isgraph(*punt)
	     && *punt != ' '
	     && *punt != '\0'
	     && *punt != '\n'
	     && *punt != '\r'
	     && *punt != '\t') {
      if(punt-chunk >= chunk_len) // end of chunk reached
	return true;
      else punt++;
    }

    // there is a word to acquire!

    // create the new entry
    tmpw = new Entry();
    memset(tmpw->name,0,254);
    strncpy(tmpw->name,pword,punt-pword);
    // append it to the list
    words.append(tmpw);
    
  }

  free(chunk);
  current_word = words.begin();
  set_filename(file);
  
  return(true);
}

char *TxtLayer::get_word(int num) {
  Entry *ent;
  ent = words.pick(num+1);
  if(!ent) return NULL;
  return ent->name;
}

int TxtLayer::wordcount() {
  return words.len();
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
    func("no fonts found in %s : %s",path, strerror(errno)); return(false); }
  while(found--) {
    if(num_fonts>=MAX_FONTS) break;
    snprintf(temp,255,"%s/%s",path,filelist[found]->d_name);
    fonts[num_fonts] = strdup(temp);
    num_fonts++;
  }
  func("scanfont found %i fonts in %s",num_fonts-num_before,path);
  return(num_fonts - num_before);
}

bool TxtLayer::init(int width, int height) {

  if(!num_fonts) {
    error("no truetype fonts found on your system, dirs searched:");
    error("/usr/X11R6/lib/X11/fonts/TTF");
    error("/usr/X11R6/lib/X11/fonts/truetype");
    error("/usr/X11R6/lib/X11/fonts/TrueType");
    error("/usr/share/truetype");
    error("you should install .ttf fonts in one of the directories above.");
    return false;
  }

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

     // TODO: initialize to which size?
     _init(width, height);
     

     buf = jalloc(buf,geo.size);

     /* get the first word */
     chunk_len = 0;  
     punt = pword = chunk;

     x=(geo.w/2)*(geo.bpp/8);
     y=(geo.h/2);
     return(true);
}

void TxtLayer::render() {
  int ret;
  int origin_x=0;
  int origin_y=0;
  int previous=0;
  int len, n;

  if(!current_word) return;
  len = strlen(current_word->name);

  glyph_current=glyphs;

  /*  Convert the character string into a series of glyph indices. */
  for ( n = 0; n < len; n++ ) {
    glyph_current->glyph_index = FT_Get_Char_Index( face, current_word->name[n] );
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
	same for next_word which is the next word switch
     */


     if(clear_screen) {

       memset(buf,0,geo.size);
       clear_screen = false; 
       onscreen = false;

     } else if(next_word) {

       if(!current_word)
	 current_word = words.begin();
       else
	 current_word = current_word->next;

       if(current_word) {
	 render();
	 next_word=false;
	 onscreen=true;
       }

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
	   next_word = true;
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

  // free all the words
  Entry *tmp = words.begin();
  while(tmp) {
    words.rem(1);
    delete tmp;
    tmp = words.begin();
  }
    
  FT_Done_Face(face);
  FT_Done_FreeType( library );
  jfree(buf);
}

bool TxtLayer::print(char *s) {
  Entry *tmpw;
  //  int len = strlen(s);

  //  if(!len) return false;
  //  if(len>MAX_WORD) len = MAX_WORD;


  tmpw = new Entry();
  tmpw->set_name(s);

  words.append(tmpw);

  /*
  if(!current_word) current_word = words.begin();

  if(current_word) {

    words.insert_after(tmpw,current_word);

  } else words.append(tmpw);
  */

  clear_screen = true;
  next_word = true;
  //  render();

  return true;
}

bool TxtLayer::set_font(int c) {

  if (c>=num_fonts) 
    sel_font = 0;
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

void TxtLayer::advance() {
  next_word = true;
  clear_screen = true;
}

bool TxtLayer::keypress(int key) {
     int res = 1;
     switch(key) {

       // ENTER
     case (char)0x111:
     case (char)13:
       next_word=true;
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
       text_dimension++;
       set_character_size(text_dimension);
       act("TxtLayer::font size %i",text_dimension);
       break;

     case 'o': /* smaller */
       text_dimension--;
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
  //  for(z = 0; z< pixel_image.rows ; z++) {
  //    for(d = 0; d< pixel_image.width ; d++)  {
      
      //	    src = (uint32_t*)tsrc;
      /** Fill up rgba with the same colors */
      *dst++ = *src;
      *dst++ = *src;
      *dst++ = *src;
      *dst++ = *src;
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
