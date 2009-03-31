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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <console_calls_ctrl.h>
#include <console_readline_ctrl.h>
#include <console_ctrl.h>
#include <parameter.h>
#include <context.h>
#include <layer.h>


#include <jutils.h>

int console_param_selection(Context *env, char *cmd) {
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

 int console_param_completion(Context *env, char *cmd) {
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
int console_blit_selection(Context *env, char *cmd) {
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
int console_blit_completion(Context *env, char *cmd) {
  int c;
  Entry **blits;
  Blit *b;

  if(!cmd) return 0;

  Layer *lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }

  blits = lay->screen->blitter->blitlist.completion(cmd);

  if(!blits[0]) return 0; // none found

  if(!blits[1]) { // exact match, then fill in command
    b = (Blit*) blits[0];
    ::notice("%s :: %s",b->name,b->desc);
    snprintf(cmd,MAX_CMDLINE,"%s",b->name);
    return 1;
  }
  
  if(cmd[0]==0x0)
    notice("List available blits");
  else
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

int console_blit_param_selection(Context *env, char *cmd) {
  Parameter *param;
  Blit *b;
  int idx;

  Layer *lay = (Layer*)env->layers.selected();
  
  if(!cmd) return 0;
  if(!strlen(cmd)) return 0;
  lay = (Layer*)env->layers.selected();
  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  b = lay->current_blit;
  if(!b) {
    ::error("no blit selected on layer %s",lay->name);
    return 0;
  }
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
  param = (Parameter*)b->parameters.search(cmd, &idx);
  if(! param) {
    error("parameter %s not found in blit %s", cmd, b->name);
    return 0;
  } else
    func("parameter %s found in blit %s at position %u",
	 param->name, b->name, idx);

  param->parse(p);
  return 1;

}

int console_blit_param_completion(Context *env, char *cmd) {
  Parameter *p, **params;
  Blit *b;

  Layer *lay = (Layer*)env->layers.selected();

  if(!lay) {
    ::error("no layer currently selected");
    return 0;
  }
  b = lay->current_blit;
  if(!b) {
    ::error("no blit selected on layer %s",lay->name);
    return 0;
  }
  params = b->parameters.completion(cmd);
  if(!params[0]) return 0;

  if(!params[1]) { // exact match, then fill in command
    p = (Parameter*)params[0];
    //    ::notice("%s :: %s",p->name,p->description);
    snprintf(cmd,MAX_CMDLINE,"%s = ",p->name);
    //    return 1;
  } else {

    if(cmd[0]==0x0)
      notice("List available blit parameters",cmd);
    else
      notice("List available blit parameters starting with \"%s\"",cmd);

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

int console_filter_selection(Context *env, char *cmd) {
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
int console_filter_completion(Context *env, char *cmd) {
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

int console_exec_script(Context *env, char *cmd) {
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

  env->open_script(cmd);

  return 0;
}

int console_exec_script_command(Context *env, char *cmd) {

  act("> %s",cmd);

  // check that is a good file
  
  env->parse_js_cmd(cmd);

  return 0;
}

int console_open_layer(Context *env, char *cmd) {
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
    l->start();
    env->add_layer(l);
    l->active=true;
    //    l->fps=env->fps_speed;

    len = env->layers.len();
    notice("layer succesfully created, now you have %i layers",len);
    return len;
  }
  error("layer creation aborted");
  return 0;
}

#if defined WITH_FT2 && defined WITH_FC
#include <text_layer.h>
int console_print_text_layer(Context *env, char *cmd) {

  ((TextLayer*)env->layers.selected())->print_text(cmd);
  return env->layers.len();

}

int console_open_text_layer(Context *env, char *cmd) {
  TextLayer *txt = new TextLayer();
  if(!txt->init(env)) {
    error("can't initialize text layer");
    delete txt;
    return 0;
  }

  
  txt->print_text(cmd);
  txt->start();
  //  txt->set_fps(0);
  env->add_layer(txt);
  txt->active=true;
  
  notice("layer succesfully created with text: %s",cmd);
  return env->layers.len();
}
#endif

#ifdef HAVE_DARWIN
int filebrowse_completion_selector(struct dirent *dir) 
#else
  int filebrowse_completion_selector(const struct dirent *dir) 
#endif
{
  if(dir->d_name[0]=='.')
    if(dir->d_name[1]!='.')
      return(0); // skip hidden files
  return(1);
}
int console_filebrowse_completion(Context *env, char *cmd) {
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
  func("file completion: %s",cmd);
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

// static int set_blit_value(Context *env, char *cmd) {
//   int val;
//   int c;
//   if(!sscanf(cmd,"%u",&val)) {
//     error("error parsing input: %s",cmd);
//     return 0;
//   }
//   func("value parsed: %s in %d",cmd,val);
//   Layer *lay = (Layer*)env->layers.begin();
//   if(!lay) return 0;
//   /* set value in all blits selected
//      (supports multiple selection) */
//   for(c=0 ; lay ; lay = (Layer*)lay->next) {
//     if(!lay->select) continue;
//     //    lay->blitter.fade_value(1,val);
//   }

//   return 1;
// }

int console_generator_completion(Context *env, char *cmd) {
  Filter **res;
  Filter *filt;
  int c;

  if(!cmd) return 0;
  func("generator completion for %s",cmd);
  res = env->generators.completion(cmd);
  if(!res[0]) {
    func("nothing found");
    return 0;
  }

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

int console_generator_selection(Context *env, char *cmd) {
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

  tmp->start();
  //  tmp->set_fps(env->fps_speed);
  env->add_layer(tmp);
  tmp->active=true;

  notice("generator %s succesfully created", tmp->name);
  return 1;
}
