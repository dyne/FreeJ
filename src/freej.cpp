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
#include <errno.h>

#include <context.h>

#include <osd.h>
#include <video_encoder.h>
#include <plugger.h>
#include <jutils.h>
#include <config.h>

#include <impl_layers.h>

/* controller interfaces */
#ifdef WITH_GLADE2
#include <gtk_ctrl.h>
#endif
#ifdef WITH_GTK2
#include <gtk/gtk.h>
#endif
#ifdef WITH_JOYSTICK
#include <joy_ctrl.h>
#endif
#ifdef WITH_MIDI
#include <midi_ctrl.h>
#endif
#ifdef WITH_JAVASCRIPT
#include <jsparser.h>
#endif

//[of]cli options parser:commandline
#define MAX_CLI_CHARS 4096

/* ================ command line options
   (scroll down about 100 lines for the real stuff)
 */

static const char *help =
" .  Usage: freej [options] [layers]\n"
" .\n"
" .   -h   print this help\n"
" .   -v   version information\n"
" .   -D   debug verbosity level - default 1\n"
#ifdef WITH_GLADE2
" .   -g   start with GTK graphical interface (deprecated)\n"
#endif
" .   -s   size of screen - default 400x300\n"
//" .   -m   software magnification: 2x,3x\n"
" .   -n   start with deactivated layers\n"
" .   -c   no interactive text console\n"
#ifdef WITH_OPENGL
" .   -g   experimental opengl engine!(better to use power of 2 resolution as 256x256)\n"
#endif
" .   -j   <javascript.js>  process javascript command file\n"
#ifndef WITH_JAVASCRIPT
" .                      ( disabled!, download spidermonkey \n"
" .        http://ftp.mozilla.org/pub/mozilla.org/js/js-1.5-rc6a.tar.gz\n"
" .        and compile freej with --with-javascript=<path_to_spidermonkey> )"
#endif
"\n"
" .   -e   <file.ogg>  set filename of local encoded ogg-theora file\n"
" .		       if a number is given, the file descriptor selected is used\n"
" .                    (default freej.ogg, start and stop it with CTRL-s)\n"
" .\n"
#ifndef WITH_OGGTHEORA
" (disabled! make sure you have installed correctly ogg http://www.vorbis.com/download.psp\n"
" .        and theora http://theora.org/download.html )\n"
" .\n"
#endif
" .   Streaming options:\n"
" .   -i   <server:port/mount.ogg> stream to server[:port] (default http://localhost:8000/freej.ogg)\n"
" .   -p   <password> mountpoint on server (default hackme)\n"
" .   -a   don't stream or save audio from audio in (select it with aumix)\n"
" .   -t   <name> name of the stream(default \"Streaming with freej\") \n"
" .   -d   <description> description of the stream for icecast server(default \"Free the veejay in you\")\n"
" .   -q   <theora_quality> quality of video encoding (range 0 - 63, default 16\n"
" .                   0 low quality less bandwidth, 63 high quality more bandwidth)\n"
//" .   -q    <vorbis_quality> quality of vorbis encoding (range 0 - 63, default 16, 0\n"
" .\n"
" .  Layers available:\n"
" .   you can specify any number of files or devices to be loaded,\n"
" .   this binary is compiled to support the following layer formats:\n";

// we use only getopt, no _long
static const char *short_options = "-hvD:gs:nj:e:i:cp:t:d:q:ag";



int debug;
char layer_files[MAX_CLI_CHARS];
int cli_chars = 0;
int width = 400;
int height = 300;
int magn = 0;
char javascript[512]; // script filename

static char encoded_filename[512]; // filename to save to

/* icecast streaming options */
static char screaming_url[512]; // s(c|t)reaming url ? :P
static char screaming_pass[512]; // password
static char screaming_name[512]; // name
static char screaming_description[512]; // name

static int theora_quality;

bool startstate = true;
bool stream_audio = true;
bool gtkgui = false;
bool opengl = false;
bool noconsole = false;

