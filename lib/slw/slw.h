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


#ifndef __SLW_H__
#define __SLW_H__

#include <slang.h>

#include <linklist.h>






/////////////////////
// table of key codes
// help completing it is really appreciated! :)
// need to be expanded and tested on different platforms.
 
#define CHAR char

#define EOL    0x0a // newline
#define EOT    0x0  // '\0'

#define CHAR_BLANK 0x20

#define KEY_ENTER 13
#define KEY_SPACE 32
#define KEY_BACKSPACE 275
#define KEY_BACKSPACE_APPLE 127 
#define KEY_BACKSPACE_SOMETIMES 272
#define KEY_LEFT 259
#define KEY_RIGHT 260
#define KEY_HOME 263
#define KEY_DELETE 275
#define KEY_TAB 9

/* unix ctrl- commandline hotkeys */
#define KEY_CTRL_A 1 // goto beginning of line
#define KEY_CTRL_B 2 // change blit
#define KEY_CTRL_D 4 // delete char
#define KEY_CTRL_E 5 // add new effect
#define KEY_CTRL_F 6 // go fullscreen
#define KEY_CTRL_G 7
#define KEY_CTRL_H_APPLE 8 // ctrl-h on apple/OSX
#define KEY_CTRL_I 9 // OSD on/off
#define KEY_CTRL_K 11 // delete until end of line
#define KEY_CTRL_L 12 // refresh screen
#define KEY_CTRL_M 13 // move layer
#define KEY_CTRL_U 21 // delete until beginning of line
#define KEY_CTRL_H 272 // help the user
#define KEY_CTRL_J 10 // javascript command
#define KEY_CTRL_O 15 // open a file in a new layer
#define KEY_CTRL_S 19 // start streaming (overrides scroll lock)
#define KEY_CTRL_T 20 // new layer with text
#define KEY_CTRL_V 22 // change blit value
#define KEY_CTRL_W 23 // start stream and save to file
#define KEY_CTRL_X 24
#define KEY_CTRL_Y 25

#define KEY_PLUS 43

// end of key codes
/////////////////////


class SLangConsole;


class SLangWidget : public Entry {

	public:

		SLangWidget();
		virtual ~SLangWidget();


		/// the following virtual functions must be implemented
		//  by every widget:

		virtual bool init() =0;
		///< called by the SLangConsole::place when the widget is placed
		
		virtual bool feed(int key) =0;
		///< pass keypress to be processed

		virtual bool refresh() =0;
		///< redraw the whole widget

		///////////////////////////////////
		/// functions to feed input in the widget
		// they can be used by external processes to
		// pass list of events into the widget

		int feed_string(char *str);
		///< pass a string of keys as input to the widget
		
		void feed_key(int k);
		///< pass a single key as input to the widget

		///////////////////////////////////

		/// the following functions can be used by the widget to
		//  draw inside itself: all coordinates are relative to the
		//  widget position inside the console.

		bool gotoxy( int x, int y);
		///< move cursor at given position

		bool putch(CHAR ch, int x, int y);
		///< draw a character at given position

		int putnch(CHAR *str, int x, int y, int nchars);
		///< draw a string of certain length at given position

		void blank_row(int y);
		///< blank the whole row in a widget

//		bool putrow(CHAR *str, int y);
		///< draw a whole row at given height
		
//		int putnrow(CHAR *str, int y, int nchars);
		///< draw a row of certain length at given height
		
		// TODO: colors, putcolumn, putncolumns, box, hline, vline
		//////////////////////////////////////////////////////////

		int orig_x, orig_y; ///< placement

		int w, h; ///< geometry

		int cur_x, cur_y; ///< cursor positioning

		bool border;	///< line border around widget (external)
		bool wrap;  ///< if true rightmost text wraps to next line
		bool visible;	///< widget is shown
		bool cursor;	///< widget has cursor
		bool initialized; ///< widget has been initialized
		bool can_focus; ///< widget can take focus
		
		SLangConsole *console;
		///< the console where this widget is placed

	private:
		bool check(int x, int y);
		///< check if coordinates are valid for this widget placing
		char *blankrow; ///< blank buffer
		int max_row; ///< maximum lenght possible for a row
};

#endif

