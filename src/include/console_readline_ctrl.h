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

#ifndef __CONSOLE_READLINE_H__
#define __CONSOLE_READLINE_H__

#include <slw.h>

#define MAX_CMDLINE 256

/* callback functions for console input modes */
typedef int (cmd_process_t)(Context *env, char *cmd);
typedef int (cmd_complete_t)(Context *env, char *cmd);

class Context;

class SlwReadline : public SLangWidget {
 public:
  SlwReadline();
  ~SlwReadline();

  bool init();
  bool feed(int key);
  bool refresh();
  
  enum parser_t { DEFAULT,
		  COMMANDLINE,
		  MOVELAYER } parser; // which parser to use for keys
  void set_parser(parser_t parser);

  /* takes a pointer to the function which will be
     in charge of processing the input collected */
  int readline(const char *msg, cmd_process_t *proc, cmd_complete_t *comp);  

  int movestep;

  Context *env;



 private:

  Linklist<Entry> history;
  
  /* input console command */
  bool commandline;
  int cursor;
  char command[MAX_CMDLINE];
  cmd_process_t *cmd_process;
  cmd_complete_t *cmd_complete;

  bool parser_default(int key);
  bool parser_commandline(int key);
  bool parser_movelayer(int key);
  
};

#endif

