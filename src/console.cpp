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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <signal.h>

#include <config.h>

#include <slang.h>
#include <context.h>
#include <blitter.h>

#include <jsparser.h>


#include <jutils.h>


#include <gen_layer.h>
#include <gen_f0r_layer.h>

#define PLAIN_COLOR 1
#define TITLE_COLOR 1
#define LAYERS_COLOR 3
#define FILTERS_COLOR 7
#define SCROLL_COLOR 5

#define EOL '\0'


#define KEY_ENTER 13
#define KEY_SPACE 32
#define KEY_BACKSPACE 275
#define KEY_BACKSPACE_APPLE 127 
#define KEY_BACKSPACE_SOMETIMES 272
#define KEY_LEFT 259
#define KEY_RIGHT 260
#define KEY_HOME 263
#define KEY_DELETE 275
#define KEY_TAB 9

/* unix ctrl- commandline hotkeys */
#define KEY_CTRL_A 1 // move/rotate/zoom the layer
#define KEY_CTRL_B 2 // change blit
#define KEY_CTRL_D 4 // delete char
#define KEY_CTRL_E 5 // add new effect
#define KEY_CTRL_F 6 // go fullscreen
#define KEY_CTRL_G 7 // create a particle layer
#define KEY_CTRL_H_APPLE 8 // ctrl-h on apple/OSX
#define KEY_CTRL_I 9 // OSD on/off
#define KEY_CTRL_K 11 // delete until end of line
#define KEY_CTRL_L 12 // refresh screen
#define KEY_CTRL_M 13 // toggle set_fps()  << this is also the RETURN key!
#define KEY_CTRL_U 21 // delete until beginning of line
#define KEY_CTRL_H 272 // help the user

#define KEY_CTRL_O 15 // open a file in a new layer
#define KEY_CTRL_P 16 // parameter set
#define KEY_CTRL_S 19 // reserved
#define KEY_CTRL_T 20 // new layer with text
#define KEY_CTRL_V 22 // change blit value
#define KEY_CTRL_W 23
#define KEY_CTRL_X 24 // execute a javascript command
#define KEY_CTRL_J 10 // load and execute a javascript script file
#define KEY_CTRL_Y 25

#define KEY_PLUS 43
#define KEY_MINUS 45

// just comfortable
#define GOTO_CURSOR \
      SLsmg_gotorc(SLtt_Screen_Rows - 1,cursor+1)



static Context *env;

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

static int param_selection(char *cmd) {
  Parameter *param;
  int idx;
  
  if(!cmd) return 0;
  if(!strlen(cmd)) return 0;

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  FilterInstance* filt =
    (FilterInstance*)lay->filters.selected();

  // find the values after the first blank space
  char *p;
  for(p = cmd; *p != '\0'; p++)
    if(*p == '=') {
      *p = '\0';
      if(*(p-1)==' ')
	*(p-1) = '\0';
      p++; break;
    }
  
  while(*p == ' ') p++; // jump all spaces
  if(*p=='\0') return 0; // no value was given

  if(filt) { ///////////////////////// parameters for filter

    // find the parameter
    param = (Parameter*)filt->proto->parameters.search(cmd, &idx);
    if( ! param) {
      error("parameter %s not found in filter %s", cmd, filt->proto->name);
      return 0;
    } else 
      func("parameter %s found in filter %s at position %u",
	   param->name, filt->proto->name, idx);

  } else if(lay->parameters) { /////// parameters for layer
    param = (Parameter*)lay->parameters->search(cmd, &idx);
    if( ! param) {
      error("parameter %s not found in layers %s", cmd, lay->name);
      return 0;
    } else 
      func("parameter %s found in layer %s at position %u",
	   param->name, lay->name, idx);
  }

  // parse from the string to the value
  param->parse(p);
  
  if(filt) {

    if( ! filt->set_parameter(idx) ) {
      error("error setting value %s for parameter %s on filter %s",
	    p, param->name, filt->proto->name);
      return 0;
    }

  } else {

    if( ! lay->set_parameter(idx) ) {
      error("error setting value %s for parameter %s on layer %s",
	    p, param->name, lay->name);
      return 0;
    }

  }
      

  return 1;
}

