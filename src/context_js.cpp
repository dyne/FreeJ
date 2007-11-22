/*  FreeJ
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: freej.cpp 654 2005-08-18 16:52:47Z jaromil $"
 *
 */

#include <dirent.h>

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <jsparser.h>
#include <video_encoder.h>

// global environment class
JSClass global_class = {
  "Freej", JSCLASS_NEW_RESOLVE,
  JS_PropertyStub,  JS_PropertyStub,
  JS_PropertyStub,  JS_PropertyStub,
  JS_EnumerateStub, JS_ResolveStub,
  JS_ConvertStub,   JS_FinalizeStub
};

JSFunctionSpec global_functions[] = {
    /*    name          native			nargs    */
    {"cafudda",         cafudda,                1},
    {"run",             cafudda,                1},
    {"quit",            quit,                   0},
    {"add_layer",	add_layer,		1},
    {"rem_layer",	rem_layer,		1},
    {"list_layers",     list_layers,            0},
    {"selected_layer",  selected_layer,         0},
    {"debug",           debug,                  1},
    {"rand",            rand,                   0},
    {"srand",           srand,                  1},
    {"pause",           pause,                  0},
    {"fullscreen",      fullscreen,             0},
    {"set_resolution",  set_resolution,         2},
    {"scandir",         freej_scandir,          1},
    {"echo",            freej_echo,             1},
    {"echo_func",       freej_echo_func,        1},
    {"strstr",          freej_strstr,           2},
    //    {"stream_start",    stream_start,           0},
    //    {"stream_stop",     stream_stop,            0},
    {"file_to_strings", file_to_strings,        1},
    {"register_controller", register_controller, 1},
    {"register_encoder", register_encoder, 1},
    {"include",         include_javascript,     1},
    {"exec",            system_exec,            1},
    {0}
};


JS(cafudda) {
  //  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  double *pdouble;
  double seconds;
  int isecs;

  if(argc>0) {

    if(JSVAL_IS_DOUBLE(argv[0])) {
      
      // JSVAL_TO_DOUBLE segfault when there's an int as input
      pdouble=JSVAL_TO_DOUBLE(argv[0]);
      seconds = *pdouble;
      
    } else if(JSVAL_IS_INT(argv[0])) {
      
      isecs=JSVAL_TO_INT(argv[0]);
      seconds=(double)isecs;

    }

  } else seconds = 0;

  
  //  func("JsParser :: run for %f seconds",seconds);
  env->cafudda(seconds);

  return JS_TRUE;
}

JS(pause) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  env->pause = !env->pause;

  return JS_TRUE;
}

JS(quit) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 env->quit = true;
 return JS_TRUE;
}


JS(rem_layer) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

    JSObject *jslayer;
    Layer *lay;

    jslayer = JSVAL_TO_OBJECT(argv[0]);
    if(!jslayer) JS_ERROR("missing argument");

    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) JS_ERROR("Layer core data is NULL");

    lay->rem();
    //    lay->quit=true;
    //    lay->signal_feed();
    //    lay->join();

    //    delete lay;
    return JS_TRUE;
}

JS(add_layer) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    Layer *lay;
    JSObject *jslayer;
    *rval=JSVAL_FALSE;

    if(argc<1) JS_ERROR("missing argument");

    jslayer = JSVAL_TO_OBJECT(argv[0]);

    lay = (Layer *) JS_GetPrivate(cx, jslayer);
    if(!lay) JS_ERROR("Layer core data is NULL");

    /** really add layer */
    env->add_layer(lay);
    *rval=JSVAL_TRUE;
    //      env->layers.sel(0); // deselect others
    //      lay->sel(true);
    return JS_TRUE;
}


JS(register_controller) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    Controller *ctrl;
    JSObject *jsctrl;
    *rval=JSVAL_FALSE;

    if(argc<1) JS_ERROR("missing argument");
    jsctrl = JSVAL_TO_OBJECT(argv[0]);

    ctrl = (Controller *)JS_GetPrivate(cx, jsctrl);
    if(!ctrl) JS_ERROR("Controller core data is NULL");

    /// really add controller
    env->register_controller( ctrl );
    *rval = JSVAL_TRUE;

    return JS_TRUE;
}

JS(register_encoder) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    VideoEncoder *enc;
    JSObject *jsenc;
    *rval=JSVAL_FALSE;

    if(argc<1) JS_ERROR("missing argument");
    jsenc = JSVAL_TO_OBJECT(argv[0]);

    enc = (VideoEncoder *)JS_GetPrivate(cx, jsenc);
    if(!enc) JS_ERROR("VideoEncoder core data is NULL");

    /// really add controller
    env->add_encoder( enc );
    *rval = JSVAL_TRUE;

    return JS_TRUE;
}

