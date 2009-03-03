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

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <jutils.h>

#include <slw_text.h>


SLW_Text::SLW_Text()
  : SLangWidget() {
  
  set_name("text");
  
  textconsole = NULL;
}

SLW_Text::~SLW_Text() {
  if(textconsole)
    delete textconsole;
}


bool SLW_Text::init() {
  
  if(!console) {
    error("can't initialize widget '%s': not placed on console", name);
    return false;
  }
  
  // default widget settings
  initialized = true;
  visible = true;
  cursor = true;
  can_focus = true;
  

  // create the private structure where to hold text
  if(textconsole) delete textconsole;
  textconsole = new SLW_TextConsole();
  textconsole->widget = this;
  textconsole->w = w;
  textconsole->h = h;
  textconsole->cur_x = 0;
  textconsole->cur_y = 0;

  
  refresh();
  
  return true;
}


bool SLW_Text::feed(int key) {
  // interprets a keycode and perform the action (or write a letter)
  bool res;
  res = textconsole->feed(key);
  if(res) {
    cur_x = textconsole->cur_x;
    cur_y = textconsole->cur_y;
  }
  return res;
}

bool SLW_Text::refresh() {
  textconsole->refresh();
  return true;
}

////////////////////////////////////////
////// textconsole widget implementation
SLW_TextConsole::SLW_TextConsole()  { }
SLW_TextConsole::~SLW_TextConsole() { }
////// overloaded functions
int SLW_TextConsole::putnch(CHAR *str, int x, int y, int nchars) {
	return widget->putnch(str, x, y, nchars);
}
void SLW_TextConsole::blank() {
	widget->blank();
}
void SLW_TextConsole::blank_row(int r) {
	widget->blank_row(r);
}
///////////////////////////////////////


// void SLW_Text::refresh_below() {
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
