/*  FreeJ - S-Lang console
 *
 *  (c) Copyright 2004-2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <controller.h>
#include <linklist.h>
#include <slang.h>



#define PLAIN_COLOR 1
#define TITLE_COLOR 1
#define LAYERS_COLOR 3
#define FILTERS_COLOR 4
#define SCROLL_COLOR 5



/* TODO port to slang 2
#if SLANG_VERSION >= 20000
#define SLANG_ERROR()  SLang_get_error()
#else
#define SLANG_ERROR()  SLang_Error
#endif
*/

extern volatile int SLang_Error; // hack for fucking debian!

class Context;
class Layer;
class FilterInstance;

class SLangConsole;
class SLW_Log;
class SlwSelector;
class SlwTitle;
class SlwReadline;




class Console: public Controller {
 public:
  
  Console();
  ~Console();
  
  bool init(Context *freej);
  int poll();
  int dispatch();

  void close();
  void cafudda();

  void notice(const char *msg);
  void error(const char *msg);
  void warning(const char *msg);
  void act(const char *msg);
  void func(const char *msg);
  

  void refresh();

  bool active;


 private:

  SLangConsole *slw;
  SlwSelector *sel;
  SlwTitle *tit;
  SLW_Log *log;
  SlwReadline *rdl;

  int x,y;

  int paramsel;

  int movestep;


};



#endif
