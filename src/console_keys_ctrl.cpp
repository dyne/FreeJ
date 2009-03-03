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


#include <console_calls_ctrl.h>

#include <console_ctrl.h>

#include <context.h>
#include <layer.h>
#include <jutils.h>

#include <keycodes.h> // from lib/slw 

////////////////////////////////////////////
// KEY PARSERS
////////////////////////////////////////////


void Console::parser_default(int key) {
  Entry *le, *fe;

  commandline = false; // print statusline

  ::func("pressed %u",key);

  if(env->layers.len() > 0) { // there are layers

    // get the one selected
    le = env->layers.selected();
    if(!le) {
      env->layers.begin();
      le->sel(true);
    }

    fe = ((Layer*)le)->filters.selected();
    
    // switch over operations and perform
    switch(key) {
      
    case SL_KEY_UP:
      
      if(!fe) break; // no filter
      
      fe = fe->prev; // take the upper one
      ((Layer*)le)->filters.sel(0); // deselect all filters
      if(fe) fe->sel(true); // select only the current
      
      break;
      
    case SL_KEY_DOWN:
      
      if(!fe) {
	fe = ((Layer*)le)->filters.begin();
	if(!fe) break; // no filters
	else fe->sel(true);
      } else if(fe->next) {
	fe = fe->next;
	((Layer*)le)->filters.sel(0);
	fe->sel(true);
      }
      break;
      
    case SL_KEY_LEFT:
    
      if(!fe) { // no filter selected, move across layers
	
	// move to the previous or the other end
	if(!le->prev)
	  le = env->layers.end();
	else
	  le = le->prev;
	
	// select only this layer
	env->layers.sel(0);
	le->sel(true);
	
      } else { // a filter is selected: move across filter parameters
	
	// TODO
	
      }
      
      break;
      
    case SL_KEY_RIGHT:

      if(!fe) { // no filter selected, move across layers
	
	// move to the next layer or the other end
	if(!le->next)
	  le = env->layers.begin();
	else le = le->next;
	
	// select only the current
	env->layers.sel(0);
	le->sel(true);
	
      } else { // move across filter parameters
	
	// TODO
	
      }
      break;
      
    case SL_KEY_PPAGE:
    case KEY_PLUS:
      if(fe) fe->up();
      else   le->up();
      break;
      
    case SL_KEY_NPAGE:
    case KEY_MINUS:
      if(fe) fe->down();
      else   le->down();
      break;
      
    case SL_KEY_DELETE:
    case KEY_CTRL_D:
      if(fe) {
	fe->rem();
	delete fe;
      } else {
	//	le->rem();
	//	((Layer*)le)->close();
	env->rem_layer( (Layer*)le );
      }
      break;
      
    case KEY_SPACE:
      if(fe) ((FilterInstance*)fe)->active =
	       !((FilterInstance*)fe)->active;
      else  ((Layer*)le)->active =
	      !((Layer*)le)->active;
    break;

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
      
      
#if defined WITH_FT2 && defined WITH_FC
    case KEY_CTRL_Y:
      if(((Layer*)le)->type == Layer::TEXT)
	readline("print a new word in Text Layer, type your words:",
		 &console_print_text_layer,NULL);
      break;
#endif

    case KEY_CTRL_A:
      ::notice("move layer with arrows, press enter when done");
      ::act("use arrow keys to move, or keypad numbers");
      ::act("+ and - zoom, < and > rotate");
      ::act("w and s spin zoom, a and d spin rotate");
      ::act(", stop rotation . stop zoom and <space> to center");
      ::act("press <enter> when you are done");
      parser = MOVELAYER;
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
    print_help();
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
    env->screen->fullscreen();
    break;
    
  case KEY_CTRL_X:
    readline("execute javascript command:",
	     &console_exec_script_command,NULL);
    break;
  
  case KEY_CTRL_J:
    readline("load and execute a javascript file:",
	     &console_exec_script, &console_filebrowse_completion);
    break;

  case KEY_CTRL_L:
    refresh();
    break;

  case KEY_CTRL_O:
    readline("open a file in a new Layer:",
	     &console_open_layer,&console_filebrowse_completion);
    break;

      
    case KEY_CTRL_G:
      readline("create a generator in a new Layer:",
	       &console_generator_selection, &console_generator_completion);
      break;


#if defined WITH_FT2 && defined WITH_FC
  case KEY_CTRL_T:
    readline("create a new Text Layer, type your words:",
	     &console_open_text_layer,NULL);
    break;
#endif
    
  default: break;
  
  }
}

void Console::parser_movelayer(int key) {
  commandline = false; // print statusline

  // get the one selected
  Layer *layer = (Layer*)env->layers.selected();
  if(!layer) {
    env->layers.begin();
    layer->sel(true);
  }

  switch(key) {
    
    // zoom
  case KEY_PLUS:  layer->set_zoom( layer->zoom_x + 0.01,
				   layer->zoom_y + 0.01); break;
  case KEY_MINUS: layer->set_zoom( layer->zoom_x - 0.01,
				   layer->zoom_y - 0.01); break;
  case 'w':       layer->set_spin(0,-0.001);    break;
  case 's':       layer->set_spin(0,0.001);     break;
  case '.':       layer->set_zoom(1,1);         break;
    
    // rotation
  case '<': layer->set_rotate( layer->rotate + 0.5 ); break;
  case '>': layer->set_rotate( layer->rotate - 0.5 ); break;
  case 'a': layer->set_spin(0.02,0);   break;
  case 'd': layer->set_spin(-0.02,0);  break;
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
      ( (env->screen->w - layer->geo.w)/2,
	(env->screen->h - layer->geo.h)/2 );
    break;

  case SL_KEY_ENTER:
  case KEY_ENTER:
  case KEY_CTRL_I:
    ::act("layer repositioned");
    parser = DEFAULT;
    break;
  }
  return;
}


// read a command from commandline
// handles completion and execution from function pointers previously setup
void Console::parser_commandline(int key) {
  int res, c;
  Entry *entr;

  commandline = true; // don't print statusline

  /* =============== console command input */
  if(cursor>MAX_CMDLINE) {
    error("command too long, can't type more.");
    return;
  }
  //::func("input key: %i",key);
  SLsmg_set_color(PLAIN_COLOR);
  
  switch(key) {
    
  case SL_KEY_ENTER:
  case KEY_ENTER:
    // a blank commandline aborts the input
    if(command[0]==EOL || command[0]==EOT) {
      parser = DEFAULT;
      cmd_process = NULL;
      cmd_complete = NULL;
      statusline(NULL);
      return;
    }
    statusline(command);
    // otherwise process the input
    res = (*cmd_process)(env,command);
    if(res<0) return;
    // reset the parser
    parser = DEFAULT;
    cmd_process = NULL;
    cmd_complete = NULL;
    statusline(NULL);
    // save in commandline history
    entr = new Entry();
    entr->data = strdup(command);
    history.append( entr );
    if(history.len()>32) // histsize
      delete history.begin();
    return;

  case SL_KEY_UP:
    // pick from history
    if(!entr) // select the latest
      entr = history.end();
    else
      entr = entr->prev;
    if(!entr) return; // no hist
    strncpy(command,(char*)entr->data,MAX_CMDLINE);
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
    strncpy(command,(char*)entr->data,MAX_CMDLINE);
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
    statusline(NULL);
    return;

  case KEY_TAB:
    if(!cmd_complete) return;
    if(command[0]=='\n')
      command[0]=0x0;
    res = (*cmd_complete)(env,command);
    if(!res) return;
    else if(res==1) { // exact match!
      SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
      SLsmg_write_string(command);
      SLsmg_erase_eol();
      //      cursor = strlen(command);
    }
    //    update_scroll();
    // type the command on the console
    SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
    SLsmg_write_string(command);
    SLsmg_erase_eol();
    cursor = strlen(command);
    GOTO_CURSOR;
    return;


  case KEY_BACKSPACE:
  case KEY_BACKSPACE_APPLE:
  case KEY_BACKSPACE_SOMETIMES:
    if(!cursor) return;

    for(c=cursor;c<MAX_CMDLINE;c++) {
      command[c-1] = command[c];
      if(command[c]==EOL) break;
    }

    SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
    SLsmg_write_string(command);
    SLsmg_erase_eol();
    cursor--;
    SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor);
    return;

    /* the following ctrl combos are to imitate
       the Emacs commandline behaviour
       c-e goto end of line,
       c-d delete,
       c-k delete until end of line
       c-u delete previous until beginning of line
    */

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
     insert mode       FIX ME!
     must save temporarly the chars to advance
  */

  for(c=cursor;command[c]!=EOL;c++); // go to the EOL
  
  command[c+1] = EOL; // be sure we have a EOL

  for(;c>cursor;c--)
    command[c] = command[c-1]; // move backwards switching right

  command[cursor] = key; // insert new char
  
  //  GOTO_CURSOR;
  SLsmg_write_string(&command[cursor]);
  SLsmg_erase_eol();
  cursor++;
  GOTO_CURSOR;

}
  
