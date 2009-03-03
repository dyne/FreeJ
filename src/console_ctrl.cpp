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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <signal.h>

#include <config.h>

#include <slang.h>

#include <slw_log.h>

#include <console_ctrl.h>
#include <console_calls_ctrl.h>
#include <console_widgets_ctrl.h>

#include <slw_console.h>
#include <keycodes.h>

#include <context.h>
#include <blitter.h>

#include <fps.h>


#include <jutils.h>


#include <gen_layer.h>
#include <gen_f0r_layer.h>



static bool screen_size_changed;
static void sigwinch_handler (int sig) {
  screen_size_changed = true;
  SLsignal (SIGWINCH, sigwinch_handler);
}

static bool real_quit;
static bool keyboard_quit;
static void sigint_handler (int sig) {
  SLsignal_intr (SIGINT, sigint_handler);
  keyboard_quit = true;
#if SLANG_VERSION < 20000
  if (SLang_Ignore_User_Abort == 0) 
	  SLang_Error = USER_BREAK;
#endif
}

/* non blocking getkey */
static int getkey_handler() {
  unsigned int ch = 0;
  if(SLang_input_pending(0))
    //    return SLang_getkey();
    ch = SLang_getkey();

  //  SLang_flush_input(); // no slow repeat

  //  if(ch) func("SLang_getkey in getkey_handler detected char %u",ch);
  return ch;
}

// confirm quit
int quit_proc(Context *env, char *cmd) {
  if(!cmd) return 0;
  if(cmd[0]=='y') {
    real_quit = true;
    return 1; }
  real_quit = false;
  return 0;
}


Console::Console() {
  env=NULL;
  movestep=2;
  commandline = false;
  parser = DEFAULT;
  active = false;
  paramsel = 1;
}

Console::~Console() {
  close();
}

bool Console::init(Context *freej) {
  ::func("%s",__PRETTY_FUNCTION__);
  env = freej;

  slw = new SLangConsole();
  slw->init();

  SLsignal (SIGWINCH, sigwinch_handler);
  SLang_set_abort_signal(sigint_handler);
  SLkp_set_getkey_function(getkey_handler);


  canvas();

  SLtt_set_cursor_visibility(0);

  // layer and filter selector
  sel = new SlwSelector();
  sel->set_name("layer & filter selector");
  sel->env = freej;
  slw->place(sel, 0, 1, slw->w, 8);
  sel->init();
  ////////////////////////////

  // log scroller
  log = new SLW_Log();
  log->set_name("console log messages");
  slw->place(log, 0, 9, slw->w, slw->h -3);
  log->init();
  ////////////////////////////

  set_console(this);

  print_help();

  slw->refresh();

  active = true;
  return true;
}

void Console::close() {
  SLtt_set_cursor_visibility(1);
  set_console(NULL);
  slw->close();
}

/* setup the flags and environment to read a new input
   saves the pointer to the command processing function
   to use it once the input is completed */
int Console::readline(const char *msg,cmd_process_t *proc,cmd_complete_t *comp) {
  ::notice(msg);
  //  update_scroll();
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  SLsmg_write_string((char *)":");
  SLsmg_erase_eol();
  
  cursor = 0;
  memset(command,EOL,MAX_CMDLINE);

  SLtt_set_cursor_visibility(1);
  cmd_process = proc;
  cmd_complete = comp;
  
  commandline = true;
  parser = COMMANDLINE;
  
  return 1;
}


int Console::dispatch() {
  int key = SLkp_getkey();
  
//  if(key) ::func("SLkd_getkey: %u",key);
//  else return; /* return if key is zero */
  if(!key) return 0;

  //  if(input) {
  if(parser == COMMANDLINE) parser_commandline(key);
  else if(parser == MOVELAYER) parser_movelayer(key);
  //  else if(parser == JAZZ) parser_jazz(key);
  else parser_default(key);
  
  return 1;
}

