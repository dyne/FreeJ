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
#include <jutils.h>
#include <config.h>

#define PLAIN_COLOR 1
#define TITLE_COLOR 1
#define LAYERS_COLOR 3
#define FILTERS_COLOR 7
#define SCROLL_COLOR 5

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
  if(SLang_input_pending(0))
    return SLang_getkey();
  else return 0;
}

Console::Console() {
  env=NULL;
  last_line=NULL;
  num_lines=0;
  do_update_scroll=true;
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

  env->track_fps = true; // we wanna know!

  return true;
}

void Console::close() {
  set_console(NULL);
  SLsmg_reset_smg();  
  SLang_reset_tty();
}


void Console::getkey() {
  int key = SLkp_getkey();
  if(key) ::func("SLkd_getkey: %u",key);
  switch(key) {
  case ':':
    ::notice("console input requested");
  break;
  case 272:
    ::notice("help triggered");
  break;
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
  layerprint();
  layerlist(); // print layer list


  /*if a filter is selected, then
    print info on a new filter, if a new one was selected */
  if(layer) {
    tmpfilt = (Filter *)layer->filters.selected();
    if(tmpfilt!=filter) {
      filter = tmpfilt;
      filterprint();
    }
    filterlist(); // print filter list
  }

  if(do_update_scroll)
    update_scroll();
  
  speedmeter();

  statusline();

  SLsmg_refresh();
 
  SLsig_unblock_signals ();
}

void Console::statusline() {
  SLsmg_set_color(TITLE_COLOR+20);
  SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  SLsmg_write_string(" press ctrl-h for help | : for commandline input | running on ");
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

  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_gotorc(14,0);
  SLsmg_draw_hline(72);
  SLsmg_gotorc(SLtt_Screen_Rows - 2,0);
  SLsmg_draw_hline(72);
}

void Console::layerprint() {
  layer = (Layer*)env->layers.selected();
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
  scroll(msg,PLAIN_COLOR);
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

  SLsmg_gotorc (nrows, col);
  SLsmg_set_color(line->color+10);
  SLsmg_write_string (line->data);
  SLsmg_erase_eol ();

  line = line->prev;

  nrows--;

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
