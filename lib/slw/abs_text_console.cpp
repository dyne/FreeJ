/*  S-Lang console widgets
 *
 *  thread-safe text console implementation using linklist
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <jutils.h>

#include <keycodes.h>
#include <abs_text_console.h>

#define ROWCHUNK 128 // chunk to add/sub when modifying the text line


TextConsole::TextConsole() {
	
	// create the first row
	cur_row = new Row();
	rows.append(cur_row);
	
	vis_row_in = cur_row;

}

TextConsole::~TextConsole() {
  func("~TextConsole destroy");

  // delete all rows (if any)
  Row *l;
  l = (Row*) rows.begin();
  
  while(l) {
    delete l;
    l = (Row*) rows.begin();
  }
  
}



bool TextConsole::feed(int key) {
  // interprets a keycode and perform the action (or write a letter)
  
  Row *r;
  
  switch(key) {
    
  case KEY_NEWLINE:
  case KEY_ENTER:
    
    r = new Row();
    
    rows.insert_after(r, cur_row);
    
    if(cur_row->pos < cur_row->len) {
      // if cursor is not at the end of the row
      
      move_string(r, cur_row,
		  cur_row->len - cur_row->pos);
      
    }
    
    cur_y++;
    
    // scrolling
    if(cur_y>h) {
      
      vis_row_in = (Row*) vis_row_in->next;
      cur_y = h;
      
    } // else  refresh_below();
    
    cur_row = r;
    cur_x = 0;

    refresh();
    
    break;
    
  case KEY_CTRL_L:
    refresh();
    break;
    
  case KEY_BACKSPACE_ASCII:
  case KEY_BACKSPACE:
  case KEY_BACKSPACE_APPLE:
  case KEY_BACKSPACE_SOMETIMES:
    
    cur_row->backspace();
    
    if(cur_x > 0) {
      
      cur_x--; // backwards with cursor
      refresh_current();
      
    } else // backspace at the beginning of a row
      if(cur_row->prev) { // there is an upper row
	
	r = (Row*) cur_row->prev; 
	
	// upper row is empty, just delete it
	if(r->len <1) {
	  
	  r->rem();
	  delete r;
	  cur_y--;
	  
	  // upper row is not empty, append to end
	} else {
	  
	  r->pos = r->len;
	  move_string(r, cur_row, cur_row->len);
	  // delete current row (now empty)
	  cur_row->rem();
	  delete cur_row;
	  cur_y--;
	  
	  cur_row = r;
	}
	
	if(cur_y <1) { // we are up in the screen
	  
	  vis_row_in = cur_row;
	  cur_y = 0;
	  refresh();
	  
	} else { // or there is more up
	  
	  // so delete a row
	  // cur_row->rem();
	  // delete cur_row;
	  
	  // move cursor up one line
	  //	cur_y--;
	  // refresh_below();
	  refresh();
	  
	}
	
	cur_x = r->pos;
      }
    
    break;
    
  case KEY_TAB:
    if(cur_x < w) cur_x += 8;
    if(cur_x > w) cur_x = w;
    break;
    
  case KEY_LEFT:
    if(cur_x<=0) break;
    cur_x--;
    cur_row->pos--;
    break;
    
  case KEY_RIGHT:
    if(cur_x >= w) break;
    if(cur_row->pos >= cur_row->len) break;
    cur_x++;
    cur_row->pos++;
    break;
    
  case KEY_UP:
    if(cur_y>0) {
      
      cur_y--;
      cur_row = (Row*) cur_row->prev;
      if(cur_x > cur_row->len)
	cur_x = cur_row->len;
      cur_row->pos = cur_x;
      
    } else { // scroll
      
      if(!cur_row->prev) break;
      vis_row_in = cur_row = (Row*) cur_row->prev;
      refresh();
      
    }
    break;
    
  case KEY_DOWN:
    if(cur_y<h) {
      
      if(cur_row->next) {
	
	cur_y++;
	cur_row = (Row*) cur_row->next;
	// cursor positioning
	if(cur_x > cur_row->len)
	  cur_x = cur_row->len;
	cur_row->pos = cur_x;
	//
      } 
      
    } else { // scrolling
      
      if(!cur_row->next) break;
      cur_row = (Row*) cur_row->next;
      // scroll down the pointer to upper row
      vis_row_in = (Row*) vis_row_in->next;
      refresh();
      
    }
    break;
    
    
  default: // insert a new char
    
    // fits in widget width?
    if(cur_x < w) {
      
      cur_row->insert_char(key);
      
      refresh_current();
      
      cur_x++;
      
    } // else wrap to next line (TODO)
    
    break;
  }
  
  return true;
}



bool TextConsole::refresh() {
  Row *r;
  register int c;
  
  if(!vis_row_in) return false;
  else r = vis_row_in;

  // tell the renderer to blank the surface for a refresh
  // this is a pure virtual function here
  blank();

  for(c = 0; c < h; c++ ) {
    
    if(r->text)
      putnch(r->text, 0, c, r->len);
    
    if(!r->next) break;
    else r = (Row*) r->next;
    
  }
  
  return true;
}

void TextConsole::refresh_current() {
  register int c;
  Row *r;

  if(!vis_row_in) return;
  else r = vis_row_in;

  for( c = 0; c < h ; c++ ) {

    if(r == cur_row) {

      // implementation function
      blank_row(c);
			
      // widget console write
      putnch(r->text, 0, c, r->len);

      break;
    }

    if(!r->next) break;
    else r = (Row*) r->next;
  }

}

// void TextConsole::refresh_below() {
// 	register int c;
// 	Row *r;
// 	bool refr;
// 	bool blank_only;

// 	if(!vis_row_in) return;
// 	else r = vis_row_in;

// 	refr = false;
// 	blank_only = false;

// 	for(c = 0; c < h; c++) {

// 		if(r == cur_row) refr = true;

// 		if(refr) {

// 			blank_row(c);

// 			if(!blank_only)
// 				putnch(r->text, 0, c, r->len);

// 		}

// 		if(!r->next) blank_only = true;
// 		else r = (Row*)r->next;
// 	}
// }

int TextConsole::move_string(Row *dest, Row *src, int len) {
  int dlen = len; // const int arg
  
  //	func("move string");
  
  // insert the string in destination
  dlen = dest->insert_string( &src->text[ src->pos ], dlen );
  
  // delete the string from the source
  dlen = src->delete_string( dlen );
  
  return dlen;
}

Row::Row() {
  text = NULL;
  // initializing with a minimum chunk allocation??
  // fit( 1 );
  len = 0;
  max = 0;
  pos = 0;
}

Row::~Row() {
  // free buffer
  if(text) free(text);
}

int Row::insert_string(CHAR *str, int inlen) {
  register int c;
  
  
  // xpand the buffer if needed
  fit( inlen );
  
  // we are growing
  len += inlen;
  
  // slide right
  for( c = len ; c > pos; c-- )
    text[c] = text[c-inlen];
  
  // termination
  text[len+1] = EOT;
  
  // new string at current position
  for( c = 0; c < inlen; c++ )
    text[pos+c] = str[c];
  
  // advance position
  // pos += inlen;
  
  return len;
  
}

int Row::insert_char(CHAR ch) {
  register int c;
  
  // check if we need to expand the buffer
  fit( 1 );
  
  
  len++; // we are growing
  
  // slide right
  for( c = len; c > pos; c-- )
    text[c] = text[c-1];
  
  // termination
  text[len+1] = EOT;
  
  // new char at current position
  text[pos] = ch;
  
  pos++; // advance position
  
  return len;
}	

int Row::delete_string( int dlen ) {
  register int c;
  
  /* shortcut in case it's until the end of the row
   * we got some optimization ongoing here ;) small graph:
   *
   * 0    pos      len  dlen
   * |----*--------|----:
   *                xxxxx                               */
  if( dlen + pos >= len ) {
    // there is nothing to move on the right hand of deletion
    len = pos; // reduce length without overwriting chars
    return len;
  }
  
  
  /* proceed deleting dlen chars on the right
   * sliding the rightmost chars to the left */
  for( c = 0 ; c < dlen; c++) {
    if( c + pos > len ) break;
    text[c+pos] = text[c+pos+dlen];
  }
  
  len -= dlen;
  
  return c;
}

