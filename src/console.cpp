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

#include <signal.h>
#include <slang.h>
#include <context.h>
#include <jsparser.h>
#include <jutils.h>
#include <config.h>

#define PLAIN_COLOR 1
#define TITLE_COLOR 1
#define LAYERS_COLOR 3
#define FILTERS_COLOR 7
#define SCROLL_COLOR 5

#define EOL '\0'

#define KEY_ENTER 13
#define KEY_BACKSPACE 275
#define KEY_LEFT 259
#define KEY_RIGHT 260
#define KEY_HOME 263
#define KEY_DELETE 275
#define KEY_TAB 9
/* unix ctrl- commandline hotkeys */
#define KEY_CTRL_A 1 // goto beginning of line
#define KEY_CTRL_B 2
#define KEY_CTRL_E 5 // goto end of line
#define KEY_CTRL_F 6
#define KEY_CTRL_K 11 // delete until end of line
#define KEY_CTRL_D 4 // delete char
#define KEY_CTRL_U 21 // delete until beginning of line
#define KEY_CTRL_H 272 // help the user
#define KEY_CTRL_V 22 // shout it loud!

static Context *env;

extern const char *layers_description;

static bool screen_size_changed;
static void sigwinch_handler (int sig) {
  screen_size_changed = true;
  SLsignal (SIGWINCH, sigwinch_handler);
}

static bool keyboard_quit;
static void sigint_handler (int sig) {
  SLsignal_intr (SIGINT, sigint_handler);
  keyboard_quit = true;
  if (SLang_Ignore_User_Abort == 0) SLang_Error = USER_BREAK;
}

/* non blocking getkey */
static int getkey_handler() {
  unsigned int ch = 0;
  if(SLang_input_pending(0))
    //    return SLang_getkey();
    ch = SLang_getkey();
  //  if(ch) func("SLang_getkey in getkey_handler detected char %u",ch);
  return ch;
}

static int js_proc(char *cmd) {
  int res = 0;
  if(!cmd) return res;
#ifdef WITH_JAVASCRIPT
  res = env->js->parse(cmd);
  if(!res) ::error("invalid javascript command: %s",cmd);
#endif
  return res;
}


// used to handle completed input from console
static int blit_selection(char *cmd) {
  if(!cmd) return 0;
  if(!strlen(cmd)) return 0;

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  lay->set_blit(cmd); // now this takes a string!
  return 1;
}
static int blit_comp(char *cmd) {
  int c;
  int *blits;
  Blit *b;

  if(!cmd) return 0;

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }

  blits = lay->blitter.blitlist.completion(cmd);
  if(!blits) return 0;
  if(!blits[0]) return 0;
  func("porcodio %i",blits[0]);
  for(c=0;blits[c];c++) {
    b = (Blit*) lay->blitter.blitlist.pick(blits[c]);

    if(!b) {
      func("error in completion: missing %i");
      continue;
    }
    ::act("%s :: %s",b->name,b->desc);
  }

  if(c==1) // exact match, then fill in command
    snprintf(cmd,511,"%s",b->name);

  return c;
}


static int filter_proc(char *cmd) {
  Filter *filt;
  if(!cmd) return 0;
  filt = env->plugger.pick(cmd);
  if(!filt) {
    ::error("filter not found: %s",cmd);  
    return 0;
  }
  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer selected for effect %s",filt->getname());
    return 0;
  }
  if(!filt->init(&lay->geo)) {
    ::error("Filter %s can't initialize",filt->getname());
    return 0;
  }
  lay->filters.add(filt);
  // select automatically the new filter
  lay->filters.sel(0);
  filt->sel(true);
  return 1;
}
static int filter_comp(char *cmd) {
  int c;
  char **res;
  Filter *filt;
  if(!cmd) return 0;
  res = env->plugger.complete(cmd);
  if(!res) return 0;
  if(!res[0]) return 0;
  for(c=0;res[c];c++) {
    filt = env->plugger.pick(res[c]);
    if(!filt) continue;
    ::act("%s :: %s",filt->getname(),filt->getinfo());
  }

  if(c==1) // exact match, then fill in command
    snprintf(cmd,511,"%s",res[0]);

  free(res);
  return c;
}

