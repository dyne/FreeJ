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

#include <config.h>


#include <context.h>
#include <layer.h>
#include <jutils.h>

#include <slang_console_ctrl.h>
#include <console_calls_ctrl.h>
#include <console_readline_ctrl.h>

#include <keycodes.h> // from lib/slw 




SlwReadline::SlwReadline() 
  : SLangWidget() {
  
  movestep=2;
  parser = DEFAULT;
  commandline = false;
  memset(command, EOL, MAX_CMDLINE);

}

SlwReadline::~SlwReadline() { 

  Entry *e;
  e = history.begin();
  while(e) {
    free(e->data);
    e->rem();
    delete(e);
    e = history.begin();
  }

}

bool SlwReadline::init() {
  initialized = true;
  return(true);
}

bool SlwReadline::feed(int key) {
  bool res = false;

  switch(parser) {
  case COMMANDLINE:
    
    parser_commandline(key);
    break;

  case MOVELAYER:

    res = parser_movelayer(key);
    break;

  case DEFAULT:
    res = parser_default(key);
    break;

  }

  // TODO: returns always true
  return(res);
}

bool SlwReadline::refresh() {
  switch(parser) {
  case DEFAULT:
  case MOVELAYER:
    color = TITLE_COLOR+20;
    putnch(" use arrows to move selection, press ctrl-h for help with hotkeys",0,0,0);
    break;
    
  case COMMANDLINE:
    blank_row(0);
    color = PLAIN_COLOR;
    putnch((char*)": ", 0, 0, 2);
    if(command[0]!=EOL)
      putnch(command, 2, 0, 0);
    else
      cursor = 0;
    gotoxy(cursor+2, 0);
    break;
    
  default: break;
  }
  return(true);
}


void SlwReadline::set_parser(parser_t pars) {
  switch(pars) {

  case DEFAULT:
    parser = DEFAULT;
    SLtt_set_cursor_visibility(0);
    break;
    
  case COMMANDLINE:
    parser = COMMANDLINE;
    SLtt_set_cursor_visibility(1);
    break;
    
  case MOVELAYER:
    ::notice("move layer with arrows, press enter when done");
    ::act("use arrow keys to move, or keypad numbers");
    ::act("+ and - zoom, < and > rotate");
    ::act("w and s spin zoom, a and d spin rotate");
    ::act(", stop rotation . stop zoom and <space> to center");
    ::act("press <enter> when you are done");
    parser = MOVELAYER;
    SLtt_set_cursor_visibility(0);
    break;

  default:
    error("unknown parser for readline");
    break;
  }
  
}


/* setup the flags and environment to read a new input
   saves the pointer to the command processing function
   to use it once the input is completed */
int SlwReadline::readline(const char *msg,cmd_process_t *proc,cmd_complete_t *comp) {
  ::notice(msg);
  //  update_scroll();
  color = PLAIN_COLOR;

  blank();

  //  putnch((char*)": ", 0, 0, 2);
  //   SLsmg_gotorc(SLtt_Screen_Rows - 1,0);
  //   SLsmg_write_string((char *)":");
  //   SLsmg_erase_eol();
  

  memset(command,EOL,MAX_CMDLINE);
  
  cmd_process = proc;
  cmd_complete = comp;
  
  commandline = true;
  set_parser(COMMANDLINE);

  if(cmd_complete)
    (*cmd_complete)(env, command);

  cursor = strlen(command);
  return 1;
}


////////////////////////////////////////////
// KEY PARSERS
////////////////////////////////////////////


