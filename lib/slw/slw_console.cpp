/*  S-Lang console functions in C++
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

#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <slang.h>

#include <jutils.h>

#include <slw_console.h>


///////////////////////////////////
// dirty signals

static bool screen_size_changed;
static void sigwinch_handler (int sig) {
  screen_size_changed = true;
  SLsignal (SIGWINCH, sigwinch_handler);
}

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
  if(SLang_input_pending(1))
  //    return SLang_getkey();
    ch = SLang_getkey();
  if(ch) func("getkey_handler detected char %u",ch);
  return ch;
}

// end of dirty signals
/////////////////////////////////////


SLangConsole::SLangConsole() {

  w = h = 0;

}

SLangConsole::~SLangConsole() { close(); }

bool SLangConsole::init() {

  setenv("TERM","xterm-color",0); 
  SLtt_get_terminfo();

  if( -1 == SLkp_init() ) { // keyboard interface
	error("failure in SLkp_init()");
  	return false;
  }

  SLang_init_tty(-1, 0, 0);

  SLsmg_init_smg(); // screen manager

  //  SLutf8_enable(1); // enable UTF8 character set
//  this is a mess ...
  screen_size_changed = false;

  // register signals
  SLsignal (SIGWINCH, sigwinch_handler);
  SLang_set_abort_signal(sigint_handler);

  SLkp_set_getkey_function(getkey_handler);
  

  // SLsmg_Tab_Width = 8;
  // SLsmg_Display_Eight_Bit = 1;
  // SLtt_Use_Ansi_Colors = 0;
  // SLtt_Term_Cannot_Scroll = 0;

  // set sizes of the whole console
  w = SLtt_Screen_Cols;
  h = SLtt_Screen_Rows;

  /* setup colors with the palette scheme:
     n = normal;
     n+10 = highlight;
     n+20 = reverse normal;
     n+30 = reverse highlight; */

  // crazy casting for crazy slang
  SLtt_set_color(1,NULL,(char *)"lightgray",(char *)"black");
  SLtt_set_color(11,NULL,(char *)"white",(char *)"black");
  SLtt_set_color(21,NULL,(char *)"black",(char *)"lightgray");
  SLtt_set_color(31,NULL,(char *)"black",(char *)"white");
  
  SLtt_set_color(2,NULL,(char *)"red",(char *)"black");
  SLtt_set_color(12,NULL,(char *)"brightred",(char *)"black");
  SLtt_set_color(22,NULL,(char *)"black",(char *)"red");
  SLtt_set_color(32,NULL,(char *)"black",(char *)"brightred");
  
  SLtt_set_color(3,NULL,(char *)"green",(char *)"black");
  SLtt_set_color(13,NULL,(char *)"brightgreen",(char *)"black");
  SLtt_set_color(23,NULL,(char *)"black",(char *)"green");
  SLtt_set_color(33,NULL,(char *)"black",(char *)"brightgreen");
  
  SLtt_set_color(4,NULL,(char *)"brown",(char *)"black");
  SLtt_set_color(14,NULL,(char *)"yellow",(char *)"black");
  SLtt_set_color(24,NULL,(char *)"black",(char *)"brown");
  SLtt_set_color(34,NULL,(char *)"black",(char *)"yellow");
  
  SLtt_set_color(5,NULL,(char *)"blue",(char *)"black");
  SLtt_set_color(15,NULL,(char *)"brightblue",(char *)"black");
  SLtt_set_color(25,NULL,(char *)"black",(char *)"blue");
  SLtt_set_color(35,NULL,(char *)"black",(char *)"brightblue");
  
  SLtt_set_color(6,NULL,(char *)"magenta",(char *)"black");
  SLtt_set_color(16,NULL,(char *)"brightmagenta",(char *)"black");
  SLtt_set_color(26,NULL,(char *)"black",(char *)"magenta");
  SLtt_set_color(36,NULL,(char *)"black",(char *)"brightmagenta");
  
  SLtt_set_color(7,NULL,(char *)"cyan",(char *)"black");
  SLtt_set_color(17,NULL,(char *)"brightcyan",(char *)"black");
  SLtt_set_color(27,NULL,(char *)"black",(char *)"cyan");
  SLtt_set_color(37,NULL,(char *)"black",(char *)"brightcyan");
  

  
  refresh();

  return true;
}

void SLangConsole::close() {

  SLsmg_reset_smg();
  SLang_reset_tty();
}

int SLangConsole::getkey() {
  // low level
  return SLkp_getkey();
}

void SLangConsole::feed(int key) {

///	func("feeding '%c' to widget %s", key, focused->name);

	if(focused)
		focused->feed(key);
}


bool SLangConsole::place(SLangWidget *wid, int hx, int hy, int lx, int ly) {
 
	wid->orig_x = hx;
	wid->orig_y = hy;

	// boundaries where lower right coord
	// get converted into WxH
	if( lx > this->w )
		wid->w = this->w - hx;
	else
		wid->w = lx - hx;

	if( ly > this->h )
		wid->h = this->h - hy;
	else
		wid->h = ly - hy;


	// save a reference of the console in the widget
	wid->console = this;


	// append the new widget in our linklist
	widgets.append( wid );

	func("s-lang widget %s sized %ux%u placed at %u,%u",
			wid->name, wid->w, wid->h,
			wid->orig_x, wid->orig_y);

	// draw border if requested
	if(wid->border)
		/* S-Lang has a coordinate system which is very
		 * different from the typical approach in computer graphics;
		 * the draw_box function works as follows:
		   SLsmg_draw_box (int r, int c, int dr, int dc);
		   Draw a box whose right corner is at row r and column c.
		   The box spans dr rows and dc columns.
		   The current position will be left at row r and column c.
		   */
		SLsmg_draw_box(wid->orig_y, wid->orig_x,
			       wid->h,    wid->w);

	// wid->refresh();	

	// focus the first widget
	if(!focused) focused = wid;

	return true;
}


bool SLangConsole::refresh() {

  /* S-Lang says: 
   * All well behaved applications should block signals that
   * may affect the display while performing screen update. */
  SLsig_block_signals ();
  
  if(screen_size_changed && !keyboard_quit) {

    SLtt_get_screen_size ();
    SLsmg_reinit_smg ();

    this->w = SLtt_Screen_Cols;
    this->h = SLtt_Screen_Rows;

    screen_size_changed = false;
  
    // refresh all widgets
    SLangWidget *wid;
    wid = (SLangWidget*) widgets.begin();
    while(wid) {

      wid->refresh();
      wid = (SLangWidget*) wid->next;

    }
  }

  if(focused && !keyboard_quit)
	  if(focused->cursor) {
	    SLtt_set_cursor_visibility(1);
	    focused->gotoxy( focused->cur_x, focused->cur_y);
          } else
	    SLtt_set_cursor_visibility(0);

 
  SLsmg_refresh();
  
  SLsig_unblock_signals();
 
  if(keyboard_quit) {
	  func("keyboard requested forced quit");
	  return false;
  }

  return true;
}

////////////////////////
/// messaging interface (TODO)
// void SLangConsole::notice (char *str) { }
// void SLangConsole::act    (char *str) { }
// void SLangConsole::warning(char *str) { }
// void SLangConsole::error  (char *str) { }
// void SLangConsole::func   (char *str) { }
////////////////////////