Console::Console() {
  env=NULL;
  last_line=NULL;
  num_lines=0;
  input = false;
  do_update_scroll=true;
  active = false;
  layer = NULL;
  filter = NULL;
}

Console::~Console() {
  close();
}

bool Console::init(Context *freej) {
  env = freej;

  SLtt_get_terminfo();

  SLkp_init(); // keyboard interface

  SLang_init_tty(-1,0,0);

  SLsmg_init_smg(); // screen manager

  /* setup colors with the palette scheme:
     n = normal;
     n+10 = highlight;
     n+20 = reverse normal;
     n+30 = reverse highlight; */

  SLtt_set_color(1,NULL,"lightgray","black");
  SLtt_set_color(11,NULL,"white","black");
  SLtt_set_color(21,NULL,"black","lightgray");
  SLtt_set_color(31,NULL,"black","white");

  SLtt_set_color(2,NULL,"red","black");
  SLtt_set_color(12,NULL,"brightred","black");
  SLtt_set_color(22,NULL,"black","red");
  SLtt_set_color(32,NULL,"black","brightred");

  SLtt_set_color(3,NULL,"green","black");
  SLtt_set_color(13,NULL,"brightgreen","black");
  SLtt_set_color(23,NULL,"black","green");
  SLtt_set_color(33,NULL,"black","brightgreen");

  SLtt_set_color(4,NULL,"brown","black");
  SLtt_set_color(14,NULL,"yellow","black");
  SLtt_set_color(24,NULL,"black","brown");
  SLtt_set_color(34,NULL,"black","yellow");

  SLtt_set_color(5,NULL,"blue","black");
  SLtt_set_color(15,NULL,"brightblue","black");
  SLtt_set_color(25,NULL,"black","blue");
  SLtt_set_color(35,NULL,"black","brightblue");

  SLtt_set_color(6,NULL,"magenta","black");
  SLtt_set_color(16,NULL,"brightmagenta","black");
  SLtt_set_color(26,NULL,"black","magenta");
  SLtt_set_color(36,NULL,"black","brightmagenta");

  SLtt_set_color(7,NULL,"cyan","black");
  SLtt_set_color(17,NULL,"brightcyan","black");
  SLtt_set_color(27,NULL,"black","cyan");
  SLtt_set_color(37,NULL,"black","brightcyan");

  set_console(this);

  canvas();

  screen_size_changed = false;
  SLsignal (SIGWINCH, sigwinch_handler);
  SLang_set_abort_signal(sigint_handler);

  SLkp_set_getkey_function(getkey_handler);

  SLtt_set_cursor_visibility(0);

  env->track_fps = true; // we wanna know!

  active = true;
  return true;
}

void Console::close() {
  set_console(NULL);
  SLsmg_reset_smg();  
  SLang_reset_tty();
}

/* setup the flags and environment to read a new input
   saves the pointer to the command processing function
   to use it once the input is completed */
int Console::readline(char *msg,cmd_process_t *proc,cmd_complete_t *comp) {
  ::notice(msg);
  update_scroll();
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  SLsmg_write_string(":");
  SLsmg_erase_eol();
  
  cursor = 0;
  memset(command,EOL,512);
  input = true;
  SLtt_set_cursor_visibility(1);
  cmd_process = proc;
  cmd_complete = comp;
  return 1;
}

#define GOTO_CURSOR \
      SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor+1)

