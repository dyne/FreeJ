/*  Abstract text console class
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
 * =========
 * This class provides abstract functionalities for a text console with
 * full editing scrolling and history. It makes use of linked lists and
 * it is optimized for speed. It still misses word wrapping.
 *
 * It is a pure virtual class and as such needs to be inherited by a
 * class implementing init() feed(int key) and refresh().
 *
 */

#ifndef __TEXT_CONSOLE_H__
#define __TEXT_CONSOLE_H__

#include <linklist.h>

#define ROWCHUNK 128


/////////////////////
// table of key codes
// help completing it is really appreciated! :)
// need to be expanded and tested on different platforms.
 
// define the type we use for a char
// if not defined then is just char
#ifndef CHAR
#define CHAR char
#endif


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



class Row;

class TextConsole {

 public:
  TextConsole();
  virtual ~TextConsole();
  
  
  bool feed(int key);
  
  bool refresh();
  
  // pure virtual functions to be overloaded
  
  virtual void blank() =0; ///< blank the whole screeen

  virtual int putnch(CHAR *str, int x, int y, int nchars) =0;
  ///< draw a string of certain length at given x,y position
  
  ////////////////////////
  
  
  int move_string(Row *dest, Row *src, int len);
  ///< move a string from a row to another
  
  bool scroll; ///< if true we have vertical scrolling
  
  int w, h; ///< geometry
  
  int cur_x, cur_y; ///< cursor positioning
  
  
 private:
  Linklist rows;
  Row *cur_row;
  Row *vis_row_in;
  
  // void refresh_row(int y);
  ///< refresh row at position y
  
  // void refresh_current();
  ///< refresh row at current cursor position
  
  // void refresh_below();
  ///< refresh all lines below the current cursor position
  
};


class Row : public Entry {
	
 public:
  Row();
  ~Row();
  
  /** this class implements a couple of methods to insert and
   * delete text in each row
   * the property 'pos' in each row is the current position,
   * meaning that operations are executed in that point
   * call it 'procedural pointer?'
   * it's just a way to save an argument in functions :)
   */
  
  int insert_char(CHAR ch);
  ///< insert a single char at current pos
  
  int insert_string(CHAR *str, int inlen);
  ///< instert string at pos
  
  int delete_string(int dlen);
  ///< delete a string right of current pos
  
  int backspace(); // deletes one char left of pos
  
  //		clear_right(int p); // clear all chars right of pos
  //		clear_left(int p); // clear all chars left of pos
  
  int len;
  int pos;
  
  CHAR *text;
  
  
 private:
  bool fit(int l); ///< realloc if necess. to fit in +len
  int max; // current allocated maximum
};


 

#endif
