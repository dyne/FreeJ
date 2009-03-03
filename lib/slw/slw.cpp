/*  S-Lang console widgets
 *
 *  (C) Copyright 2004-2006 Denis Rojo <jaromil@dyne.org>
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

#include <stdlib.h>

#include <slw.h>
#include <slw_console.h>

#include <jutils.h>

SLangWidget::SLangWidget()
  : Entry() {
  
  int c;
  
  // real name will be set by inheriting widget
  set_name("unknown");
  
  // reset geometry
  w = 0;
  h = 0;
  cur_x = 0;
  cur_y = 0;
  orig_x = 0;
  orig_y = 0;
  
  border = false;
  visible = false;
  cursor = false;
  initialized = false;
  can_focus = false;
  
  console = 0x0;
  
  // maximum lenght for a row
  max_row = 512;

  color = 1; // white on black
  
  
  // generate a blank row for line blanking
  blankrow = (char*)malloc(max_row*sizeof(char));
  for(c=0; c<max_row; c++) {
    blankrow[c] = CHAR_BLANK;
  }
    
}

SLangWidget::~SLangWidget() {   
  func("de-allocating widget");
  free(blankrow);
  
  rem();
}

int SLangWidget::feed_string(char *str) {
  register int c;
  char *p;
  
  for( p = &str[0], c = 0 ;
       *p != '\0'
	 && *p != '\n'
	 && *p != EOT;
       p++,         c++)
    this->feed( (int)(*p) );
  
  if( *p == '\n' ) feed( KEY_ENTER );
  
  return c;
}

// wrapper for broken polimorphism in old compilers
void SLangWidget::feed_key(int k) {
  this->feed( k );
}

bool SLangWidget::gotoxy(int x, int y) {
  
  // check validity of coordinates
  if( ! check(x, y) ) {
    warning("gotoxy called with invalid coords");
    return false;
  }
  
  SLsmg_gotorc(y + orig_y, x + orig_x);
  
  return true;
}

bool SLangWidget::putch(CHAR ch, int x, int y) {

  // check validity of coordinates
  if( ! check(x, y) ) {
    warning("putch called with invalid coords");
    return false;
  }

  SLsmg_set_color(color);
   
  SLsmg_gotorc( y + orig_y, x + orig_x );
  SLsmg_write_char(ch);
  
  return true;
}

int SLangWidget::putnch(CHAR *str, int x, int y, int nchars) {

  int len;
  int nwrap = 0;
  
  if( !nchars ) return 0;
  
  // check validity of coordinates
  if( ! check(x, y) ) {
    warning("putnch called with invalid coords");
    return false;
  }
  
  /* further check to cut off on the right bound
   * in case we are not wrapping, otherwise go down
   * to the next row.
   
   nchars  w  cut
   x-----------|-----              
  */
  if( x+nchars > w) {
 /*   
    if(wrap) {
      
      // here we do the wrapping
      // this is not yet working as it should
      int rlen, xwrap;
      
      for(rlen = nchars, xwrap = x;
	  rlen > 0;
	  rlen -= w-xwrap, nwrap++, xwrap = 0) {
	
	SLsmg_gotorc( nwrap+y+orig_y, xwrap+orig_x );
	SLsmg_write_nchars( str, w-xwrap );
	// if lower bound is reached, stop printing
	if(nwrap+y > h) return nwrap;
	
      }
      
      return nwrap;
      
    } else {   // no wrap: just cut off the rightmost part
   */   
      len = w - x;
      
  //  }
    
  } else // line fits in width
    len = nchars;
  
  SLsmg_set_color(color);

  SLsmg_gotorc( y+orig_y, x+orig_x );

  SLsmg_write_nchars( str, len );

  return nwrap;
}

void SLangWidget::blank_row(int y) {
  
  if( ! check(0,y) ) {
    warning("blank_row failed: y=%u",y);
    return;
  }

  SLsmg_set_color(color);
  
  SLsmg_gotorc( y+orig_y, orig_x );
  
  SLsmg_write_nchars( blankrow, w);
  
}

void SLangWidget::blank() {
  register int c;
  SLsmg_set_color(color);
  for(c=h;c>=0;c--) {
    SLsmg_gotorc( c+orig_y,orig_x);
    SLsmg_write_nchars( blankrow, w);
  }
}

	
bool SLangWidget::check(int x, int y) {

  if(!console) { // check if its placed
    error("can't draw on unplaced widget '%s'", name);
    return false;
  }
  
  if(!initialized) { // check if its initialized
    error("can't operate on non initialized widget '%s'",name);
    return false;
  }
  
  if( x>w || y>h ) { // check bounds
    warning("position out of bounds (%u,%u) in widget '%s'",
	    x, y, name);
    return false;
  }
  return true;
}

void SLangWidget::set_color(int col) {
  // check if color is valid
  switch(col) {
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
  case 16:
  case 17:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
    func("color of widget %s set to %u",name, col);
    color = col;
    break;
  default:
    warning("invalid color %u selected for widget %s", col, name);
    break;
  }
}