bool SlwReadline::parser_default(int key) {
  Entry *le, *fe;
  bool res = true;

  commandline = false; // print statusline

  //  ::func("pressed %u",key);

  if(env->screens.selected()->layers.len() > 0) { // there are layers

    // get the one selected
    le = env->screens.selected()->layers.selected();
    if(!le) {
      env->screens.selected()->layers.begin();
      le->sel(true);
    }

    fe = ((Layer*)le)->filters.selected();
    
    // switch over operations and perform
    switch(key) {
      

//     case KEY_CTRL_M: {
// 		Layer *l=((Layer*)le);
// 		if (l->fps->get() > 0)
// 			l->set_fps(0);
// 		else
// 			if (l->fps_old > 0)
// 				l->set_fps(l->fps_old);
// 			else
// 				l->set_fps(env->fps_speed);
// 		l->signal_feed();
// 		::notice("Layer.set_fps(%f)", l->fps);
// 	}
// 	break;

    case KEY_CTRL_E:
      readline("add new Effect - press TAB for completion:",
	       &console_filter_selection, &console_filter_completion);
      break;

    case KEY_CTRL_P:
      readline("set parameter - press TAB for completion:",
	       &console_param_selection, &console_param_completion);
      break;

    case KEY_CTRL_B:
      readline("select Blit mode for the selected Layer - press TAB for completion:",
	       &console_blit_selection, &console_blit_completion);
      break;
    case KEY_CTRL_V:
      readline("select Blit parameter value - press TAB for completion:",
	       &console_blit_param_selection, &console_blit_param_completion);
      break;
      
#if defined WITH_TEXTLAYER
    case KEY_CTRL_Y:
      if(((Layer*)le)->type == Layer::TEXT)
	readline("print a new word in Text Layer, type your words:",
		 &console_print_text_layer,NULL);
      break;
#endif

    case KEY_CTRL_A:
      set_parser(MOVELAYER);
      break;
      
      //  case KEY_CTRL_J:
      //    ::notice("JAZZ mode activated, press keys to pulse layers");
      //  parser = JAZZ;
      //  break;
      
    default:
      //      ((Layer*)le)->keypress( key );
      break;
      
    }
  }
  
  switch(key) {
  case KEY_CTRL_H:
  case KEY_CTRL_H_APPLE:
  case '?':
    notice("Hotkeys available in FreeJ console:");
    act("ctrl+o  = Open a Layer (will prompt for path to file)");
    act("Arrow keys browse selection thru layers and effects");
    act("SPACE to de/activate layers and filters selected");
    act("ENTER to start/stop layers selected");
    act("+ and - move filters and effects thru chains");
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
    break;
    /*    
  case '!':
    env->osd.active = !env->osd.active;
    break;
    */

  case '@':
    env->clear_all = !env->clear_all;
    break;
    
    /*
  case '<':
    // decrease global fps
    if(env->fps_speed>1)
      env->fps_speed--;
    else break;
    //    env->set_fps_interval(env->fps_speed);
    //    ::act("Frames per second decreased to %i",env->fps_speed);
    break;
  case '>':
    // increase global fps
    env->fps_speed++;
    //    env->set_fps_interval(env->fps_speed);
    //    ::act("Frames per second increased to %i",env->fps_speed);
    break;
    */
  case KEY_CTRL_F:
    env->screens.selected()->fullscreen();
    break;
    
  case KEY_CTRL_X:
    readline("execute javascript command:",
	     &console_exec_script_command,NULL);
    break;
  
  case KEY_CTRL_J:
    readline("load and execute a javascript file:",
	     &console_exec_script, &console_filebrowse_completion);
    break;


  case KEY_CTRL_O:
    readline("open a file in a new Layer:",
	     &console_open_layer,&console_filebrowse_completion);
    break;

      
  case KEY_CTRL_G:
    readline("create a generator in a new Layer:",
	     &console_generator_selection, &console_generator_completion);
    break;
    

#if defined WITH_TEXTLAYER
  case KEY_CTRL_T:
    readline("create a new Text Layer, type your words:",
	     &console_open_text_layer,NULL);
    break;
#endif
    
  default:
    res = false;
    break;
  
  }
  return(res);
}

bool SlwReadline::parser_movelayer(int key) {
  bool res = true;

  commandline = false; // print statusline

  // get the one selected
  Layer *layer = (Layer*)env->screens.selected()->layers.selected();
  if(!layer) {
    env->screens.selected()->layers.begin();
    layer->sel(true);
  }

  switch(key) {
    
    // zoom
  case KEY_PLUS:  layer->set_zoom( layer->zoom_x + 0.01,
				   layer->zoom_y + 0.01); break;
  case KEY_MINUS: layer->set_zoom( layer->zoom_x - 0.01,
				   layer->zoom_y - 0.01); break;
  case '.':       layer->set_zoom(1,1);         break;
    
    // rotation
  case '<': layer->set_rotate( layer->rotate + 0.5 ); break;
  case '>': layer->set_rotate( layer->rotate - 0.5 ); break;
  case ',': layer->set_rotate(0);      break;
  case 'z': layer->antialias =
      !layer->antialias;       break;
    
    
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
      ( (env->screens.selected()->geo.w - layer->geo.w)/2,
	(env->screens.selected()->geo.h - layer->geo.h)/2 );
    break;

  case SL_KEY_ENTER:
  case KEY_ENTER:
  case KEY_CTRL_I:
    ::act("layer repositioned");
    set_parser(DEFAULT);
    break;

  default:
    res = false;
    break;
  }
  return(res);
}