JS(fullscreen) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  env->screen->fullscreen();
  //  env->clear_all = !env->clear_all;
  return JS_TRUE;
}

JS(set_resolution) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int w = JSVAL_TO_INT(argv[0]);
  int h = JSVAL_TO_INT(argv[1]);
  env->screen->resize(w, h);
  return JS_TRUE;
}

/*
JS(stream_start) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  notice ("Streaming to %s:%u",env->shouter->host(), env->shouter->port());
  act ("Saving to %s", env -> video_encoder -> get_filename());
  env->save_to_file = true;
  return JS_TRUE;
}
JS(stream_stop) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  ::notice ("Stopped stream to %s:%u", env->shouter->host(), env->shouter->port());
  ::act ("Video saved in file %s",env -> video_encoder -> get_filename());
  env->save_to_file = false;
  return JS_TRUE;
}
*/

static int dir_selector(const struct dirent *dir) {
  if(dir->d_name[0]=='.') return(0); // remove hidden files
  return(1);
}
JS(freej_scandir) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  JSObject *arr;
  JSString *str;
  jsval val;
    
  struct dirent **filelist;
  int found;
  int c = 0;
  char *dir;
  
  JS_ARG_STRING(dir,0);
  
  found = scandir(dir,&filelist,dir_selector,alphasort);
  if(found<0) {
    error("scandir error: %s",strerror(errno));
    return JS_TRUE; // fatal error
  }

  arr = JS_NewArrayObject(cx, 0, NULL); // create void array
  if(!arr) return JS_FALSE;

  // now fill up the array  
  while(found--) {
    char tmp[512];
    snprintf(tmp,512,"%s/%s",dir, filelist[found]->d_name);
    str = JS_NewStringCopyZ(cx, tmp); 
    val = STRING_TO_JSVAL(str);    
    JS_SetElement(cx, arr, c, &val );
    c++;
  }

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

JS(freej_echo) {
  char *msg;
  JS_ARG_STRING(msg,0);
  notice(msg); 
  return JS_TRUE;
}

JS(freej_echo_func) {
  char *msg;
  JS_ARG_STRING(msg,0);
  func(msg); 
  return JS_TRUE;
}

JS(freej_strstr) {
  char *haystack;
  char *needle;
  char *res;
  int intval;
  JS_ARG_STRING(haystack,0);
  JS_ARG_STRING(needle,1);
  res = strstr(haystack, needle);
  if(res == NULL)
    intval = 0;
  else intval = 1;
  *rval = INT_TO_JSVAL(intval);
  return JS_TRUE;
}
  

JS(file_to_strings) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  JS_CHECK_ARGC(1);

  JSObject *arr;
  JSString *str;
  jsval val;

  FILE *fd;

  char *buf;
  char *punt;
  char *pword;
  int len;
  int c;

  char *file;
  JS_ARG_STRING(file,0);

  // try to open the file and read it in memory
  fd = ::fopen(file,"r");
  if(!fd) {
    error("file_to_strings failed for %s: %s",file, strerror(errno) );
    *rval = JSVAL_NULL;
    return JS_TRUE;
  }

  // read it all in *buf
  fseek(fd,0,SEEK_END);
  len = ftell(fd);
  rewind(fd);
  buf = (char*)calloc(len,sizeof(char));
  fread(buf,len,1,fd);
  fclose(fd);
  // file is now read in memory

  arr = JS_NewArrayObject(cx, 0, NULL);
  if(!arr) return JS_FALSE;

  punt = buf;
  c = 0;
  // now fill up the array

  while(punt - buf < len) { // parse it until the end
    while(!isgraph(*punt)) // goes forward until it meets a word
      if(punt-buf >= len) // end of chunk reached
	break;
      else punt++;
    
    // word found, now reach its end
    pword = punt;
    while(   isgraph(*punt)
	     && *punt != ' '
	     && *punt != '\0'
	     && *punt != '\n'
	     && *punt != '\r'
	     && *punt != '\t') {
      if(punt-buf >= len) // end of chunk reached
	break;
      else punt++;
    }

    // there is a word to acquire!
    // create the new entry
    str = JS_NewStringCopyN(cx, pword, punt-pword);
    val = STRING_TO_JSVAL(str);
    JS_SetElement(cx, arr, c, &val);
    c++;
  }

  free(buf);

  *rval = OBJECT_TO_JSVAL( arr );
  return JS_TRUE;
}

// debugging commodity
// run freej with -D3 to see this
JS(debug) {
  char *msg;
  
  JS_ARG_STRING(msg,0);
 
  func("%s", msg);

  return JS_TRUE;
}