void Console::getkey() {
  int res,c;
  int key = SLkp_getkey();
  
  if(key) ::func("SLkd_getkey: %u",key);
  else return; /* return if key is zero */

  if(input) {
    /* =============== console command input */
    if(cursor>512) {
      error("command too long, can't type more.");
      return;
    }
    //::func("input key: %i",key);
    SLsmg_set_color(PLAIN_COLOR);

    switch(key) {
      
    case SL_KEY_ENTER:
    case KEY_ENTER:
      (*cmd_process)(command);
      input = false;
      cmd_process = NULL;
      cmd_complete = NULL;
      statusline();
      return;

    case KEY_TAB:
      if(!cmd_complete) return;
      res = (*cmd_complete)(command);
      if(!res) return;
      else if(res==1) { // exact match!
	SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
	SLsmg_write_string(command);
	SLsmg_erase_eol();
	cursor = strlen(command);
      }
      update_scroll();
      GOTO_CURSOR;
      return;

      /*** FIXME */
    case KEY_BACKSPACE:
      ::func("BACKSPACE");
      if(!cursor) return;
      cursor--;
      for(c=cursor;command[c]!=EOL;c++)
	command[c] = command[c+1];
      if(c==cursor) {
	cursor--; command[cursor] = EOL;
	GOTO_CURSOR;
      } else {
	SLsmg_write_string(&command[cursor]);
      }
      SLsmg_erase_eol();
      GOTO_CURSOR;
      return;

    case KEY_CTRL_D:
      for(c=cursor;command[c]!=EOL;c++)
	command[c] = command[c+1];
      GOTO_CURSOR;
      SLsmg_write_string(&command[cursor]);
      SLsmg_erase_eol();
      GOTO_CURSOR;
      return;

    case SL_KEY_LEFT:
      if(cursor) cursor--;
      GOTO_CURSOR;
      return;
    case SL_KEY_RIGHT:
      if(command[cursor]) cursor++;
      GOTO_CURSOR;
      return;
      
    case KEY_CTRL_A:
    case KEY_HOME:
      cursor=0;
      GOTO_CURSOR;
      return;
    case KEY_CTRL_E:
      while(command[cursor]!=EOL) cursor++;
      GOTO_CURSOR;
      return;
    case KEY_CTRL_K:
      for(c=cursor;command[c]!=EOL;c++)
	command[c] = EOL;
      GOTO_CURSOR;
      SLsmg_erase_eol();
      return;
    case KEY_CTRL_U:
      for(c=0;command[cursor+c]!=EOL;c++)
	command[c] = command[cursor+c];
      for(;command[c]!=EOL;c++)
	command[c] = EOL;
      cursor=0;
      GOTO_CURSOR;
      SLsmg_write_string(&command[cursor]);
      SLsmg_erase_eol();
      GOTO_CURSOR;
      return;

    }
    /* add char at cursor position
       insert mode
       FIX ME! */
    for(c=cursor;command[c+1]!=EOL;c++)
      command[c+1] = command[c];
    command[cursor] = key;
    
    GOTO_CURSOR;
    SLsmg_write_string(&command[cursor]);
    SLsmg_erase_eol();
    cursor++;
    GOTO_CURSOR;

  } else { /* ======== hotkey input */

    switch(key) {
    case SL_KEY_UP:
      if(filter)
	filter = (Filter*)filter->prev;
      break;
    case SL_KEY_DOWN:
      if(!filter) {
	if(!layer) break;
	filter = (Filter*)layer->filters.begin();
	break;
      }
      if(!filter->next) break;
      filter = (Filter*)filter->next;
      break;

    case SL_KEY_LEFT:
      if(!layer) {
	layer = (Layer*)env->layers.end();
	break;
      }
      if(!layer->prev) break;
      layer = (Layer*)layer->prev;
      break;
    case SL_KEY_RIGHT:
      if(!layer) {
	layer = (Layer*)env->layers.begin();
	break;
      }
      if(!layer->next) break;
      layer = (Layer*)layer->next;
      break;

    case SL_KEY_PPAGE: break;
    case SL_KEY_NPAGE: break;
    case SL_KEY_END: break;

    case SL_KEY_BACKSPACE:      
    case SL_KEY_DELETE:
      if(!filter) break;
      filter->rem();
      filter->clean();
      filter = NULL;
    break;

    case KEY_CTRL_E:
      readline("add new Effect - press TAB for completion",&filter_proc,&filter_comp);
      break;
      
    case KEY_CTRL_B:
      readline("select Blit mode - press TAB for completion",
	       &blit_selection,&blit_comp);
      break;

    case ':':
      if(!env->js) {
	::error("javascript is not compiled in this FreeJ binary");
	return;
      }
      /* cleans up the status and sets input = true
	 if(input==true) then keys are treated as string input */
      readline("input javascript command:",&js_proc,NULL);
      break;

    case KEY_CTRL_V:
      ::notice("Welcome to %s %s",PACKAGE,VERSION);
    :: act("layers supported:\n%s",layers_description);
    break;
    }
  }
}