// read a command from commandline
// handles completion and execution from function pointers previously setup
bool SlwReadline::parser_commandline(int key) {
  int res, c;
  bool parsres = true;
  Entry *entr = NULL;

  commandline = true; // don't print statusline

  /* =============== console command input */
  if(cursor>MAX_CMDLINE) {
    error("command too long, can't type more.");
    return(parsres);
  }
  //::func("input key: %i",key);
  //  SLsmg_set_color(PLAIN_COLOR);
  color = PLAIN_COLOR;

  switch(key) {
    
  case SL_KEY_ENTER:
  case KEY_ENTER:
    // a blank commandline aborts the input
    if(command[0]==EOL || command[0]==EOT) {
      set_parser(DEFAULT);
      break;
    }
    // otherwise process the input
    res = (*cmd_process)(env,command);
    if(res<0) break;
    // reset the parser
    set_parser(DEFAULT);
    // save in commandline history
    entr = new Entry();
    entr->data = strdup(command);
    history.append( entr );
    if(history.len()>32) // histsize
      delete history.begin();
    // cleanup the command
    memset(command, EOL, MAX_CMDLINE);
    break;

  case SL_KEY_UP:
    // pick from history
    entr = history.selected();
    if(!entr) { // select the latest
      entr = history.end();
      if(entr) entr->sel(true);
    } else {
      entr = entr->prev;
      if(entr) {
	history.sel(0);
	entr->sel(true);
      }
    }
    if(!entr) break; // no hist
    strncpy(command,(char*)entr->data,MAX_CMDLINE);
    // type the command on the consol
    cursor = strlen(command);
    //    GOTO_CURSOR;
    break;

  case SL_KEY_DOWN:
    // pick from history
    if(!entr) break;
    if(!entr->next) break;
    entr = entr->next;
    strncpy(command,(char*)entr->data,MAX_CMDLINE);
    // type the command on the console
    cursor = strlen(command);

    //    GOTO_CURSOR;
    break;

  case KEY_CTRL_G:
    set_parser(DEFAULT);
    // cleanup the command
    memset(command, EOL, MAX_CMDLINE);
    break;

  case KEY_TAB:
    if(!cmd_complete) break;
    if(command[0]=='\n') command[0]=0x0;
    res = (*cmd_complete)(env,command);
    if(!res) break;
    //    else if(res==1) { // exact match!
    //      putnch(command,3,0,0);
      //      cursor = strlen(command);
    //    }
    //    update_scroll();
    // type the command on the console
    cursor = strlen(command);
    //    GOTO_CURSOR;
    break;


  case KEY_BACKSPACE:
  case KEY_BACKSPACE_APPLE:
  case KEY_BACKSPACE_SOMETIMES:
    if(!cursor) break;

    for(c=cursor;c<MAX_CMDLINE;c++) {
      command[c-1] = command[c];
      if(command[c]==EOL) break;
    }
    cursor--;
    //    putnch(command,3,0,cursor);
    //    SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor);
    break;

    /* the following ctrl combos are to imitate
       the Emacs commandline behaviour
       c-e goto end of line,
       c-d delete,
       c-k delete until end of line
       c-u delete previous until beginning of line
    */

  case SL_KEY_LEFT:
    if(cursor) cursor--;
    //    gotoxy(cursor,0);
    break;
  case SL_KEY_RIGHT:
    if(command[cursor]) cursor++;
    //    gotoxy(cursor,0);
    break;

  case KEY_CTRL_D:
    for(c=cursor;command[c]!=EOL;c++)
      command[c] = command[c+1];
    //    putnch(command,1,0,0);
    //    gotoxy(cursor,0);
//     GOTO_CURSOR;
//     SLsmg_write_string(&command[cursor]);
//     SLsmg_erase_eol();
//     GOTO_CURSOR;
    break;

  case KEY_CTRL_A:
  case KEY_HOME:
    cursor=0;
    //    gotoxy(cursor,0);
    break;

  case KEY_CTRL_E:
    while(command[cursor]!=EOL) cursor++;
    //    gotoxy(cursor,0);
    break;

  case KEY_CTRL_K:
    for(c=cursor;command[c]!=EOL;c++)
      command[c] = EOL;
    //    putnch(command,1,0,0);
    //    gotoxy(cursor,0);
    break;

  case KEY_CTRL_U:
    for(c=0;command[cursor+c]!=EOL;c++)
      command[c] = command[cursor+c];
    for(;command[c]!=EOL;c++)
      command[c] = EOL;
    cursor=0;
    //    putnch(command,1,0,0);
    //    gotoxy(cursor,0);
    break;

  default:
    parsres = false;
    break;

  }
  /* add char at cursor position
     insert mode       FIX ME!
     must save temporarly the chars to advance
  */
  if( key >= 32 && key < 127) {
    for(c=cursor;command[c]!=EOL;c++); // go to the EOL
    
    command[c+1] = EOL; // be sure we have a EOL
    
    for(;c>cursor;c--)
      command[c] = command[c-1]; // move backwards switching right
    
    command[cursor] = key; // insert new char
    
    cursor++;
    parsres = true;
  }
    //  gotoxy(cursor+2,0);
  return(parsres);
}
  
