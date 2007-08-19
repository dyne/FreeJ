/*  FreeJ
 *  (c) Copyright 2001-2007 Denis Rojo <jaromil@dyne.org>
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
#include <dirent.h>
#include <assert.h>
#include <errno.h>

#include <context.h>

#include <osd.h>
#include <video_encoder.h>
#include <audio_input.h>
#include <plugger.h>
#include <jutils.h>
#include <config.h>

#include <impl_layers.h>
#include <impl_video_encoders.h>

// javascript
#include <jsparser.h>


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
" .   -s   size of screen - default 400x300\n"
//" .   -m   software magnification: 2x,3x\n"
" .   -n   start with deactivated layers\n"
" .   -c   no interactive text console\n"
" .   -f   <frame_per_second>  select global fps for freej\n"
" .   -F   start in fullscreen\n"
#ifdef WITH_OPENGL
" .   -g   experimental opengl engine! (better pow(2) res as 256x256)\n"
#endif
" .   -j   <javascript.js>  process javascript command file\n"
" .   -e   <file.ogg>  set filename of local encoded ogg-theora file\n"
" .		       if a number is given, the file descriptor selected is used\n"
" .                    (default freej.ogg, start and stop it with CTRL-w)\n"
" .\n"
" .   Streaming options:\n"
" .   -i   <server:port/mount.ogg> stream to server (default http://localhost:8000/freej.ogg)\n"
" .   -p   <password> mountpoint on server (default hackme)\n"
" .   -a   don't stream or save audio from audio in (select it with aumix)\n"
" .   -t   <name> name of the stream (\"Streaming with freej\") \n"
" .   -d   <description> description of the stream (\"Free the veejay in you\")\n"
" .   -T   <theora_quality> quality of video encoding (range 0 - 63, default 16\n"
" .                   0 low quality less bandwidth, 63 high quality more bandwidth)\n"
" .   -V   <vorbis_quality> quality of audio encoding (range from -1 to 10, default 1\n"
" .                   -1 lowest quality, smallest file)\n"
//" .   -q    <vorbis_quality> quality of vorbis encoding (range 0 - 63, default 16, 0\n"
" .\n"
" .  Layers available:\n"
" .   you can specify any number of files or devices to be loaded,\n"
" .   this binary is compiled to support the following layer formats:\n";

// we use only getopt, no _long
static const char *short_options = "-hvD:gs:nj:e:i:cp:t:d:T:V:agf:F";

/* this is the global FreeJ context */
Context freej;


int   debug_level = 0;
char  layer_files[MAX_CLI_CHARS];
int   cli_chars = 0;
int   width = 400;
int   height = 300;
int   magn = 0;
char  javascript[512]; // script filename

static char encoded_filename[512]; // filename to save to

/* icecast streaming options */
static char screaming_url[512]; // s(c|t)reaming url ? :P
static char screaming_pass[512]; // password
static char screaming_name[512]; // name
static char screaming_description[512]; // name

static int theora_quality;
static int vorbis_quality;

static unsigned int fps = 25;

bool startstate = true;
bool stream_audio = true;
bool gtkgui = false;
bool opengl = false;
bool noconsole = false;
bool fullscreen = false;

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
  vorbis_quality           = -1;

  debug_level              = 0;

  do {
//    res = getopt_long (argc, argv, short_options); TODO getopt_long
    res = getopt(argc, argv, short_options);
    switch(res) {
    case 'h':
      fprintf(stdout, "%s", help);
      fprintf(stdout, "%s", layers_description);
      exit(0);
      break;
    case 'v':
      fprintf(stderr,"\n");
      exit(0);
      break;
    case 'D':
      debug_level = atoi(optarg);
      if(debug_level>3) {
	warning("debug verbosity ranges from 1 to 3\n");
	debug_level = 3;
      }
      break;

    case 's':
      sscanf(optarg,"%ux%u",&width,&height);
      freej.screen->resize(width,height);
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

     case 'T':
      sscanf (optarg, "%u", &theora_quality);
      break;

     case 'f':
      sscanf (optarg, "%u", &fps);
      break;

    case 'F':
      freej.screen->fullscreen();
      break;

     case 'V':
      sscanf (optarg, "%u", &vorbis_quality);
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

/* ===================================== */

// scandir selection for .js or .freej
int script_selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".freej")) return(1);
  if(strstr(dir->d_name,".js"))    return(1);
  return(0);
}