int Console::poll() {

  if(keyboard_quit) {
    readline("do you really want to quit? type yes to confirm:",&quit_proc,NULL);
    keyboard_quit = false;
    return 0;
  }

  if(real_quit) {
    notice("QUIT requested from console! bye bye");
    env->quit = true;
    real_quit = false;
    return 0;
  }   

  dispatch();

  // refresh all widgets
  slw->refresh();

  return(1);
}

void Console::refresh() {
//   SLsmg_cls();
//   canvas();
//   speedmeter();
//   //  update_scroll();
//   if(!commandline)
//     statusline(NULL);
//   else
//     GOTO_CURSOR;

}    

void Console::print_help() {
  notice("Hotkeys available in FreeJ console:");
  act("ctrl+o  = Open a Layer (will prompt for path to file)");
  act("Arrow keys browse selection thru layers and effects");
  act("SPACE to de/activate layers and filters selected");
  act("ENTER to start/stop layers selected");
  act("+ and - move filters and effects thru chains");
  act(" ! = Switch on/off On Screen Display information");
  act(" @ = Switch on/off screen cleanup after every frame");
  act("ctrl+e  = Add a Filter to a Layer");
  act("ctrl+b  = Change the Blit for a Layer");
  act("ctrl+v  = Fade the Blit Value for a Layer");
  act("ctrl+a  = Move Rotate and Zoom a Layer"); 
  act("ctrl+p  = Set Parameters for a Layer or Filter");
  act("ctrl+t  = Add a new Text layer (will prompt for text)");
  act("ctrl+x  = execute a javascript command");
  act("ctrl+j  = load and execute a script file");
  act("ctrl+l  = Cleanup and redraw the console");
  act("ctrl+f  = Go to Fullscreen");
  act("ctrl+c  = Quit FreeJ");
}

void Console::statusline(char *msg) {
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);

  if(msg) {
    SLsmg_write_string(msg);
    SLsmg_erase_eol();
  } else
    SLsmg_write_string
      ((char *)" use arrows to move selection, press ctrl-h for help with hotkeys      ");

  SLsmg_set_color(PLAIN_COLOR);
}


void Console::speedmeter() {
  char tmp[256];
  int fps = env->fps->get();
  SLsmg_gotorc(1,1);
  sprintf(tmp,"Running at %u fps : ",fps);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_write_string(tmp);
  if(fps <10) {
    SLsmg_set_color(12);
    SLsmg_write_string("very slow ");
  } else if(fps < 24) {
    SLsmg_set_color(2);
    SLsmg_write_string("slow ");
  } else if(fps < 30) {
    SLsmg_set_color(14);
    SLsmg_write_string("ok ");
  } else if(fps > 30) {
    SLsmg_set_color(3);
    SLsmg_write_string("smooth ");
  } else if(fps > 40) {
    SLsmg_set_color(13);
    SLsmg_write_string("fast ");
  } else if(fps > 50) {
    SLsmg_set_color(13);
    SLsmg_write_string("very fast ");
  }
  SLsmg_draw_hline((int)fps);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_erase_eol();

}

void Console::canvas() {
  SLsmg_gotorc(0,0);
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_printf((char *)" %s version %s | set the veejay free! | freej.dyne.org | ",
	       PACKAGE, VERSION);
  
  /* this is RASTA SOFTWARE! */
  SLsmg_set_color(32);
  SLsmg_write_string((char *)"RAS");
  SLsmg_set_color(34);
  SLsmg_write_string((char *)"TAS");
  SLsmg_set_color(33);
  SLsmg_write_string((char *)"OFT");

  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_gotorc(SLtt_Screen_Rows - 2,0);
  SLsmg_draw_hline(72);
}



void Console::notice(const char *msg) {
  log->append(msg);
}
void Console::warning(const char *msg) {
  log->append(msg);
}
void Console::act(const char *msg) {
  log->append(msg);
}
void Console::error(const char *msg) {
  log->append(msg);
}
void Console::func(const char *msg) {
  log->append(msg);
}
