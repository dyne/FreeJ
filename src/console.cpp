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
#define KEY_SPACE 32
#define KEY_BACKSPACE 275
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
#define KEY_CTRL_I 9 // OSD on/off
#define KEY_CTRL_K 11 // delete until end of line
#define KEY_CTRL_L 12 // refresh screen
#define KEY_CTRL_M 13 // move layer
#define KEY_CTRL_U 21 // delete until beginning of line
#define KEY_CTRL_H 272 // help the user
#define KEY_CTRL_J 10 // javascript command
#define KEY_CTRL_O 15 // open a file in a new layer
#define KEY_CTRL_V 22 // change blit value
#define KEY_CTRL_X 24

#define KEY_PLUS 43
#define KEY_MINUS 45

// just comfortable
#define GOTO_CURSOR \
      SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor+1)



static Context *env;

extern const char *layers_description;

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

#ifdef WITH_JAVASCRIPT
static int js_proc(char *cmd) {
  int res = 0;
  if(!cmd) return res;

  res = env->js->parse(cmd);
  if(!res) ::error("invalid javascript command: %s",cmd);
  return res;
}
#endif


// callbacks used by readline to handle input from console
static int blit_selection(char *cmd) {
  if(!cmd) return 0;
  if(!strlen(cmd)) return 0;

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  lay->blitter.set_blit(cmd); // now this takes a string!
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

  if(!blits[0]) return 0; // none found

  if(!blits[1]) { // exact match, then fill in command
    b = (Blit*) lay->blitter.blitlist.pick(blits[0]);
    ::notice("%s :: %s",b->get_name(),b->desc);
    snprintf(cmd,511,"%s",b->get_name());
    return 1;
  }
  
  notice("List available blits starting with \"%s\"",cmd);
  for(c=0;blits[c];c++) {
    b = (Blit*) lay->blitter.blitlist.pick(blits[c]);

    if(!b) {
      func("error in completion: missing %i");
      continue; }
    ::act("%s :: %s",b->get_name(),b->desc);
  }
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

  if(!res[0]) return 0; // no hit 

  if(!res[1]) { // exact match: fill in the command
    filt = env->plugger.pick(res[0]);
    if(!filt) return 0; // doublecheck safety fix
    ::notice("%s :: %s",filt->getname(),filt->getinfo());
    snprintf(cmd,511,"%s",res[0]); c=1;
  } else { // list all matches
    for(c=0;res[c];c++) {
      filt = env->plugger.pick(res[c]);
      if(!filt) continue;
      ::act("%s :: %s",filt->getname(),filt->getinfo());
    }
  }
  return c;
}
// confirm quit
static int quit_proc(char *cmd) {
  if(!cmd) return 0;
  if(cmd[0]=='y') {
    real_quit = true;
    return 1; }
  real_quit = false;
  return 0;
}

static int open_layer(char *cmd) {
  int len;
  Layer *l = create_layer(cmd);
  if(l)
    if(!l->init(env)) {
      error("can't initialize layer");
      delete l;
    } else {
      env->layers.add(l);
      len = env->layers.len();
      notice("layer succesfully created, now you have %i layers",len);
      env->console->refresh();
      return env->layers.len();
    }
  error("layer creation aborted");
  env->console->refresh();
  return 0;
}

static int filebrowse_completion(char *cmd) {
  func("filebrowser completion TODO");
  return(0);
}

static int set_blit_value(char *cmd) {
  int val;
  int c;
  if(!sscanf(cmd,"%u",&val)) {
    error("error parsing input: %s",cmd);
    return 0;
  }
  func("value parsed: %s in %d",cmd,val);
  Layer *lay = (Layer*)env->layers.begin();
  if(!lay) return 0;
  /* set value in all blits selected
     (supports multiple selection) */
  for(c=0 ; lay ; lay = (Layer*)lay->next) {
    if(!lay->select) continue;
    lay->blitter.fade_value(1,val);
  }

  return 1;
}

Console::Console() {
  env=NULL;
  last_line=NULL;
  num_lines=0;
  movestep=2;
  jazzstep=10;
  jazzvalue=0xff;
  commandline = false;
  parser = DEFAULT;
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

  SLtt_set_cursor_visibility(1);
  cmd_process = proc;
  cmd_complete = comp;
  
  commandline = true;
  parser = COMMANDLINE;
  
  return 1;
}


void Console::getkey() {
  int key = SLkp_getkey();
  
  if(key) ::func("SLkd_getkey: %u",key);
  else return; /* return if key is zero */

  //  if(input) {
  if(parser == COMMANDLINE) parser_commandline(key);
  else if(parser == MOVELAYER) parser_movelayer(key);
  else if(parser == JAZZ) parser_jazz(key);
  else parser_default(key);
    
}