// load all default scripts in $DATADIR/freej and ~/.freej
int scripts(char *path) {
  char *dir;
  struct dirent **filelist;
  int found;

  dir = strtok(path,":");
  do {
    found = scandir(dir,&filelist,script_selector,alphasort);
    if(found<0) {
      error("loading default scripts: scandir error: %s", strerror(errno));
      return(-1);
    }
    /* .so files found, check if they are plugins */
    while(found--) {
      char temp[256];
      snprintf(temp,255,"%s/%s",dir,filelist[found]->d_name);
      // if it exist is a default one: source it
      freej.open_script(temp);
    }
  } while(( dir = strtok(NULL,":") ));

  return 1;
}
//[js]

int main (int argc, char **argv) {


  Layer *lay = NULL;

  notice("%s version %s   free the veejay",PACKAGE,VERSION);
  act("2001-2007 RASTASOFT :: freej.dyne.org");
  act("----------------------------------------------");

  assert( freej.init(width,height, opengl) );

  cmdline(argc,argv);
  set_debug(debug_level);

  /* sets realtime priority to maximum allowed for SCHED_RR (POSIX.1b)
     this hangs on some linux kernels - darwin doesn't even bothers with it
     anybody knows what's wrong when you turn it on? ouch! it hurts :|
     set_rtpriority is inside jutils.cpp
     if(set_rtpriority(true))
     notice("running as root: high priority realtime scheduling allowed.");
  */


  // refresh the list of available plugins
  freej.plugger.refresh();

  // load default settings
  freej.config_check("keyboard.js");
  

  /* execute javascript */
  if( javascript[0] ) {
    freej.interactive = false;
    freej.open_script(javascript);
    if(freej.quit) {
      //      freej.close();
      // here calling close directly we double the destructor
      // fixed omitting the explicit close() call
      // but would be better to make the destructor reentrant
      exit(1);
    } else freej.interactive = true;
  }



  /* initialize the S-Lang text Console */
  if(!noconsole) {
    if( getenv("TERM") ) {
      freej.console = new Console();
      freej.console->init( &freej );
    }
  }

  /* initialize the Keyboard Listener */
  //  freej.kbd.init( );

  /* initialize the On Screen Display */
  freej.osd.init( &freej );

  //  freej.video_encoder -> handle_audio (stream_audio );

  /* initialize encoded filename */
  if (encoded_filename[0] != '\0') {
    VideoEncoder *enc = get_encoder("theora");
    enc->init(&freej);
    enc->set_filedump(encoded_filename);
    freej.add_encoder(enc);
  }



  // Set fps
  freej.set_fps_interval ( fps );

  freej.start_running = startstate;

  /* create layers requested on commandline */
  {
    char *l, *p, *pp = layer_files;
    while(cli_chars>0) {

      p = pp;

      while(*p!='#' && cli_chars>0) { p++; cli_chars--; }
      l = p+1;
      if(cli_chars<=0) break; *p='\0';

      lay = create_layer(&freej, pp);

      if(lay) {
	
	if(! lay->init(&freej) ) {
	  error("can't initalize %s layer", lay->name);
	  delete lay; lay = NULL;
	}

	if(lay) { 
	  freej.add_layer(lay);
	  
	  if(! lay->open(pp) ) {
	    error("can't open %s %s layer",pp,lay->name);
	    delete lay; lay = NULL;
	  }
	}
      }
      pp = l;
    }
  }



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

  //  freej.close();

  jsleep(1,0);

  exit(1);
}