static int param_completion(char *cmd) {
  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  FilterInstance* filt =
    (FilterInstance*)lay->filters.selected();

  Linklist<Parameter> *parameters;
  if(filt) parameters = &filt->proto->parameters;
  else     parameters = lay->parameters;

  if(!parameters) return 0;

  Parameter **params = parameters->completion(cmd);
  if(!params[0]) return 0;

  Parameter *p;

  if(!params[1]) { // exact match, then fill in command
    p = (Parameter*)params[0];
    //    ::notice("%s :: %s",p->name,p->description);
    snprintf(cmd,MAX_CMDLINE,"%s = ",p->name);
    //    return 1;
  } else {
    
    notice("List available parameters starting with \"%s\"",cmd);

  }

  int c;
  for(c=0; params[c]; c++) {
    p = (Parameter*)params[c];
    switch(p->type) {
    case Parameter::BOOL:
      ::act("(bool) %s = %s ::  %s", p->name,
	    (*(bool*)p->value == true) ? "true" : "false",
	    p->description);
      break;
    case Parameter::NUMBER:
      ::act("(number) %s = %g :: %s", p->name,
	    *(double*)p->value,
	    p->description);
      break;
    case Parameter::STRING:
      ::act("%s (string) %s", p->name, p->description);
      break;
    case Parameter::POSITION:
      {
	double *val = (double*)p->value;
	::act("(position) %s = %g x %g :: %s", p->name,
	      val[0], val[1],
	      p->description);
      }
      break;
    case Parameter::COLOR:
      ::act("%s (color) %s", p->name, p->description);
      break;
    default:
      ::error("%s (unknown) %s", p->name, p->description);
      break;
    }
  }
  return c;
}

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
  Entry **blits;
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
    b = (Blit*) blits[0];
    ::notice("%s :: %s",b->name,b->desc);
    snprintf(cmd,MAX_CMDLINE,"%s",b->name);
    return 1;
  }
  
  notice("List available blits starting with \"%s\"",cmd);
  for(c=0;blits[c];c+=4) {
    char tmp[256];

    b = (Blit*)blits[c];
    if(!b) break;
    snprintf(tmp,256,"%s", b->name);

    b = (Blit*)blits[c+1];
    if(b) {
      strncat(tmp, "\t", 256);
      strncat(tmp, b->name, 256);
    }

    b = (Blit*)blits[c+2];
    if(b) {
      strncat(tmp, "\t", 256);
      strncat(tmp, b->name, 256);
    }
    
    b = (Blit*)blits[c+3];
    if(b) {
      strncat(tmp, "\t", 256);
      strncat(tmp, b->name, 256);
    }

    ::act("%s",tmp);
    
  }
  return c;
}


static int filter_proc(char *cmd) {
  Filter *filt;
  int idx;

  if(!cmd) return 0;

  filt = (Filter*)env->filters.search(cmd, &idx);
  if(!filt) {
    ::error("filter not found: %s",cmd);  
    return 0;
  }

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer selected for effect %s",filt->name);
    return 0;
  }
  
  if( ! filt->apply(lay) ) {
    ::error("error applying filter %s on layer %s",filt->name, lay->name);
    return 0;
  }

  // select automatically the new filter
//  lay->filters.sel(0);
//  ff->sel(true);
  return 1;
}
static int filter_comp(char *cmd) {
  int c;
  Filter **res;
  Filter *filt;
  if(!cmd) return 0;
  // QUAAA
  res = env->filters.completion(cmd);

  if(!res[0]) return 0; // no hit 

  if(!res[1]) { // exact match: fill in the command
    filt = res[0];
    if(!filt) return 0; // doublecheck safety fix
    ::notice("%s :: %s",filt->name,filt->description());
    snprintf(cmd,511,"%s",res[0]->name); c=1;
  } else { // list all matches
    for(c=0;res[c];c+=4) {
      char tmp[256];

      filt = res[c];
      if(!filt) break;
      snprintf(tmp,256,"%s", filt->name);

      filt = res[c+1];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }

      filt = res[c+2];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }

      filt = res[c+3];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }

      //      ::act("%s :: %s",filt->name,filt->description());

      ::act("%s",tmp);
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

static int exec_script(char *cmd) {
  struct stat filestatus;

  func("exec_script(%s)",cmd);

  // check that is a good file
  if( stat(cmd,&filestatus) < 0 ) {
    error("invalid file %s: %s",cmd,strerror(errno));
    return 0;
  } else { // is it a directory?
    if( S_ISDIR( filestatus.st_mode ) ) {
      error("can't open a directory as a script",cmd);
      return 0;
    }
  }

  env->js->open(cmd);

  env->console->refresh();
  return 0;
}

