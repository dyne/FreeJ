/*  FreeJ
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <osd.h>
#include <plugger.h>
#include <jutils.h>
#include <config.h>

/* controller interfaces */
#ifdef WITH_GLADE2
#include <gtk_ctrl.h>
#endif
#include <joy_ctrl.h>
#ifdef WITH_MIDI
#include <midi_ctrl.h>
#endif
#ifdef WITH_JAVASCRIPT
#include <jsparser.h>
#endif

#define MAX_CLI_CHARS 4096

/* ================ command line options
   (scroll down about 100 lines for the real stuff)
 */

static const char *help = 
" .  Usage: freej [options] [layers]\n"
" .  options:\n"
" .   -h   print this help\n"
" .   -v   version information\n"
" .   -D   debug verbosity level - default 1\n"
" .   -C   start without graphical interface\n"
" .   -s   size of screen - default 400x300\n"
" .   -m   software magnification: 2x,3x\n"
" .   -n   start with deactivated layers\n"
#ifdef WITH_JAVASCRIPT
" .   -j   process javascript command file\n"
#endif
" .  layers available:\n"
" .   you can specify any number of files or devices to be loaded,\n"
" .   this binary is compiled to support the following layer formats:\n";

// we use only getopt, no _long 
static const char *short_options = "-hvD:Cs:m:nj:";

int debug;
char layer_files[MAX_CLI_CHARS];
int cli_chars = 0;
int width = 400;
int height = 300;
int magn = 0;
char javascript[512]; // script filename

bool startstate = true;
#ifdef WITH_GLADE2
bool gtkgui = true;
#else
bool gtkgui = false;
#endif

void cmdline(int argc, char **argv) {
  int res, optlen;

  /* initializing defaults */
  char *p = layer_files;
  javascript[0] = 0;
  debug = 1;

  do {
    res = getopt (argc, argv, short_options);
    switch(res) {
    case 'h':
      fprintf(stderr, "%s", help);
      fprintf(stderr, "%s", layers_description);
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
    case 'C':
      gtkgui = false;
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

    case 'm':
      sscanf(optarg,"%u",&magn);
      magn -= 1;
      magn = (magn>3) ? 3 : (magn<1) ? 0 : magn;
      act("CAZ %i",magn);
      break;

    case 'n':
      startstate = false;
      break;

    case 'j':
      snprintf(javascript,512,"%s",optarg);
      {
	FILE *fd;
	fd = fopen(javascript,"r");
	if(!fd)
	  error("can't open %s: %s",javascript,strerror(errno));
	else
	  fclose(fd);
      }
      break;

    case '?':
      warning("unrecognized option: %s",optarg);
      break;

    case 1:
      optlen = strlen(optarg);
      if( (cli_chars+optlen) < MAX_CLI_CHARS ) {
	sprintf(p,"%s#",optarg);
	cli_chars+=optlen+1;
	p+=optlen+1;
      } else warning("too much files on commandline, list truncated");
      break;

    default:
      // act("received commandline parser code %i with optarg %s",res,optarg);
      break;
    }
  } while (res != -1);

#ifdef HAVE_DARWIN
  for(;optind<argc;optind++) {
    optlen = strlen(argv[optind]);
    if( (cli_chars+optlen) < MAX_CLI_CHARS ) {
      sprintf(p,"%s#",argv[optind]);
	cli_chars+=optlen+1;
	p+=optlen+1;
      } else warning("too much files on commandline, list truncated");
  }
#endif
}

/* ===================================== */

int main (int argc, char **argv) {

  Layer *lay = NULL;

  notice("%s version %s [ http://freej.dyne.org ]",PACKAGE,VERSION);
  act("(c)2001-2004 Denis Rojo < jaromil @ dyne.org >");
  act("----------------------------------------------");
  cmdline(argc,argv);
  set_debug(debug);

  /* sets realtime priority to maximum allowed for SCHED_RR (POSIX.1b)
     this hangs on some linux kernels - darwin doesn't even bothers with it
     anybody knows what's wrong when you turn it on? ouch! it hurts :|
     set_rtpriority is inside jutils.cpp 
     if(set_rtpriority(true))
     notice("running as root: high priority realtime scheduling allowed.");
  */


  /* this is the output context (screen) */
  Context freej;
  assert( freej.init(width,height) );

  /* refresh the list of available plugins */
  freej.plugger.refresh();


#ifdef WITH_JAVASCRIPT
  /* execute javascript */
  if(javascript[0]) {
    freej.interactive = false;
    freej.js->open(javascript);
    if(freej.quit) {
      freej.close();
      exit(1);
    } else freej.interactive = true;
  }
#endif

    
  
  /* initialize the S-Lang text Console */
  freej.console = new Console();
  freej.console->init( &freej );
  
  /* initialize the Keyboard Listener */
  freej.kbd.init( &freej );

  /* initialize the On Screen Display */
  freej.osd.init( &freej );

  freej.start_running = startstate;

  /* create layers requested on commandline */
  {
    char *l, *p, *pp = layer_files;
    while(cli_chars>0) {

      p = pp;

      while(*p!='#' && cli_chars>0) { p++; cli_chars--; }
      l = p+1;
      if(cli_chars<=0) break; *p='\0';

      lay = create_layer(pp);
      if(lay) {
	lay->init(&freej);
	freej.layers.add(lay);
      }
      pp = l;
    }
  }

  /* even if not specified on commandline
     try to open the default video device

     ok, we don't do it anymore now //0.7-cvs

     lay = create_layer("/dev/video0");
     if(lay) {
     lay->init(&freej);
     freej.layers.add(lay);
     } */

  /* launches the joystick controller thread
     if any joystick is connected */
  JoyControl *joystick = new JoyControl();
  if(! joystick->init(&freej) ) delete joystick;


#ifdef WITH_MIDI
  MidiControl *midi = new MidiControl();
  if(! midi->init(&freej) ) delete midi;
#endif



#ifdef WITH_GLADE2
  /* check if we have an X11 display running */
  if(!getenv("DISPLAY")) gtkgui = false;
  /* this launches gtk2 interface controller thread
     this interface is completely asynchronous to freej */
  if(gtkgui) gtk_ctrl_init(&freej,&argc,argv);
#endif




  /* apply screen magnification */
  freej.magnify(magn);



  /* MAIN loop */
  while( !freej.quit )
    /* CAFUDDARE in sicilian means to do the bread
       or the pasta for the pizza. it's an intense
       action for your arms, processing materia */
    freej.cafudda(1);

  /* also layers have the cafudda() function
     which is called by the Context class (freej instance here)
     so it's a tree of cafudda calls originating from here
     all synched to the environment, yea, feels good */
     




  
  /* quit */

#ifdef WITH_GLADE2
  if(gtkgui) gtk_ctrl_quit();
#endif

  freej.close();

  notice("cu on http://freej.dyne.org");
  jsleep(1,0);

  exit(1);
}