void Console::cafudda() {

  getkey(); // get pending keyboard input

  if(keyboard_quit) {
    env->quit = true;
    return;
  }

  /* S-Lang says: 
   * All well behaved applications should block signals that may affect
   * the display while performing screen update. */
  SLsig_block_signals ();
  
  if(screen_size_changed) {
    SLtt_get_screen_size ();
    SLsmg_reinit_smg ();
    canvas();
    screen_size_changed = false;
  }

  /* print info the selected layer */
  layerprint(); // updates the *layer selected pointer
  layerlist(); // print layer list   
    
  filterprint(); // updates the *filter selected pointer
  filterlist(); // print filter list
  
  if(do_update_scroll)
    update_scroll();
  
  speedmeter();

  if(!input) statusline();
  
  SLsmg_refresh();
  
  SLsig_unblock_signals ();
}

void Console::statusline() {
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  SLsmg_write_string("ctrl-h help | : command input | running on ");
  SLsmg_write_string(SLcurrent_time_string());
  SLsmg_set_color(PLAIN_COLOR);
}

void Console::speedmeter() {
  SLsmg_gotorc(1,1);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_write_string("Running: ");
  if(env->fps <10) {
    SLsmg_set_color(12);
    SLsmg_write_string("very slow ");
  } else if(env->fps < 24) {
    SLsmg_set_color(2);
    SLsmg_write_string("slow ");
  } else if(env->fps < 30) {
    SLsmg_set_color(14);
    SLsmg_write_string("ok ");
  } else if(env->fps > 30) {
    SLsmg_set_color(3);
    SLsmg_write_string("smooth ");
  } else if(env->fps > 50) {
    SLsmg_set_color(13);
    SLsmg_write_string("fast ");
  } else if(env->fps > 50) {
    SLsmg_set_color(13);
    SLsmg_write_string("very fast ");
  }
  SLsmg_draw_hline((int)env->fps);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_erase_eol();

}

void Console::canvas() {
  SLsmg_gotorc(0,0);
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_printf(" %s version %s | set the veejay free! | freej.dyne.org | ",
	       PACKAGE, VERSION);
  
  /* this is RASTA SOFTWARE! */
  SLsmg_set_color(32);
  SLsmg_write_string("RAS");
  SLsmg_set_color(34);
  SLsmg_write_string("TAS");
  SLsmg_set_color(33);
  SLsmg_write_string("OFT");


  //  SLsmg_gotorc(14,0);
  //  SLsmg_draw_hline(72);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_gotorc(SLtt_Screen_Rows - 2,0);
  SLsmg_draw_hline(72);
}