static int exec_script_command(char *cmd) {

  act("> %s",cmd);

  // check that is a good file
  
  env->js->parse(cmd);

  env->console->refresh();
  return 0;
}

static int open_layer(char *cmd) {
  struct stat filestatus;
  int len;

  func("open_layer(%s)",cmd);

  // check that is a good file
  if (strncasecmp(cmd, "/dev/video",10)!=0) {
    if( stat(cmd,&filestatus) < 0 ) {
      error("invalid file %s: %s",cmd,strerror(errno));
      return 0;
    } else { // is it a directory?
      if( S_ISDIR( filestatus.st_mode ) ) {
	error("can't open a directory as a layer",cmd);
	return 0;
      }
    }
  }

  // ok the path in cmd should be good here

  Layer *l = create_layer(env, cmd);
  if(l) {
  /*
    if(!l->init(env)) {
      error("can't initialize layer");
      delete l;
    } else {
    */
    //	  l->set_fps(env->fps_speed);
    //      l->start();
    env->add_layer(l);
    l->active=true;
    l->fps=env->fps_speed;

    len = env->layers.len();
    notice("layer succesfully created, now you have %i layers",len);
    env->console->refresh();
    return len;
  }
  error("layer creation aborted");
  env->console->refresh();
  return 0;
}

#if defined WITH_FT2 && defined WITH_FC
#include <text_layer.h>
static int print_text_layer(char *cmd) {

  ((TextLayer*)env->layers.selected())->print_text(cmd);
  return env->layers.len();

}

static int open_text_layer(char *cmd) {
  TextLayer *txt = new TextLayer();
  if(!txt->init(env)) {
    error("can't initialize text layer");
    delete txt;
    return 0;
  }

  
  txt->print_text(cmd);
  //  txt->start();
  txt->set_fps(0);
  env->add_layer(txt);
  txt->active=true;
  
  notice("layer succesfully created with text: %s",cmd);
  env->console->refresh();
  return env->layers.len();
}
#endif

#ifdef HAVE_DARWIN
static int filebrowse_completion_selector(struct dirent *dir) 
#else
  static int filebrowse_completion_selector(const struct dirent *dir) 
#endif
{
  if(dir->d_name[0]=='.')
    if(dir->d_name[1]!='.')
      return(0); // skip hidden files
  return(1);
}
static int filebrowse_completion(char *cmd) {
  Linklist<Entry> files;
  Entry *e;

  struct stat filestatus;
#ifdef HAVE_DARWIN
  struct dirent **filelist;
#else
  struct dirent **filelist;
#endif
  char path[MAX_CMDLINE];
  char needle[MAX_CMDLINE];
  bool incomplete = false;
  Entry **comps;
  int found;
  int c;

  if(cmd[0]!='/') // path is relative: prefix our location
    snprintf(path,MAX_CMDLINE,"%s/%s",getenv("PWD"),cmd);
  else // path is absolute
    strncpy(path,cmd,MAX_CMDLINE);
  
  if( stat(path,&filestatus) < 0 ) { // no file there?
    
    // parse backwards to the first '/' and zero it,
    // store the word of the right part in needle
    for( c = strlen(path); path[c]!='/' && c>0; c-- );
    strncpy(needle,&path[c+1],MAX_CMDLINE);
    path[c+1] = '\0';
    incomplete = true;

    if( stat(path,&filestatus) < 0) { // yet no valid file?
      error("error on file completion path %s: %s",path,strerror(errno));
      return 0;
    }

  } else { // we have a file!
    
    if( S_ISREG( filestatus.st_mode ) )
      return 1; // is a regular file!
    
    // is it a directory? then append the trailing slash
    if( S_ISDIR( filestatus.st_mode ) ) {
      c = strlen(path);
      if(path[c-1]!='/') {
	path[c] = '/'; path[c+1] = '\0';
      }
    }

    strncpy(cmd,path,MAX_CMDLINE);

  }
  
  // at this point in path there should be something valid
  found = scandir
    (path,&filelist,
     filebrowse_completion_selector,alphasort);

  if(found<0) {
    error("filebrowse_completion: scandir: %s",strerror(errno));
    return 0;
  }
    
  for(c=found-1;c>0;c--) { // insert each entry found in a linklist
    e = new Entry();
    e->set_name(filelist[c]->d_name);
    files.append(e);
  }

  c = 0; // counter for entries found

  if(incomplete) {

    // list all files in directory *path starting with *needle
    
    comps = files.completion(needle);
    if(comps[0]) { // something found
      
      if(!comps[1]) { // exact match

	e = comps[0];
	snprintf(cmd,MAX_CMDLINE,"%s%s",path,e->name);
	  
	c = 1;
	
      } else { // multiple matches

	notice("list of %s* files in %s:",needle,path);	
	for(c=0;comps[c];c++) {
	  ::act(" %s",comps[c]->name);
	}
	
      }
      
    } else c = 0;
    
  } else {

    // list all entries
    notice("list of all files in %s:",path);
    e = files.begin();
    for(c=0, e=files.begin();
	e; e=e->next, c++)
      ::act("%s",e->name);

  }
  // free entries allocated in memory
  e = files.begin();
  while(e) {
    files.rem(1);
    delete e;
    e = files.begin();
  }
  
  return(c);
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

static int generator_completion(char *cmd) {
  Filter **res;
  Filter *filt;
  int c;

  if(!cmd) return 0;
  res = (Filter**)env->generators.completion(cmd);
  if(!res[0]) return 0;

  if(!res[1]) { // exact match: fill in the command
    filt = res[0];
    ::notice("%s :: %s",filt->name,filt->description());
    snprintf(cmd,511,"%s",filt->name); c=1;
  } else { // list all matches
    for(c=0;res[c];c+=4) {
      
      char tmp[256];
      
      filt = res[c];
      if(!filt) break;
      snprintf(tmp,256,"%s", filt->name);
      
      filt = res[c+1];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }
      
      filt = res[c+2];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }
      
      filt = res[c+3];
      if(filt) {
	strncat(tmp, "\t", 256);
	strncat(tmp, filt->name, 256);
      }
      
      //      ::act("%s :: %s",filt->name,filt->description());
      
      ::act("%s",tmp);
    }

  }
  return c;
}