int Row::backspace() {
  register int c;
  
  // left boundary
  if(pos < 1) return len;
  
  if(pos>0) {
    
    if(pos >= len) {
      // if at the end of the line just step one back
      text[pos-1] = text[pos];
      text[pos] = CHAR_BLANK;
    } else {
      // move all right of the cursor to 1 step left
      for ( c = pos; c <= len; c++) {
	text[c-1] = text[c];
	text[c] = CHAR_BLANK;
      }
    }
    
    pos--; // backwards with position
    len--; // shrink down one
    text[len+1] = EOT;
    
  }
  
  return len;
}

bool Row::fit(int l) {
  int enlarge = 0;
  
  while(len + l >= max + enlarge)
    enlarge += ROWCHUNK;
  
  // can fit without problems
  if(!enlarge) return true;
  
  if(!text) {
    
    // this is the first time we allocate the buffer
    text = (CHAR*) calloc(max + enlarge, sizeof(CHAR));
    if( ! text ) {
      error("can't allocate new text buffer for a row: %s",
	    strerror(errno));
      return false;
    }
    
  } else {
    
    // is gonna overload the boundary
    // so we must reallocate a larger buffer
    // preserving the contents of the old
    text = (CHAR*) realloc(text, (max + enlarge) * sizeof(CHAR) );
    if( ! text) {
      error("can't reallocate text buffer for a row: %s",
	    strerror(errno));
      return false; 
    }
    
  }
  
  max += enlarge;
  
  return true;
  
}
