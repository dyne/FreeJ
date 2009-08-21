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

#include <config.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <errno.h>

#include <context.h>

// #include <osd.h>
#include <slang_console_ctrl.h>
#include <video_encoder.h>
#include <plugger.h>
#include <jutils.h>
//#include <fps.h>

#include <impl_layers.h>
#include <impl_screens.h>
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
" .   -a   initialize audio (using jack)\n"
//" .   -m   software magnification: 2x,3x\n"
" .   -n   start with deactivated layers\n"
" .   -c   no interactive text console\n"
" .   -f   <frame_per_second>  select global fps for freej\n"
" .   -F   start in fullscreen\n"
#ifdef WITH_OPENGL
" .   -g   experimental opengl engine! (better pow(2) res as 256x256)\n"
#endif
" .   -j   <javascript.js>  execute a javascript file\n"
" .\n"
" .  Layers available:\n"
" .   you can specify any number of files or devices to be loaded,\n"
" .   this binary is compiled to support the following layer formats:\n";

// we use only getopt, no _long
static const char *short_options = "-hvD:gas:nj:cgf:F";

/* this is the global FreeJ context */
Context *freej = NULL;

// the runtime will open one screen by default
ViewPort *screen = NULL;

int   debug_level = 0;
char  layer_files[MAX_CLI_CHARS];
int   cli_chars = 0;
int   width = 400;
int   height = 300;
int   magn = 0;
char  javascript[512]; // script filename

int fps = 25;

bool startstate = true;
bool gtkgui = false;

int audiomode = 0;
bool noconsole = false;
bool fullscreen = false;
bool opengl = false;

void cmdline(int argc, char **argv) {
  int res, optlen;

  /* initializing defaults */
  char *p                  = layer_files;
  javascript[0]            = 0;

  debug_level              = 0;

  do {
//    res = getopt_long (argc, argv, short_options); TODO getopt_long
    res = getopt(argc, argv, short_options);
    switch(res) {
    case 'h':
      fprintf(stdout, "%s", help);
      fprintf(stdout, "%s", freej->layers_description);
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
//      freej.screen->resize(width,height);
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

    case 'a':
      audiomode = 1;
      break;

    case 'm':
      sscanf(optarg,"%u",&magn);
      magn -= 1;
      magn = (magn>3) ? 3 : (magn<1) ? 0 : magn;
      break;

    case 'n':
      startstate = false;
      break;

    case 'c':
      noconsole = true;
      break;

     case 'f':
      sscanf (optarg, "%u", &fps);
      break;

    case 'F':
      fullscreen = true;
//      freej.screen->fullscreen();
      break;

   case 'j':
      FILE *fd;
      fd = fopen(optarg,"r");
      if(!fd) {
	error("can't open JS file '%s': %s", optarg, strerror(errno));
	error("missing script, fatal error.");
	exit(0);
      }
      else {
	snprintf(javascript,512,"%s",optarg);
	fclose(fd);
      }
      break;

#ifdef WITH_OPENGL
    case 'g':
      opengl = true;
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
    } else 
	  warning("too many files on commandline, list truncated");
  }
#endif
}


/* ===================================== */

// scandir selection for .js or .freej
#if defined (HAVE_DARWIN) || defined (HAVE_FREEBSD)
int script_selector(struct dirent *dir)
#else
int script_selector(const struct dirent *dir) 
#endif
{
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
      freej->open_script(temp);
    }
  } while(( dir = strtok(NULL,":") ));

  return 1;
}
//[js]

#ifndef HAVE_DARWIN
int main (int argc, char **argv) {
  Layer *lay = NULL;
  ConsoleController *con = NULL;
  
  freej = new Context();

  notice("%s version %s   free the veejay",PACKAGE,VERSION);
  act("2001-2008 RASTASOFT :: freej.dyne.org");
  act("----------------------------------------------");

  cmdline(argc,argv);
  set_debug(debug_level);

  // create SDL screen by default at selected size
#ifdef WITH_OPENGL
  if(opengl)
    screen = new SdlGlScreen(width, height);
  else
#endif
    screen = new SdlScreen(width, height);

  // add the screen to the context
  freej->add_screen(screen);

  if(fullscreen) freej->screen->fullscreen();

  /* sets realtime priority to maximum allowed for SCHED_RR (POSIX.1b)
     this hangs on some linux kernels - darwin doesn't even bothers with it
     anybody knows what's wrong when you turn it on? ouch! it hurts :|
     set_rtpriority is inside jutils.cpp
     if(set_rtpriority(true))
     notice("running as root: high priority realtime scheduling allowed.");
  */


  
  /* initialize the S-Lang text Console */
  if(!noconsole) {
    if( getenv("TERM") ) {
      con = new SlwConsole();
      freej->register_controller( con );
      con->console_init();
      set_console( con );
    }
  }

  // refresh the list of available plugins
  freej->plugger.refresh(freej);

  // load default settings
  freej->config_check("keyboard.js");

  /* execute javascript */
  if( javascript[0] ) {
    freej->interactive = false;
    freej->open_script(javascript); // TODO: quit here when script failed??
    if(freej->quit) {
      //      freej.close();
      // here calling close directly we double the destructor
      // fixed omitting the explicit close() call
      // but would be better to make the destructor reentrant
      exit(1);
    } else freej->interactive = true;
  }




  /* initialize the On Screen Display */
  //  freej.osd.init( &freej );


  // Set fps
  freej->fps.set( fps );

  freej->start_running = startstate;

  /* create layers requested on commandline */
  {
    char *l, *p, *pp = layer_files;
    while(cli_chars>0) {

      p = pp;

      while(*p!='#' && cli_chars>0) { p++; cli_chars--; }
      l = p+1;
      if(cli_chars<=0) break; *p='\0';

      func("creating layer for file %s",pp);

      lay = freej->open(pp); // hey, this already init and open the layer !!
      if(lay)  { 
        if( freej->add_layer(lay) ) {
	  lay->start();
	  lay->fps.set(fps);
	}
        if (!startstate) 
	  lay->active = false;
      }

      pp = l;
    }
  }


  /* apply screen magnification */
  //  freej->magnify(magn);
  // deactivated for now


  /* MAIN loop */
  while( !freej->quit )
    /* CAFUDDARE in sicilian means to add a lot of 
       stuff into something; for example, to do the
       bread or the pasta for the pizza you have to
       CAFUDDARE a lot of wheat flour or water. 
       This also involve an intense work for your 
       arms, mixing wheat flour, ingredients, and so
       processing materia */
    freej->cafudda(1.0);
  /* also layers have the cafudda() function
     which is called by the Context class (freej instance here)
     so it's a tree of cafudda calls originating from here
     all synched to the environment, yea, feels good */






  /* quit */
  if(con) delete con;
  delete freej;

  exit(1);
}
#endif
