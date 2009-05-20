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
#include <console_readline_ctrl.h>

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
static bool keyboard_quit = false;
static void sigint_handler (int sig) {
  SLsignal_intr (SIGINT, sigint_handler);
  keyboard_quit = true;
  func("%s : keyboard quit", __PRETTY_FUNCTION__);
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


Console::Console() 
  : Controller() {
  env=NULL;
  active = false;
  paramsel = 1;

  slw = NULL;
  sel = NULL;
  log = NULL;
  tit = NULL;
  rdl = NULL;

  // the console might be the main controller used in debugging
  // more often than in other cases, so we make it indestructible
  // by default, meaning that reset() won't delete it.
  indestructible = true;

  set_name("Console");
}

Console::~Console() {
  set_console(NULL);
  SLtt_set_cursor_visibility(1);

  if(sel) delete sel;
  if(log) delete log;
  if(tit) delete tit;
  if(rdl) delete rdl;
  if(slw) delete slw;

}

bool Console::init(Context *freej) {
  ::func("%s",__PRETTY_FUNCTION__);
  env = freej;

  slw = new SLangConsole();
  slw->init();

  /** register WINdow CHange signal handler (TODO) */
  SLsignal (SIGWINCH, sigwinch_handler);

  /** register SIGINT signal */
  signal(SIGINT, sigint_handler);
  SLang_set_abort_signal(sigint_handler);

  SLkp_set_getkey_function(getkey_handler);

  SLtt_set_cursor_visibility(0);

  // title
  tit = new SlwTitle();
  tit->set_name("console title");
  tit->env = freej;
  slw->place(tit, 0, 0, slw->w, 2);
  tit->init();
  /////////////////////////////////

  // layer and filter selector
  sel = new SlwSelector();
  sel->set_name("layer & filter selector");
  sel->env = freej;
  slw->place(sel, 0, 2, slw->w, 8);
  sel->init();
  ////////////////////////////

  // log scroller
  log = new SLW_Log();
  log->set_name("console log messages");
  slw->place(log, 0, 10, slw->w, slw->h -3);
  log->init();
  ////////////////////////////


  // status line
  rdl = new SlwReadline();
  rdl->set_name("console readline");
  rdl->env = freej;
  slw->place(rdl, 0, slw->h-1, slw->w, slw->h);
  rdl->init();
  ////////////////////////////

  set_console(this);

  refresh();

  initialized = true;
  active = true;
  return true;
}



int Console::dispatch() {
  int key = SLkp_getkey();
  
//  if(key) ::func("SLkd_getkey: %u",key);
//  else return; /* return if key is zero */
  if(!key) return(0);

  if( key==KEY_CTRL_L) {
    tit->blank();
    log->blank();
    sel->blank();
    rdl->blank();
  }

  if( rdl->feed(key) ) return(1);

  if( log->feed(key) ) return(1);
  
  if( sel->feed(key) ) return(1);
  


  return(0);
}

int Console::poll() {
  if(keyboard_quit) {
    rdl->readline("do you really want to quit? type yes to confirm:",&quit_proc,NULL);
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
  refresh();

  return(1);
}

void Console::refresh() {
  tit->refresh();
  log->refresh();
  sel->refresh();
  rdl->refresh();
  slw->refresh();

//   SLsmg_cls();
//   canvas();
//   speedmeter();
//   //  update_scroll();
//   if(!commandline)
//     statusline(NULL);
//   else
//     GOTO_CURSOR;

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