void cmdline(int argc, char **argv) {
  int res, optlen;

  /* initializing defaults */
  char *p                  = layer_files;
  javascript[0]            = 0;

  encoded_filename[0]      = '\0';
  screaming_url[0]         = '\0';
  screaming_pass[0]        = '\0';
  screaming_name[0]        = '\0';
  screaming_description[0] = '\0';

  theora_quality           = -1;

  debug                    = 1;

  do {
//    res = getopt_long (argc, argv, short_options); TODO getopt_long
    res = getopt(argc, argv, short_options);
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

#ifdef WITH_GLADE2
    case 'g':
      gtkgui = true;
      break;
#endif

    case 's':
      sscanf(optarg,"%ux%u",&width,&height);
      /* what the fuck ???
      if(width<320) {
	error("display width can't be smaller than 400 pixels");
	width = 320;
      }
      if(height<240) {
	error("display height can't be smaller than 300 pixels");
	width = 240;
      }
      */
      break;

    case 'm':
      sscanf(optarg,"%u",&magn);
      magn -= 1;
      magn = (magn>3) ? 3 : (magn<1) ? 0 : magn;
      break;

    case 'n':
      startstate = false;
      break;

     case 'e':
	snprintf (encoded_filename, 512, "%s", optarg);
      break;

     case 'i':
	snprintf (screaming_url, 512, "%s", optarg);
      break;

    case 'c':
      noconsole = true;
      break;

     case 'p':
	snprintf (screaming_pass, 512, "%s", optarg);
      break;

     case 't':
	snprintf (screaming_name, 512, "%s", optarg);
      break;
      
     case 'd':
	snprintf (screaming_description, 512, "%s", optarg);
      break;

     case 'a':
	stream_audio = false;
      break;

     case 'q':
      sscanf(optarg,"%u",&theora_quality);

      break;

   case 'j':
      FILE *fd;
      fd = fopen(optarg,"r");
      if(!fd) error("can't open %s: %s",javascript,strerror(errno));
      else {
	snprintf(javascript,512,"%s",optarg);
	fclose(fd);
      }
      break;

#ifndef WITH_OPENGL
   case 'g':
      opengl=true;
      break;
#endif

      
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
//[cf]


int main (int argc, char **argv) {

  /* this is the output context (screen) */
  Context freej;

  Layer *lay = NULL;

  notice("%s version %s   RASTA SOFTWARE",PACKAGE,VERSION);
  act("(c)2001-2005 Jaromil & Kysucix @ dyne.org");
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

  assert( freej.init(width,height, opengl) );

  // refresh the list of available plugins
  freej.plugger.refresh();

#ifdef WITH_JAVASCRIPT
  /* execute javascript */
  if( javascript[0] ) {
    freej.interactive = false;
    freej.js->open(javascript);
    if(freej.quit) {
      freej.close();
      exit(1);
    } else freej.interactive = true;
  }
#endif



  /* initialize the S-Lang text Console */
  if(!noconsole) {
    if( getenv("TERM") ) {
      freej.console = new Console();
      freej.console->init( &freej );
    }
  }

  /* initialize the Keyboard Listener */
  freej.kbd.init( &freej );

  /* initialize the On Screen Display */
  freej.osd.init( &freej );

  /* initialize encoded filename */
  if (encoded_filename[0] != '\0') {
	  freej.video_encoder -> handle_audio (stream_audio );
	  freej.video_encoder -> set_output_name (encoded_filename );
  }

  /*
   * streaming options 
   */
  if (screaming_url[0] != '\0') {
	  char *port = strrchr (screaming_url, ':');
	  char *slash;
	  char *mount;

	  if (port) {
		  slash = strchr (port, '/');
		  if(slash) {
			  *slash = '\0';
			  slash++;
			  func("Mount point is %s",slash);
		  }

		  func("Port to stream is %s", port+1);
		  freej.shouter -> port (atoi(port+1));
		  *port = '\0';
	  } 
	  mount = strrchr(screaming_url,'/') + 1;
	  func("URL %s", mount);
	  freej.shouter -> host (mount);
	  freej.shouter -> mount (slash );
	  freej.video_encoder -> handle_audio (stream_audio );
  }

  if (screaming_name[0] != '\0')
	  freej.shouter -> name (screaming_name );

  if (screaming_description[0] != '\0')
	  freej.shouter -> desc (screaming_description );

  if (screaming_pass[0] != '\0')
	  freej.shouter -> pass (screaming_pass );

  if (theora_quality > 0)
	  freej.video_encoder -> set_video_quality (theora_quality );

  
  freej.shouter -> apply_profile ( );

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

#ifdef WITH_JOYSTICK
  /* launches the joystick controller thread
     if any joystick is connected */
  JoyControl *joystick = new JoyControl();
  if(! joystick->init(&freej) ) delete joystick;
#endif

#ifdef WITH_MIDI
  MidiControl *midi = new MidiControl();
  if(! midi->init(&freej) ) delete midi;
#endif



#ifdef WITH_GLADE2
  /* check if we have an X11 display running */
  if(!getenv("DISPLAY")) gtkgui = false;
  if(gtkgui) {
    /* this launches glade2 interface controller thread
       this interface is completely asynchronous to freej */
    gtk_ctrl_init(&freej,&argc,argv);
  }
#endif


  /* apply screen magnification */
  //  freej.magnify(magn);
  // deactivated for now



  /* MAIN loop */
  while( !freej.quit )
    /* CAFUDDARE in sicilian means to do the bread
       or the pasta for the pizza. it's an intense
       action for your arms, processing materia */
    freej.cafudda(1.0);

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