static int create_generator(char *cmd) {
  Layer *tmp = new GenF0rLayer();
  if(!tmp) return 0;
  if(!tmp->init(env)) {
    error("can't initialize generator layer");
    delete tmp;
    return 0;
  }
  if(!tmp->open(cmd)) {
    error("generator %s is not found", cmd);
    delete tmp;
    return 0;
  }
  //  tmp->start();
  tmp->set_fps(env->fps_speed);
  env->add_layer(tmp);
  tmp->active=true;

  //env->console->refresh();	

  notice("generator %s succesfully created", tmp->name);
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
  paramsel = 1;
}

Console::~Console() {
  close();
}

bool Console::init(Context *freej) {
  env = freej;

  setenv("TERM","xterm-color",0); // set if not present (overwrite=0)

  SLtt_get_terminfo();

  SLkp_init(); // keyboard interface

  SLang_init_tty(-1,0,0);

  SLsmg_init_smg(); // screen manager

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

  set_console(this);

  canvas();

  screen_size_changed = false;
  SLsignal (SIGWINCH, sigwinch_handler);
  SLang_set_abort_signal(sigint_handler);

  SLkp_set_getkey_function(getkey_handler);

  SLtt_set_cursor_visibility(0);

  print_help();

  active = true;
  return true;
}

void Console::close() {
  SLtt_set_cursor_visibility(1);
  set_console(NULL);
  SLsmg_reset_smg();
  SLang_reset_tty();
}

/* setup the flags and environment to read a new input
   saves the pointer to the command processing function
   to use it once the input is completed */
