/*  FreeJ
 *  (c) Copyright 2001-2003 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <context.h>
#include <keyboard.h>

#include <png_layer.h>
#include <txt_layer.h>

#include <osd.h>
#include <plugger.h>
#include <jutils.h>
#include <lubrify.h>
#include <config.h>

#ifdef WITH_V4L
#include <v4l_layer.h>
#endif

#ifdef WITH_AVIFILE
#include <avi_layer.h>
#endif

#define MAX_CLI_CHARS 4096

/* ================ command line options */

static const char *help = 
" .  Usage: freej [options] [files and video devices]\n"
" .  options:\n"
" .   -h   print this help\n"
" .   -v   version information\n"
" .   -D   debug verbosity level - default 1\n"
" .   -s   set display size - default 400x300\n"
" .   -2   double screen size\n"
" .   -0   start with deactivated layers\n"
" .  files:\n"
" .   you can specify any number of files or devices to be loaded,\n"
" .   this binary is compiled to support the following layer formats:\n"
#ifdef WITH_V4L
" .  - Video4Linux devices as of BTTV cards and webcams\n"
" .    you can specify the size  /dev/video0%160x120\n"
#endif
#ifdef WITH_AVIFILE
" .  - AVI,ASF,WMA,WMV movies as of codecs supported by avifile lib\n"
#endif
" .  - PNG images (also with transparency)\n"
" .  - TXT files rendered with freetype2 library\n"
"\n";

/*
static const struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"debug", required_argument, NULL, 'D'},
  {"size", required_argument, NULL, 's'},
  {"double", no_argument, NULL, '2'},
  {"zero", no_argument, NULL, '0'},
  {0, 0, 0, 0}
};
*/

static const char *short_options = "-hvD:s:20";

int debug;
char layer_files[MAX_CLI_CHARS];
int cli_chars = 0;
int width = 400;
int height = 300;
bool doublesize = false;
bool startstate = true;

void cmdline(int argc, char **argv) {
  int res, c;
  int optlen;

  /* initializing defaults */
  char *p = layer_files;

  debug = 1;

  do {
    res = getopt (argc, argv, short_options);
    switch(res) {
    case 'h':
      fprintf(stderr, "%s", help);
      exit(0);
      break;
    case 'v':
      fprintf(stderr,"\n");
      exit(0);
      break;
    case 'D':
      debug = atoi(optarg);
      if(debug>3) {
	warning("debug verbosity ranges from 1 to 3\n");
	debug = 3;
      }
      break;
    case 's':
      sscanf(optarg,"%ux%u",&width,&height);
      if(width<320) {
	error("display width can't be smaller than 400 pixels");
	width = 320;
      }
      if(height<240) {
	error("display height can't be smaller than 300 pixels");
	width = 240;
      }
      break;

    case '0':
      startstate = false;
      break;

    case '2':
      doublesize = true;
      break;
      
    case '?':
      func("unrecognized option: %s",optarg);
      break;

    case 1:
      {
      }
      break;
	  
    default:
      break;
    }
  } while (res > 0);
  
  func("extra args: optind=%i and argc=%i",optind,argc);
  argc -= optind;
  argv += optind;
  for(c=0;c<argc;c++) {
    optlen = strlen(argv[c]);
    if( (cli_chars+optlen) < MAX_CLI_CHARS ) {
      sprintf(p,"%s#",argv[c]);
      cli_chars+=optlen+1;
      p+=optlen+1;
    } else warning("too much files on commandline, list truncated");
  }

}

/* ===================================== */