static uint32_t randval;
JS(rand) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  randval = randval * 1073741789 + 32749;

  *rval = INT_TO_JSVAL(randval);
  /*
  r = rand();

  if(argc<1) *rval = 1+(int)(r/(RAND_MAX+1.0));
  else {
    JS_ARG_NUMBER(max, 0);
    func("randomizing with max %f",max);
    r = 1+(int)(max*r/(RAND_MAX+1.0));
    *rval = INT_TO_JSVAL(r);
  }
  */
  return JS_TRUE;
}

JS(srand) {
  // this is not fast and you'd better NOT use it often
  // to achieve more randomization on higher numbers:
  // u get unpredictably sloow
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  int seed;
  if(argc<1)
    seed = time(NULL);
  else {
    JS_ARG_NUMBER(r,0);
    seed = (int)r;
  }
  randval = seed;

  return JS_TRUE;
}


////////////////////////////////
// Linklist Entry Methods

JS(entry_down) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Layer);

 if(! lay->down() )
   warning("cannot move %s down", lay->name);
 
 return JS_TRUE;
}
JS(entry_up) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Layer);

 if(! lay->up() )
   warning("cannot move %s up", lay->name);

 return JS_TRUE;
}

JS(entry_move) {
 func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

 GET_LAYER(Layer);

 int pos = JSVAL_TO_INT(argv[0]);
 if( ! lay->move(pos) )
   warning("cannot move %s to position %u", lay->name, pos);

 return JS_TRUE;
}

JS(entry_next) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  Layer *res;
  JSObject *objtmp;

  GET_LAYER(Layer);

  res = (Layer*)lay->next;
  if(!res) // if last entry the cycle to the first
    res = (Layer*) lay->list->begin();

  objtmp = JS_NewObject(cx, res->jsclass, NULL, obj);
  JS_SetPrivate(cx, objtmp, (void*) res);

  *rval = OBJECT_TO_JSVAL(objtmp);

  return JS_TRUE;
}

JS(entry_prev) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  Layer *res;
  JSObject *objtmp;

  GET_LAYER(Layer);
  
  res = (Layer*)lay->prev;
  if(!res) // if first entry the cycle to the end
    res = (Layer*) lay->list->end();
  
  objtmp = JS_NewObject(cx, res->jsclass, NULL, obj);
  JS_SetPrivate(cx, objtmp, (void*) res);
  
  *rval = OBJECT_TO_JSVAL(objtmp);
  
  return JS_TRUE;
}

JS(entry_select) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  Entry *tmp;

  GET_LAYER(Layer);

  // select only one
  lay->sel(true);

  // deselects all others in the list
  tmp = lay->prev;
  while(tmp) {
    tmp->sel(false);
    tmp = tmp->prev;
  }
  tmp = lay->next;
  while(tmp) {
    tmp->sel(false);
    tmp = tmp->next;
  }
  
  return JS_TRUE;
}

JS(include_javascript) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  
  FILE *fd;
  char *jscript;

  JS_ARG_STRING(jscript,0);
  
  fd = ::fopen(jscript,"r");
  if(!fd) { 
    error("include failed for %s: %s", jscript, strerror(errno) );
    JS_ReportErrorNumber( 
        cx, JSFreej_GetErrorMessage, NULL,
        JSSMSG_FJ_NOLUCK,
        jscript, strerror(errno)
    );
    return JS_FALSE;
  }

  fclose(fd);
  if (env->js->open(jscript) == 0) { 
    // JS_EvaluateScript failed, maybe a 
    // syntax error. JS_ReportError was already called,
    // so no need to throw something here.
      func("%s eval failed", __func__);
      return JS_FALSE;
  } 

  return JS_TRUE;
}

JS(system_exec) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  unsigned int c;
  char *prog;
  char **args;

  // get the executable program
  JS_ARG_STRING(prog, 0);

  // get the arguments in a NULL terminated array
  args = (char**)calloc(argc +2, sizeof(char*));
  
  for (c=1; c<argc; c++) {

    if( JSVAL_IS_STRING(argv[c]) )

      args[c] = JS_GetStringBytes( JS_ValueToString(cx, argv[c]) );

    else {

      JS_ReportError(cx,"%s: argument %u is not a string",__FUNCTION__, c);
      env->quit = true;
      return JS_FALSE;

    }

  }

  /* execvp(3) functions provide an array of pointers to
     null-terminated strings that represent the argument list
     available to the new program.  The first argument, by convention,
     should point to the file name associated with the file being
     executed.  The array of pointers must be terminated by a NULL
     pointer.  */
  execvp( prog, args );
  
  return JS_TRUE;
}
