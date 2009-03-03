/*  S-Lang console widgets
 *
 *  (C) Copyright 2004-2008 Denis Rojo <jaromil@dyne.org>
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

#include <slw_prompt.h>


SLW_Prompt::SLW_Prompt()
	: SLangWidget() {

	set_name("prompt");
	textconsole = NULL;

}


SLW_Prompt::~SLW_Prompt() {
	if(textconsole)
		delete textconsole;
}

bool SLW_Prompt::init() {

  if(!console) {
    ::error("can't initialize widget '%s': not placed on console", name);
    return false;
  }

   // default widget settings
  initialized = true;
  visible = true;
  cursor = false;
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

bool SLW_Prompt::feed(int key) {
	switch(key) {
	case 0: return false;
	default:
		textconsole->cur_row->insert_char(key);
		textconsole->cur_x++;
	}
	refresh();
  return true;
}

bool SLW_Prompt::refresh() {
  Row *r = textconsole->cur_row;

  blank_row(1);

  putnch(r->text, 0, 0, r->len);

  return true;
}