int Console::readline(const char *msg,cmd_process_t *proc,cmd_complete_t *comp) {
  ::notice(msg);
  update_scroll();
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


void Console::getkey() {
  int key = SLkp_getkey();
  
//  if(key) ::func("SLkd_getkey: %u",key);
//  else return; /* return if key is zero */
  if(!key) return;

  //  if(input) {
  if(parser == COMMANDLINE) parser_commandline(key);
  else if(parser == MOVELAYER) parser_movelayer(key);
  //  else if(parser == JAZZ) parser_jazz(key);
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

  /* S-Lang says: 
   * All well behaved applications should block signals that
   * may affect the display while performing screen update. */
  SLsig_block_signals ();
  
  if(screen_size_changed) {
    SLtt_get_screen_size ();
    SLsmg_reinit_smg ();
    canvas();
    env->console->refresh();
    screen_size_changed = false;
  }

  /* print info the selected layer */
  if(env->layers.len()) {
    
    layerprint(); // updates the *layer selected pointer
    layerlist(); // print layer list   

  }

  filterprint(); // updates the *filter selected pointer
  filterlist(); // print filter list
  
  if(do_update_scroll)
    update_scroll();

  if(!commandline) {
    //    speedmeter();
    statusline(NULL);
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
    statusline(NULL);
  else
    GOTO_CURSOR;

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

/*
void Console::speedmeter() {
  char tmp[256];
  SLsmg_gotorc(1,1);
  sprintf(tmp,"Running at %.0f fps : ",env->fps);
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_write_string(tmp);
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
*/
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

void Console::layerprint() {
  Layer *layer = (Layer*)env->layers.selected();
  if(!layer) return;

  SLsmg_gotorc(2,1);
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_string((char *)"Layer: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_write_string(layer->get_filename());
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_char(' ');
  SLsmg_write_string((char *)"blit: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_write_string(layer->blitter.current_blit->name);
  SLsmg_write_char(' ');
  SLsmg_printf((char *)"[%.0f]",layer->blitter.current_blit->value);
  SLsmg_write_char(' ');
  SLsmg_set_color(LAYERS_COLOR);
  SLsmg_write_string((char *)"geometry: ");
  SLsmg_set_color(LAYERS_COLOR+10);
  SLsmg_printf((char *)"x%i y%i w%u h%u",
	       layer->geo.x, layer->geo.y,
		 layer->geo.w, layer->geo.h);
  SLsmg_erase_eol();
}

void Console::layerlist() {
  int color;
  Entry *layer = NULL;
  Entry *filter = NULL;

  SLsmg_gotorc(4,1);
  //  env->layers.lock();
  /* take layer selected and first */

  layer = env->layers.selected();
  if(layer) filter = ((Layer*)layer)->filters.selected();

  //  filter = (FilterInstance*)l->filters.selected();
  Layer *l = (Layer *)env->layers.begin();  
  while(l) { /* draw the layer's list */

    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_string((char *)" -> ");

    color=LAYERS_COLOR;
    if( l == layer && !filter) {
      color+=20;
      layercol = SLsmg_get_column();
    }
    
    if(l->fade | l->active) color+=10;

    SLsmg_set_color (color);
    
    SLsmg_printf((char *)"%s",l->get_name());

    l = (Layer *)l->next;
  }
  //  env->layers.unlock();
  SLsmg_set_color(PLAIN_COLOR);
  SLsmg_erase_eol();
}

void Console::filterprint() {

  Layer *layer = (Layer*)env->layers.selected();
  if(!layer) return;

  FilterInstance *filter = (FilterInstance*)layer->filters.selected();

  SLsmg_gotorc(3,1);
  SLsmg_set_color(FILTERS_COLOR);
  SLsmg_write_string((char *)"Filter: ");
  if(!filter) {
    SLsmg_write_string((char *)"none selected");
    SLsmg_set_color(PLAIN_COLOR);
    SLsmg_erase_eol();
    return;
  }
  SLsmg_set_color(FILTERS_COLOR+10);
  SLsmg_write_string(filter->name);
  SLsmg_erase_eol();
  SLsmg_forward(2);
  SLsmg_write_string((char *)filter->proto->description());
  SLsmg_set_color(PLAIN_COLOR);

}
    
void Console::filterlist() {
  Layer *layer;
  FilterInstance *f, *filter;
  int color;
  int pos = 5;
  
  layer = (Layer*)env->layers.selected();
  if(layer) {
    filter = (FilterInstance*)layer->filters.selected();
    
    f = (FilterInstance *)layer->filters.begin();
    while(f) {
      
      SLsmg_set_color(PLAIN_COLOR);
      SLsmg_gotorc(pos,0);
      SLsmg_erase_eol();
      
      SLsmg_gotorc(pos,layercol);
      color=FILTERS_COLOR;
      if( f == filter ) color+=20;
      if( f->active) color+=10;
      SLsmg_set_color (color);
      
      SLsmg_printf((char *)"%s",f->name);
      
      pos++;
      f = (FilterInstance *)f->next;
    }
  }
  SLsmg_set_color(PLAIN_COLOR);
  for(;pos<5;pos++) {
    SLsmg_gotorc(pos,0);
    SLsmg_erase_eol();
  }
}



void Console::notice(const char *msg) {
  scroll(msg,PLAIN_COLOR+10);
}
void Console::warning(const char *msg) {
  scroll(msg,2);
}
void Console::act(const char *msg) {
  scroll(msg,PLAIN_COLOR);
}
void Console::error(const char *msg) {
  scroll(msg,2);
}
void Console::func(const char *msg) {
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

File_Line_Type *Console::create_line (const char *buf)
{
   File_Line_Type *line;
   
   line = (File_Line_Type *) calloc (1, sizeof (File_Line_Type));

   if (!line) return NULL;
   
   line->data = SLmake_string ((char *)buf);   /* use a slang routine */
   if (!line->data) {
     free (line);
     return NULL;
   }
   
   return line;
}


void Console::scroll(const char *msg, int color) {

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
  row = 8; // first upper row
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
      env->console->refresh();
      break;
      
    case KEY_SPACE:
      if(fe) ((FilterInstance*)fe)->active =
	       !((FilterInstance*)fe)->active;
      else  ((Layer*)le)->active =
	      !((Layer*)le)->active;
    break;

    case KEY_CTRL_M: {
		Layer *l=((Layer*)le);
		if (l->fps > 0)
			l->set_fps(0);
		else
			if (l->fps_old > 0)
				l->set_fps(l->fps_old);
			else
				l->set_fps(env->fps_speed);
		l->signal_feed();
		::notice("Layer.set_fps(%f)", l->fps);
	}
	break;

    case KEY_CTRL_E:
      readline("add new Effect - press TAB for completion:",&filter_proc,&filter_comp);
      break;

    case KEY_CTRL_P:
      readline("set parameter - press TAB for completion:",&param_selection, &param_completion);
      break;

    case KEY_CTRL_B:
      readline("select Blit mode for the selected Layer - press TAB for completion:",
	       &blit_selection,&blit_comp);
      break;
      
    case KEY_CTRL_V:
      readline("set Blit value for the selected Layer:",
	       &set_blit_value,NULL);
      break;
      
#if defined WITH_FT2 && defined WITH_FC
    case KEY_CTRL_Y:
      if(((Layer*)le)->type == Layer::TEXT)
	readline("print a new word in Text Layer, type your words:",
		 &print_text_layer,NULL);
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
    readline("execute javascript command:",&exec_script_command,NULL);
    break;
  
  case KEY_CTRL_J:
    readline("load and execute a javascript file:", &exec_script, &filebrowse_completion);
    break;

  case KEY_CTRL_L:
    refresh();
    break;

  case KEY_CTRL_O:
    readline("open a file in a new Layer:",
	     &open_layer,&filebrowse_completion);
    break;

      
    case KEY_CTRL_G:
      readline("create a generator in a new Layer:",
	       &create_generator,&generator_completion);
      break;


#if defined WITH_FT2 && defined WITH_FC
  case KEY_CTRL_T:
    readline("create a new Text Layer, type your words:",
	     &open_text_layer,NULL);
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
  case KEY_PLUS:  layer->blitter.set_zoom( layer->blitter.zoom_x + 0.01,
					   layer->blitter.zoom_y + 0.01); break;
  case KEY_MINUS: layer->blitter.set_zoom( layer->blitter.zoom_x - 0.01,
					   layer->blitter.zoom_y - 0.01); break;
  case 'w':       layer->blitter.set_spin(0,-0.001);    break;
  case 's':       layer->blitter.set_spin(0,0.001);     break;
  case '.':       layer->blitter.set_zoom(1,1);         break;
    
    // rotation
  case '<': layer->blitter.set_rotate( layer->blitter.rotate + 0.5 ); break;
  case '>': layer->blitter.set_rotate( layer->blitter.rotate - 0.5 ); break;
  case 'a': layer->blitter.set_spin(0.02,0);   break;
  case 'd': layer->blitter.set_spin(-0.02,0);  break;
  case ',': layer->blitter.set_rotate(0);      break;
  case 'z': layer->blitter.antialias =
      !layer->blitter.antialias;       break;
    
    
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
    if(command[0]==EOL) {
      parser = DEFAULT;
      cmd_process = NULL;
      cmd_complete = NULL;
      statusline(NULL);
      return;
    }
    statusline(command);
    // otherwise process the input
    res = (*cmd_process)(command);
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
    entr = NULL;
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
    res = (*cmd_complete)(command);
    if(!res) return;
    else if(res==1) { // exact match!
      SLsmg_gotorc(SLtt_Screen_Rows - 1,1);
      SLsmg_write_string(command);
      SLsmg_erase_eol();
      //      cursor = strlen(command);
    }
    update_scroll();
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
  
