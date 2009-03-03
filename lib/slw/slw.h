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

#include <keycodes.h>




 

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

  void blank();
  ///< blank the whole row in a widget

  void set_color(int col);
  /// colors are from 1 to 7, +10 highlight, +20 reverse, +30 highlight and reverse

  int color;

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