int main (int argc, char **argv) {

  Layer *lay = NULL;
#ifdef WITH_V4L
  V4lGrabber *v4l = NULL;
#endif
#ifdef WITH_AVIFILE
  AviLayer *avi = NULL;
#endif
  PngLayer *png = NULL;
  TxtLayer *txt = NULL;

  notice("%s version %s [ http://freej.dyne.org ]",PACKAGE,VERSION);
  act("(c)2001-2003 Denis Rojo < jaromil @ dyne.org >");
  act("----------------------------------------------");
  cmdline(argc,argv);
  set_debug(debug);

  /* sets realtime priority to maximum allowed for SCHED_RR (POSIX.1b)
     this hangs on some kernels
  if(set_rtpriority(true))
    notice("running as root: high priority realtime scheduling allowed.");
  */

  /* this is the output context (screen) */
  Context screen(width,height,32,SDL_HWPALETTE|SDL_HWSURFACE);
  if(screen.surf==NULL) exit(0);
  screen.doublesize(doublesize);

  /* create layers requested on commandline */
  {
    char *l, *p, *pp = layer_files;
    while(cli_chars>0) {
      p = pp;

      while(*p!='#' && cli_chars>0) { p++; cli_chars--; }
      l = p+1;
      if(cli_chars<=0) break; *p='\0';

      /* LIVE VIDEO LAYERS */
      if( strncmp(pp,"/dev/",5)==0 ) {
	unsigned int w=320, h=240;
	while(p!=pp) {
	  if(*p!='%') p--;
	  else { /* size is specified */
	    *p='\0'; p++;
	    sscanf(p,"%ux%u",&w,&h);
	    p = pp; }
	}
#ifdef WITH_V4L
	v4l = new V4lGrabber();
	if(v4l->detect(pp))
	  v4l->init(&screen,w,h);
#else
	error("Video4Linux layer support not compiled");
	act("can't load %s",pp);
#endif
      }
      
      /* AVI LAYERS */
      if( strncmp((p-4),".avi",4)==0
	  | strncmp((p-4),".asf",4)==0
	  | strncmp((p-4),".asx",4)==0
	  | strncmp((p-4),".wma",4)==0
	  | strncmp((p-4),".wmv",4)==0 ) {
#ifdef WITH_AVIFILE 
	avi = new AviLayer();
	if(avi->open(pp))
	  avi->init(&screen);
#else
	error("AVI layer support not compiled");
	act("can't load %s",pp);
#endif
      }	

      /* PNG LAYERS */
      if(strncmp((p-4),".png",4)==0) {
	png = new PngLayer();
	if(png->open(pp))
	  png->init(&screen);
      }

      /* TXT LAYERS */
      if(strncmp((p-4),".txt",4)==0) {
	txt = new TxtLayer();
	if(txt->open(pp))
	  txt->init(&screen);
      }

      pp = l;
    }
  }

#ifdef WITH_V4L
  /* even if not specified on commandline
     try to open the default video device */
  if(!v4l) {
    v4l = new V4lGrabber();
    if(v4l->detect("/dev/video"))
      v4l->init(&screen,320,240);
    else delete v4l;
  }
#endif

  /* this is the Plugin manager */
  Plugger plugger;
  plugger.refresh();

  /* this is the On Screen Display */
  Osd osd;
  osd.init(&screen);
  osd.active();
  set_osd(osd.status_msg); /* let jutils know about the osd */

  /* this is the keyboard listener */
  KbdListener keyb;
  assert( keyb.init(&screen, &plugger) );



  /* if the context has no layers quit here */
  if(screen.layers.len()<1) {
    error("no layers present, quitting");
    screen.close();
    plugger.close();
    act("you should at least load a movie,");
    act("a png image or have a video card");
    act("to see something more (see freej -h)");
    exit(0);
  }

  /* launch layer threads */
  func("rock the house");
  screen.rocknroll(startstate);
  func("OK, rolling");

  keyb.start();

  while(!keyb.quit) {
    /* main loop */

    if(screen.clear_all) {
      screen.clear(); /* clear all the screen */
    } else {
      osd.clean();    /* clear only the OSD */
    }

    lay = (Layer *)screen.layers.end();

    while(lay != NULL) {
      lay->cafudda();
      lay = (Layer *)lay->prev;
    }

    screen.flip();

    screen.calc_fps();
    
  }

  /* quitting */
  screen.close();
  plugger.close();

  act(" ");
  act("You are encouraged to exhibit the output of this software publicly");
  act("as long as this software and its author are fairly mentioned! THANKS!");
  jsleep(2,0);
  act(" ");
  act("bye.");
  exit(1);
}