void Console::layerprint() {
  if(!layer) return;

  SLsmg_gotorc(2,1);
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_string("Layer: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_write_string(layer->get_filename());
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_char(' ');
  SLsmg_write_string("blit: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_write_string(layer->get_blit());
  SLsmg_write_char(' ');
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_char(' ');
  SLsmg_write_string("geometry: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_printf("x%i y%i w%u h%u",
	       layer->geo.x, layer->geo.y,
		 layer->geo.w, layer->geo.h);
  SLsmg_erase_eol();
}

void Console::layerlist() {
  int color;
  SLsmg_gotorc(4,1);
  env->layers.lock();
  /* take layer selected and first */
  Layer *l = (Layer *)env->layers.begin();
  
  while(l) { /*draw the layer's list */

    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_string(" -> ");

    color=LAYERS_COLOR;
    if( l == layer) {
      color+=20;
      layercol = SLsmg_get_column();
    }
    if( l->active) color+=10;
    SLsmg_set_color (color);
    
    SLsmg_printf("%s",l->get_name());

    l = (Layer *)l->next;
  }
  env->layers.unlock();
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_erase_eol();
}

void Console::filterprint() {
  if(!layer) return;
  if(!filter) return;

  SLsmg_gotorc(3,1);
  SLsmg_set_color(FILTERS_COLOR);
  SLsmg_write_string("Filter: ");
  if(!filter) {
    SLsmg_write_string("none");
    SLsmg_set_color(PLAIN_COLOR);
    SLsmg_erase_eol();
    return;
  }
  SLsmg_set_color(FILTERS_COLOR+10);
  SLsmg_write_string(filter->getname());
  SLsmg_erase_eol();
  SLsmg_forward(2);
  SLsmg_write_string(filter->getinfo());
  SLsmg_set_color(PLAIN_COLOR);
}

void Console::filterlist() {
  int color;
  int pos = 5;
  
  if(layer && filter) {

    layer->filters.lock();
    Filter *f = (Filter *)layer->filters.begin();
    while(f) {

      SLsmg_set_color(PLAIN_COLOR);
      SLsmg_gotorc(pos,0);
      SLsmg_erase_eol();
      
      SLsmg_gotorc(pos,layercol);
      color=FILTERS_COLOR;
      if( f == filter) color+=20;
      if( f->active) color+=10;
      SLsmg_set_color (color);
      
      SLsmg_printf("%s",f->getname());
      
      pos++;
      f = (Filter *)f->next;
    }
    layer->filters.unlock();
  }
  SLsmg_set_color(PLAIN_COLOR);
  for(;pos<14;pos++) {
    SLsmg_gotorc(pos,0);
    SLsmg_erase_eol();
  }
}



void Console::notice(char *msg) {
  scroll(msg,PLAIN_COLOR+10);
}
void Console::warning(char *msg) {
  scroll(msg,2);
}
void Console::act(char *msg) {
  scroll(msg,PLAIN_COLOR);
}
void Console::error(char *msg) {
  scroll(msg,2);
}
void Console::func(char *msg) {
  scroll(msg,5);
  update_scroll();
}

void Console::free_lines (void)
{
   File_Line_Type *line, *next;
   
   line = File_Lines;
   while (line != NULL)
     {
	next = line->next;
	if (line->data != NULL) free (line->data);
	free (line);
	line = next;
     }
   File_Lines = NULL;
}

File_Line_Type *Console::create_line (char *buf)
{
   File_Line_Type *line;
   
   line = (File_Line_Type *) malloc (sizeof (File_Line_Type));
   if (line == NULL) return NULL;
   
   memset ((char *) line, sizeof (File_Line_Type), 0);
   
   line->data = SLmake_string (buf);   /* use a slang routine */
   if (line->data == NULL)
     {
	free (line);
	return NULL;
     }
   
   return line;
}


void Console::scroll(char *msg, int color) {

  line = create_line(msg);
  line->color = color;

  if (!last_line)
    File_Lines = line;
  else 
    last_line->next = line;
  
  line->prev = last_line;
  line->next = NULL;
  
  last_line = line;
  num_lines++;
   
  memset ((char *)&Line_Window, 0, sizeof (SLscroll_Window_Type));
  
  Line_Window.current_line = (SLscroll_Type *) File_Lines;
  Line_Window.current_line = (SLscroll_Type *) last_line;
  Line_Window.lines = (SLscroll_Type *) File_Lines;
  Line_Window.line_num = 1;
  Line_Window.num_lines = num_lines;
  //  Line_Window.border = 3;
  do_update_scroll = true;
}

void Console::update_scroll() {
  unsigned int row, col, nrows;
  row = 14; // first upper row
  col = 1; // left bound

  Line_Window.nrows = nrows = SLtt_Screen_Rows - 3;
  
  /* Always make the current line equal to the top window line. */
  //  if (Line_Window.top_window_line != NULL)
  //    Line_Window.current_line = Line_Window.top_window_line;
  
  //  SLscroll_find_top (&Line_Window);
  //  SLscroll_find_line_num (&Line_Window);
  
  //  line = (File_Line_Type *) Line_Window.top_window_line;
  line = last_line;
  if (!line) return;

  for(; nrows>row; nrows--) {
    if (!line) break;
    SLsmg_gotorc (nrows, col);    
    SLsmg_set_color(line->color);
    SLsmg_write_string (line->data);
    SLsmg_erase_eol ();
    line = line->prev;
  } 
  SLsmg_set_color(PLAIN_COLOR);

  do_update_scroll = false;
}
