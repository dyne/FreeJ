/*  S-Lang console functions
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

#ifndef __SLW_CONSOLE_H__
#define __SLW_CONSOLE_H__


#include <slw.h>


extern volatile int SLang_Error; // hack for old versions 


class SLangConsole {
 public:
  
  SLangConsole();
  ~SLangConsole();

  bool init();

  void close();



  // by short circuiting the three functions below we make everything run
 
  int getkey(); ///< returns the key typed
  
  void feed(int key); ///< interprets and print the keycode

  bool refresh(); ///< run one cycle, return false if quit signal was received



  int w, h; ///< console geometry



  bool place(SLangWidget *wid,
             int hx, int hy, int lx, int ly);
             ///< place a widget on this console
	     //   box coords: hx,hy = upper left corner
	     //               lx,ly = lower right corner

  Linklist widgets; ///< a console can hold multiple widgets

  SLangWidget *focused; ///< only the focused widget receives the key


};

#endif