void Console::cafudda() {

  getkey(); // get pending keyboard input

  if(keyboard_quit) {
    readline("do you really want to quit? type yes to confirm:",&quit_proc,NULL);
    keyboard_quit = false;
    return;
  }

  if(real_quit) {
    notice("QUIT requested from console! bye bye");
    env->quit = true;
    real_quit = false;
  }   

  if(!layer) // if no layer selected, pick the first
    layer = (Layer*)env->layers.begin();
  
  /* S-Lang says: 
   * All well behaved applications should block signals that
   * may affect the display while performing screen update. */
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

  if(!commandline) {
    speedmeter();
    statusline();
  } else
    GOTO_CURSOR;

  SLsmg_refresh();
  
  SLsig_unblock_signals ();
}

void Console::refresh() {
  SLsmg_cls();
  canvas();
  layerprint(); layerlist();
  filterprint(); filterlist();  
  update_scroll();
  if(!commandline)
    statusline();
  else
    GOTO_CURSOR;
}    

void Console::statusline() {
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  SLsmg_write_string
    (" use arrows to move selection, press ctrl-h for help with hotkeys      ");
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
  } else if(env->fps > 40) {
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
  SLsmg_write_string(layer->blitter.current_blit->get_name());
  SLsmg_write_char(' ');
  SLsmg_printf("[%u]",layer->blitter.current_blit->value);
  SLsmg_write_char(' ');
  SLsmg_set_color(LAYERS_COLOR);
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
  
  while(l) { /* draw the layer's list */

    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_string(" -> ");

    color=LAYERS_COLOR;
    if( l == layer && !filter) {
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
  Filter *f;
  int color;
  int pos = 5;
  
  if(layer) {

    layer->filters.lock();
    f = (Filter *)layer->filters.begin();
    while(f) {

      SLsmg_set_color(PLAIN_COLOR);
      SLsmg_gotorc(pos,0);
      SLsmg_erase_eol();
      
      SLsmg_gotorc(pos,layercol);
      color=FILTERS_COLOR;
      if( f == filter ) color+=20;
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

// delete all lines previous to this
// making it become the first (upper last in console)
void Console::free_lines (File_Line_Type *line)
{
   File_Line_Type *prev;
   int c;
   
   for(c=0 ; line ; c++) {
	prev = line->prev;
	if (line->data) free (line->data);
	free (line);
	line = prev;
   }
   Line_Window.num_lines -= c;
}

File_Line_Type *Console::create_line (char *buf)
{
   File_Line_Type *line;
   
   line = (File_Line_Type *) calloc (1, sizeof (File_Line_Type));

   if (!line) return NULL;
   
   line->data = SLmake_string (buf);   /* use a slang routine */
   if (!line->data) {
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
  // erase forgotten lines
  //  if(line) free_lines(line);

  SLsmg_set_color(PLAIN_COLOR);
  do_update_scroll = false;
  GOTO_CURSOR;
}




////////////////////////////////////////////
// KEY PARSERS
////////////////////////////////////////////


void Console::parser_default(int key) {
  
  commandline = false; // print statusline
  
  switch(key) {
  case SL_KEY_UP:
    if(!layer) break;
    if(!filter) break;
    filter = (Filter*)filter->prev;
    if(filter) {
      // select only the current
      layer->filters.sel(0);
      filter->sel(true);
    }
    break;

  case SL_KEY_DOWN:
    if(!filter) {
      if(!layer) break;
      filter = (Filter*)layer->filters.begin();
      break;
    }
    if(!filter->next) break;
    filter = (Filter*)filter->next;
    // select only the current
    layer->filters.sel(0);
    filter->sel(true);      
    break;

  case SL_KEY_LEFT:
    if(!layer) 
      layer = (Layer*)env->layers.begin();
    else if(!layer->prev)
      layer = (Layer*)env->layers.end();
    else
      layer = (Layer*)layer->prev;
    // select only the current
    env->layers.sel(0);
    layer->sel(true);
    break;

  case SL_KEY_RIGHT:
    if(!layer)
      layer = (Layer*)env->layers.begin();
    else if(!layer->next)
      layer = (Layer*)env->layers.begin();
    else
      layer = (Layer*)layer->next;
    // select only the current
    env->layers.sel(0);
    layer->sel(true);
    break;
      
  case SL_KEY_PPAGE:
    // move layer/filter selected up in chain
    if(filter)
      filter->up();
    else if(layer)
      layer->up();
    break;
      
  case SL_KEY_NPAGE:
    // move layer/filter selected up in chain
    if(filter)
      filter->down();
    else if(layer)
      layer->down();
    break;

  case SL_KEY_END: break;

  case SL_KEY_DELETE:
    if(filter) {
      filter->rem();
      filter->clean();
      filter = NULL;
    } else if(layer) {
      layer->rem();
      layer->close();
      layer = NULL;
    }
    break;
    
  case SL_KEY_IC:
    if(!filter) break;
    filter->active = !filter->active;
    break;

  case SL_KEY_HOME:
    if(!layer) break;
    layer->active = !layer->active;
    break;

  case KEY_CTRL_H:
    notice("Hotkeys available in FreeJ console:");
    act("arrow keys browse selection thru layers and effects");
    act("HOME de/activate layer, INS de/activates filter");
    act("ctrl+o  = Open new layer (will prompt for path to file)");
    act("ctrl+e  = add a new Effect to the selected layer");
    act("ctrl+b  = change the Blit for the selected layer");
    act("ctrl+v  = fade the Blit Value for the selected layer");
    act("ctrl+m  = move the selected layer around the screen");
    act("ctrl+j  = activate jazz mode to pulse layers");
    act("ctrl+c  = quit FreeJ");
    act("ctrl+f  = go to Fullscreen");
    act("ctrl+l  = cleanup and redraw the console");
    act("ctrl+i  = switch on/off On Screen Display information");
#ifdef WITH_JAVASCRIPT
    act("ctrl+x  = execute a Javascript command");
#endif
    break;

  case KEY_CTRL_I:
    env->osd.active = !env->osd.active;
    break;

  case KEY_CTRL_E:
    if(!layer) {
      error("can't add Effect: no Layer is selected, select one using arrows.");
      break;
    }
    readline("add new Effect - press TAB for completion:",&filter_proc,&filter_comp);
    break;

  case KEY_CTRL_F:
    env->screen->fullscreen();
    break;

  case KEY_CTRL_B:
    if(!layer) {
      error("can't change Blit: no Layer is selected, select one using arrows.");
      break;
    }
    readline("select Blit mode for the selected Layer - press TAB for completion:",
	     &blit_selection,&blit_comp);
    break;
  case KEY_CTRL_V:
    if(!layer) {
      error("can't change Blit Value: no Layer is selected, select one using arrows.");
      break;
    }
    readline("set Blit value for the selected Layer:",
	     &set_blit_value,NULL);
    break;

  case KEY_CTRL_O:
    readline("open a file in a new Layer:",
	     &open_layer,&filebrowse_completion);
    break;
      
  case KEY_CTRL_X:
#ifndef WITH_JAVASCRIPT
    ::error("javascript is not compiled in this FreeJ binary");
  break;
#else
  readline("input script command:",&js_proc,NULL);
  break;
#endif

  case KEY_CTRL_L:
    refresh();
    break;

  case KEY_CTRL_M:
    ::notice("move layer with arrows, press enter when done");
  ::act("use arrow keys to move or keypad numbers");
  ::act("also nethack movement keys work, press space to center");
  parser = MOVELAYER;
  break;

  case KEY_CTRL_J:
    ::notice("JAZZ mode activated, press keys to pulse layers");
  parser = JAZZ;
  break;

  default:
    if(filter)
      filter->kbd_input( key );
    else if(layer)
      layer->keypress( key );
    break;
			 
    //    case KEY_CTRL_T:
    //      ::notice("Welcome to %s %s",PACKAGE,VERSION);
    //    :: act("layers supported:\n%s",layers_description);
    //    break;
    }
}

void Console::parser_movelayer(int key) {
  commandline = false; // print statusline

  switch(key) {
  case KEY_PLUS:
    if(movestep<0xff) {
      movestep++;
      ::act("movement step increased to %i",movestep);
    }
    break;
  case KEY_MINUS:
    if(movestep>1) {
      movestep--;
      ::act("movement step decreased to %i",movestep);
    }
    break;
  case '8':
  case 'k':
  case SL_KEY_UP:
    layer->set_position(layer->geo.x,layer->geo.y-movestep);
    break;
  case '2':
  case 'j':
  case SL_KEY_DOWN:
    layer->set_position(layer->geo.x,layer->geo.y+movestep);
    break;
  case '4':
  case 'h':
  case SL_KEY_LEFT:
    layer->set_position(layer->geo.x-movestep,layer->geo.y);
    break;
  case '6':
  case 'l':
  case SL_KEY_RIGHT:
    layer->set_position(layer->geo.x+movestep,layer->geo.y);
    break;
  case '7':
  case 'y': // up+left
    layer->set_position(layer->geo.x-movestep,layer->geo.y-movestep);
    break;
  case '9':
  case 'u': // up+right
    layer->set_position(layer->geo.x+movestep,layer->geo.y-movestep);
    break;
  case '1':
  case 'b': // down+left
    layer->set_position(layer->geo.x-movestep,layer->geo.y+movestep);
    break;
  case '3':
  case 'n': // down+right
    layer->set_position(layer->geo.x+movestep,layer->geo.y+movestep);
    break;
    
  case '5':
  case KEY_SPACE:
    // place at the center
    layer->set_position
      ( (env->screen->w - layer->geo.w)/2,
	(env->screen->h - layer->geo.h)/2 );
    break;

  case SL_KEY_ENTER:
  case KEY_ENTER:
    ::act("layer repositioned");
    parser = DEFAULT;
    break;
  }
  return;
}

void Console::parser_jazz(int key) {
  commandline = false;
  Layer *lay;
  int num;
  // table of valid keys
  char *jazzkeys = "qwertyuiopasdfghjklzxcvbnm\0";
  char *p;

  // search for the key
  for(p = jazzkeys; *p != key; p++)
    if(*p=='\0') break;

  if(*p!='\0') { // found
    num = p - jazzkeys +1;
    ::func("pulse layer %i",num);
    // find it
    lay = (Layer*) env->layers.pick(num);
    // check it
    if(!lay) return;
    if(!lay->active) return;
    // pulse it
    lay->blitter.pulse_value(jazzstep,jazzvalue);
    env->layers.sel(0);
    lay->sel(true);
    return;
  }

  switch(key) {
  case SL_KEY_UP:
    if(jazzstep<0xff) jazzstep++;
    ::act("jazz mode step set to %i",jazzstep);
    break;
  case SL_KEY_DOWN:
    if(jazzstep>1) jazzstep--;
    ::act("jazz mode step set to %i",jazzstep);
    break;
  case SL_KEY_RIGHT:
    if(jazzvalue<0xff) jazzvalue++;
    ::act("jazz mode value set to %i",jazzvalue);
    break;
  case SL_KEY_LEFT:
    if(jazzvalue>1) jazzvalue--;
    ::act("jazz mode value set to %i",jazzvalue);
    break;

  case SL_KEY_ENTER:
  case KEY_ENTER:
    ::act("JAZZ mode exited");
  parser = DEFAULT;
  break;
  }
  
}

// read a command from commandline
// handles completion and execution from function pointers previously setup
void Console::parser_commandline(int key) {
  int res, c;

  commandline = true; // don't print statusline

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
    // a blank commandline aborts the input
    if(command[0]==EOL) {
      func("command aborted");
      parser = DEFAULT;
      cmd_process = NULL;
      cmd_complete = NULL;
      statusline();
      return;
    }
    // otherwise process the input
    res = (*cmd_process)(command);
    if(res<0) return;
    // reset the parser
    parser = DEFAULT;
    cmd_process = NULL;
    cmd_complete = NULL;
    statusline();
    // save in commandline history
    entr = new Entry();
    entr->data = strdup(command);
    history.append( entr );
    if(history.len()>32) // histsize
      delete history.begin();
    entr = NULL;
    return;

  case SL_KEY_UP:
    // pick from history
    if(!entr) // select the latest
      entr = history.end();
    else
      entr = entr->prev;
    if(!entr) return; // no hist
    strncpy(command,(char*)entr->data,512);
    // type the command on the console
    SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
    SLsmg_write_string(command);
    SLsmg_erase_eol();
    cursor = strlen(command);
    GOTO_CURSOR;
    return;

  case SL_KEY_DOWN:
    // pick from history
    if(!entr) return;
    if(!entr->next) return;
    entr = entr->next;
    strncpy(command,(char*)entr->data,512);
    // type the command on the console
    SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
    SLsmg_write_string(command);
    SLsmg_erase_eol();
    cursor = strlen(command);
    GOTO_CURSOR;
    return;

  case KEY_CTRL_G:
    parser = DEFAULT;
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


  case KEY_BACKSPACE:      /*** FIXME */
    if(!cursor) return;
    cursor--;
    if(command[cursor+1]==EOL) {
      // func("cursor on end of line");
      command[cursor] = EOL;
    } else {
      for(c=cursor;command[c]!=EOL;c++)
	command[c] = command[c+1];
      command[c] = EOL;
      SLsmg_write_string(&command[cursor]);
    }
    SLsmg_erase_eol();
    GOTO_CURSOR;
    return;

    /* the following ctrl combos are to imitate
       the UNIX commandline behaviour
       (c-e eol, c-d delete, c-k eol del etc.) */
  case SL_KEY_LEFT:
    if(cursor) cursor--;
    GOTO_CURSOR;
    return;
  case SL_KEY_RIGHT:
    if(command[cursor]) cursor++;
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
     insert mode       FIX ME! */
  for(c=cursor;command[c+1]!=EOL;c++)
    command[c+1] = command[c];
  command[cursor] = key;
  
  GOTO_CURSOR;
  SLsmg_write_string(&command[cursor]);
  SLsmg_erase_eol();
  cursor++;
  GOTO_CURSOR;
}
  
