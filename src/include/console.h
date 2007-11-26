/*  FreeJ - S-Lang console
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * $Id$
 *
 */

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <linklist.h>
#include <slang.h>

#define MAX_CMDLINE 256

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

/* callback functions for console input modes */
typedef int (cmd_process_t)(char *cmd);
typedef int (cmd_complete_t)(char *cmd);

extern volatile int SLang_Error; // hack for fucking debian!

class Context;
class Layer;
class FilterInstance;


class Console {
 public:
  
  Console();
  ~Console();
  
  bool init(Context *freej);
  void close();
  void cafudda();

  void notice(char *msg);
  void error(char *msg);
  void warning(char *msg);
  void act(char *msg);
  void func(char *msg);
  
  /* takes a pointer to the function which will be
     in charge of processing the input collected */
  int readline(char *msg, cmd_process_t *proc, cmd_complete_t *comp);  

  void refresh();

  bool active;

  Linklist history;

 private:
  int x,y;

  void canvas();

  void layerprint();
  void layerlist();
  int layercol;

  void filterprint();
  void filterlist();
  int paramsel;

  void speedmeter();

  void statusline(char *msg);

  void print_help();

  void getkey();

  void scroll(char *msg,int color);
  void update_scroll();
  bool do_update_scroll;

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
  void parser_jazz(int key);

  int movestep;
  int jazzstep;
  int jazzvalue;

  /* The SLscroll routines will use this structure. */
  void free_lines(File_Line_Type *line);
  File_Line_Type *create_line(char *buf);
  File_Line_Type *File_Lines;  
  SLscroll_Window_Type Line_Window;
  File_Line_Type *line, *last_line;
  unsigned int num_lines;

  Entry *entr;
};



#endif
