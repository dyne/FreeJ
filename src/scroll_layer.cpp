/*  FreeJ
 *  Text Scroller layer
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 * This is a simple vertical text scroller using prebuffered rendering
 *
 */

#include <stdlib.h>
#include <errno.h>

#include <scroll_layer.h>
#include <context.h>
#include <jutils.h>
#include <font_acorn_8x8.h>
#include <config.h>

ScrollLayer::ScrollLayer()
  :Layer() {

  first = last = NULL;
  procbuf = NULL;
  path[0] = (char)NULL;

  set_name("SCR");
}

ScrollLayer::~ScrollLayer() {
  close();
}



int ScrollLayer::streol(char *line) {
  // trova newline alla fine della linea
  int c;
  char *p = line;
  if(!p) return 0;
  for(c=0;c<512;c++) {
    if(*p == '\n'
       || *p == '\0')
      break;
    else p++;
  }
  return c;
}
			  

bool ScrollLayer::init(Context *scr) {

  _init(scr,scr->screen->w,scr->screen->h,32);
  
  if(procbuf) free(procbuf);
  procbuf = malloc(geo.size);

  border = 10;
  wmax = (scr->screen->w/(CHAR_WIDTH+1))-1; // max strings in a line
  step = 1;
  line_space = 2;
  kerning = 1;
  length = 0;
  
  if(filename[0])
    _open(filename);
  
  return true;
}

void ScrollLayer::close() {
  struct txtline *l, *tmp;
  l = first;

  // cleanup all the line buffers
  while(l) {
    if(l->buf) free(l->buf);
    if(l->txt) free(l->txt);
    tmp = l->next;
    free(l);
    l = tmp;
  }

  if(procbuf) free(procbuf);
  procbuf = NULL;
  first = NULL; last = NULL;
}

void ScrollLayer::render(struct txtline *l) {
  // raster textline rendering
  // TODO: truetype font rendering
  int x,y,i,f;
  uint32_t *dst;

  if(!l->txt) { // we don't render blank lines
    l->rendered = true;
    return;
  }

  // allocate 32bit render buffer:
  // horizontal screen stripe of font's height
  // here we calcolate the size: be CAREFUL
  l->buf = calloc ( geo.pitch, CHAR_HEIGHT );
  if(!l->buf) {
    error("ScrollLayer::render can't allocate buffer");
    return;
  }
  l->size = geo.pitch*CHAR_HEIGHT;
		     
  for (y=0; y<CHAR_HEIGHT; y++) {
    dst = (uint32_t*)l->buf + y*geo.w + border;
    for(x=0;x<l->len;x++) {
      f = fontdata[l->txt[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH-1; i >= 0; i--) {
	if (f & (CHAR_START << i))
	  *dst = 0xffffffff;
	dst++; 
      }
      dst += kerning;
    }
  }
  l->rendered = true;
}

/* this open wrapper is necessary because in freej you
   open the files before initializing, while the append
   function needs the screen to be initialized in order
   to know the maximum lenght for the text lines */
bool ScrollLayer::open(char *file) {
  FILE *fd;
  fd = fopen(file,"r");
  if(!fd) {
    error("ScrollLayer::open : error opening %s : %s",
	  file, strerror(errno));
    return false;
  }
  strncpy(path,file,512);
  set_filename(file);
  fclose(fd);
  return true;
}

bool ScrollLayer::_open(char *file) {
  FILE *fd;
  char str[512]; // 4096 width resolution is bound here
  if(!path[0]) return false;
  fd = fopen(path,"r");
  if(!fd) {
    error("ScrollLayer::open : error opening %s : %s",
	  file, strerror(errno));
     return false;
  }
  while(!feof(fd)) {
    if(fgets(str,511,fd) <0) {
      error("ScrollLayer::open : error reading %s : %s",
	    file, strerror(errno));
      break;
    }
    append(str);
  }
  fclose(fd);
  set_filename(file);
  func("ScrollLayer read %u lines, maximum length is %u bytes",
       length, wmax);
  return true;
}

void ScrollLayer::append(char *txt) {
  struct txtline *l;

  // allocate structure and set all to 0
  l = (struct txtline*)calloc( 1, sizeof( struct txtline ) );

  // check length
  l->len = streol(txt);
  if(l->len) { // blank line?!
    l->len = (l->len>wmax) ? wmax : l->len;
    
    // make a copy of the string
    l->txt = (char*)calloc(l->len,sizeof(char));
    memcpy(l->txt, txt, l->len * sizeof(char));
  }

  // put it last
  if(last) last->next = l;
  if(!first) first = l;
  last = l;

  // put it bottom
  l->y = 0;

  if(!first) first = last;
  length++;

}


bool ScrollLayer::keypress(char key) {
  return false;
}


void *ScrollLayer::feed() {

  if(!first) // there is no line to process
    return procbuf;
  else // update is needed: blank the layer
    jmemset(procbuf,0,geo.size);

  struct txtline *l, *tmp;

  l = first;
  
  while(l) {
    l->y+=step;

    // check if it needs to be rendered
    if( !l->rendered ) {
      // yes, this is the last entry we process!
      render(l);
      break;
    }

    // check if it flowed out of screen
    if( l->y >= geo.h ) {
      // yes, delete the line!
      tmp = l->next;
      if(tmp) { // it is not the last line
	tmp->prev = l->prev;
	if(first==l) first = tmp;
	else l->prev->next = tmp;
	if(last==l) last = tmp;
      } else { // there are no more lines
	if(l==first) first = NULL;
	if(l==last) last = NULL;
	jmemset(procbuf,0,geo.size);
      }
      if(l->buf) free(l->buf);
      if(l->txt) free(l->txt);
      free(l);
      l = tmp;
      continue;
    }

    // make space in between
    if(l->y < CHAR_HEIGHT+line_space) break;

    if(l->buf) {
      // blit to the buffer
      jmemcpy( ((uint32_t*)procbuf) 
	       + geo.w*(geo.h-l->y),
	       l->buf, l->size );
    }

    l = l->next;
  }

  return procbuf;
}
  
