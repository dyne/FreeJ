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

#include <slw_popup.h>

SLW_Popup::SLW_Popup()
	: SLangWidget() {

	set_name("popup");
	txt = NULL;
	len = 0;

}

SLW_Popup::~SLW_Popup() { 

	if(txt) free(txt);

}

bool SLW_Popup::init() {

	if(!console) {
		error("can't initialize widget '%s': not places on console",
				name);
		return false;
	}

	// default widget settings
	initialized = true;
	visible = true;
	cursor = false;
	can_focus = true;
	border = true;

	refresh();

	return true;
}

bool SLW_Popup::feed(int key) {
	// interprets a keycode and perform the action (or write a letter)

	return true;
}

bool SLW_Popup::refresh() {
	int tc, hc, lc;
	char *start, *end;

	if(!txt) return false;

	start = end = txt; // char pointers

	tc = 0; // whole text counter

	for(hc=0; hc < h; hc++ ) {
		
		blank_row(hc);

		for( lc=0; tc < len-2; tc++, lc++, end++ ) {

			if( *end == '\n' ) // there is a newline
				break;
			
			if( lc >= w ) // line is longer than width
				break;

		}

		func("popup printing line %i long %i chars (%i of %i)",
				hc, lc, tc, len);

		putnch(start, 1, hc, lc);

		if(tc >= len) // text is really over
			break;

		end++;
		start = end;
	}

	return true;
}

bool SLW_Popup::set_text(const char *text) {
	// set a NULL terminated text for the dialog
	int c, num;

	// count the chars until the NULL
//	for( p = text , c = 0 ; *p != '\0'; p++ , c++     );
	c = strlen(text);
	func("popup text long %i",c);
	
	num = c * sizeof(char);

	if(txt) free(txt);

	txt = (char*) malloc( num );

	memcpy(txt, text, num);

	len = c;

	// safety bound
	//	txt[len+1] = '\0';
	//	txt[len+2] = '\0';


	return true;
}

