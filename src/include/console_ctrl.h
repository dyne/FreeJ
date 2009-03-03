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

#define MAX_CMDLINE 256


#define PLAIN_COLOR 1
#define TITLE_COLOR 1
#define LAYERS_COLOR 3
#define FILTERS_COLOR 4
#define SCROLL_COLOR 5


// just comfortable
#define GOTO_CURSOR \
      SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor+1)


/* TODO port to slang 2
#if SLANG_VERSION >= 20000
#define SLANG_ERROR()  SLang_get_error()
#else
#define SLANG_ERROR()  SLang_Error
#endif
*/

/* The SLscroll routines will use this structure. */
typedef struct _File_Line_Type {
  struct _File_Line_Type *next;
  struct _File_Line_Type *prev;
  char *data;			       /* pointer to line data */
  int color; // line color
} File_Line_Type;


extern volatile int SLang_Error; // hack for fucking debian!

class Context;
class Layer;
class FilterInstance;

class SLangConsole;
class SLW_Log;
class SlwSelector;

/* callback functions for console input modes */
typedef int (cmd_process_t)(Context *env, char *cmd);
typedef int (cmd_complete_t)(Context *env, char *cmd);


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
  
  /* takes a pointer to the function which will be
     in charge of processing the input collected */
  int readline(const char *msg, cmd_process_t *proc, cmd_complete_t *comp);  

  void refresh();

  bool active;

  Linklist<Entry> history;

 private:

  SLangConsole *slw;
  SlwSelector *sel;
  SLW_Log *log;

  int x,y;

  void canvas();

  int paramsel;

  void speedmeter();

  void statusline(char *msg);

  void print_help();


  void scroll(const char *msg,int color);

  int movestep;

  /* input console command */
  bool commandline;
  int cursor;
  char command[MAX_CMDLINE];
  cmd_process_t *cmd_process;
  cmd_complete_t *cmd_complete;

  enum parser_t { DEFAULT,
		  COMMANDLINE,
		  MOVELAYER,
		  JAZZ } parser; // which parser to use for keys

  void parser_default(int key);
  void parser_commandline(int key);
  void parser_movelayer(int key);

};



#endif
