/*  FreeJ
 *  (c) Copyright 2001-2002 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#include <iostream.h>

#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <getopt.h>

#include <context.h>
#include <keyboard.h>
#include <v4l_layer.h>


#include <png_layer.h>

#include <osd.h>
#include <plugger.h>
#include <jutils.h>
#include <lubrify.h>
#include <config.h>

#ifdef WITH_AVIFILE
#include <avi_layer.h>
#endif

/* BE WARNED: hires mode is not supported, if you change here
   you get strange results. */
#define LORES 1

#ifdef LORES
#define W 400
#define H 300
#define D 32
#define GW 320
#define GH 240
#else
#define W 640
#define H 480
#define D 32
#define GW 512
#define GH 384
#endif

#define MAX_CLI_CHARS 4096

/* ================ command line options */

static const char *help = 
" .  Usage: freej [options]\n"
" .  options:\n"
" .  -h --help     print this help\n"
" .  -v --version  version information\n"
" .  -d --device   video grabbing device - default /dev/video\n"
" .  -D --debug    debug verbosity level - default 1\n"
"\n";

static const struct option long_options[] = {
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'v'},
  {"device", required_argument, NULL, 'd'},
  {"debug", required_argument, NULL, 'D'},
  {0, 0, 0, 0}
};

static const char *short_options = "-hvd:D:";

char *v4ldevice;
int debug;
char layer_files[MAX_CLI_CHARS];
int cli_chars = 0;

void cmdline(int argc, char **argv) {
  int res;

  /* initializing defaults */
  v4ldevice = strdup("/dev/video");
  char *p = layer_files;

  debug = 1;

  do {
    res = getopt_long (argc, argv, short_options, long_options, NULL);
    switch(res) {
    case 'h':
      fprintf(stderr, "%s", help);
      exit(0);
      break;
    case 'v':
      cerr << endl;
      exit(0);
      break;
    case 'd':
      free(v4ldevice);
      v4ldevice = strdup(optarg);
      break;
    case 'D':
      debug = atoi(optarg);
      if(debug>3) {
	warning("debug verbosity ranges from 1 to 3\n");
	debug = 3;
      }
      break;

    case 1:
      {
	int optlen = strlen(optarg);
	if( (cli_chars+optlen) < MAX_CLI_CHARS ) {
	  sprintf(p,"%s#",optarg);
	  cli_chars+=optlen+1;
	  p+=optlen+1;
	} else warning("too much files on commandline, list truncated");
      }
      break;
	  
    default:
      break;
    }
  } while (res > 0);

}

/* ===================================== */

int main (int argc, char **argv) {

  Layer *lay = NULL;
#ifdef WITH_AVIFILE
  AviLayer *avi = NULL;
#endif
  PngLayer *png = NULL;

  notice("%s version %s [ http://freej.dyne.org ]",PACKAGE,VERSION);
  act("(c)2001-2002 Denis Rojo < jaromil @ dyne.org >");
  cmdline(argc,argv);
  set_debug(debug);

  /* sets realtime priority to maximum allowed for SCHED_RR (POSIX.1b) */
  if(set_rtpriority(true))
    notice("running as root: high priority realtime scheduling allowed.");

  /* this is the output context (screen) */
  Context screen(W,H,D,SDL_HWPALETTE|SDL_HWSURFACE|SDL_DOUBLEBUF);
  if(screen.surf==NULL) exit(0);

  /* create layers requested on commandline */
  {
    char *p, *pp = layer_files;
    while(cli_chars>0) {
      p = pp;
      while(*p!='#' && cli_chars>0) {
	p++; cli_chars--; }
      if(cli_chars<=0) break; *p='\0';

      /* AVI LAYERS */
      if( strncmp((p-4),".avi",4)==0
	  | strncmp((p-4),".asf",4)==0
	  | strncmp((p-4),".wma",4)==0
	  | strncmp((p-4),".wmv",4)==0 )
	{
#ifdef WITH_AVIFILE
	avi = new AviLayer();
	if(avi->open(pp))
	  assert( avi->init(&screen) );
#else
	error("AVI layer support not compiled");
	act("can't load %s",pp);
#endif
      }	

      /* PNG LAYERS */
      if(strncmp((p-4),".png",4)==0) {
	png = new PngLayer();
	if(png->open(pp))
	  assert( png->init(&screen) );
      }

      pp = p+1;
    }
  }

  /* ================= Video4Linux layer */
  V4lGrabber grabber;
  /* detect v4l grabber layer */
  if(grabber.detect(v4ldevice))
    assert( grabber.init(&screen,GW,GH) );
  else
    act("a video 4 linux device is not present: no live video");

  /* if the context has no layers quit here */
  if(screen.layers.len()<1) {
    error("no layers present, quitting");
    screen.close();
    exit(0);
  }

  /* this is the Plugin manager */
  Plugger plugger(screen.bpp);
  plugger.refresh();

  /* this is the On Screen Display */
  Osd osd;
  osd.init(&screen);
  osd.active();
  /* let jutils know about the osd */
  set_osd(osd.status_msg);


  /* this is the keyboard listener */
  KbdListener keyb;
  assert( keyb.init(&screen, &plugger) );

  screen.rocknroll();

  keyb.start();

  while(!keyb.quit) {
    /* main loop */


    if(screen.clear_all)
      clearscr(screen.get_surface(),screen.size);

    lay = (Layer *)screen.layers.end();

    while(lay != NULL) {
      lay->cafudda();
      lay = (Layer *)lay->prev;
    }

    screen.calc_fps();

    screen.flip();
  }

  /* quitting */
  osd.splash_screen();
  screen.flip();
  grabber.quit = true;
#ifdef WITH_AVIFILE
  if(avi) avi->quit = true;
#endif
  /* we need to wait here to be sure the threads quitted:
     don't use join because threads are detached for better performance */
  SDL_Delay(3000);
  
  /* this calls all _delete() methods to safely free all memory */
  plugger.close();
  screen.close();

  /*
#ifdef WITH_AVIFILE
  if(avi) delete avi;
#endif
  */
  exit(1);
}
