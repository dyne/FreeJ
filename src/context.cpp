/*  FreeJ
 *  (c) Copyright 2001 - 2007 Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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
 */

#include <config.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <context.h>
#include <blitter.h>
#include <controller.h>


#include <jsparser.h>
#include <video_encoder.h>
#include <audio_collector.h>
#include <fps.h>

#include <signal.h>
#include <errno.h>

#include <jutils.h>
#include <fastmemcpy.h>

#ifdef WITH_JAVASCRIPT
#include <jsparser_data.h>
#endif

#ifdef WITH_FFMPEG
#include <video_layer.h>
#endif

#include <impl_layers.h>
#include <impl_video_encoders.h>
#include <factory.h>

static Factory<Layer> layer_factory;
static Factory<Controller> controller_factory;

void fsigpipe (int Sig);
int got_sigpipe;

void * run_context(void * data){
	Context * context = (Context *)data;
	context->start();
	/*
	context->quit = false;
	while(!context->quit) {
		context->cafudda(0.0);
		pthread_yield();
		SDL_framerateDelay(&FPS); // synced with desired fps here
	}
	*/
	pthread_exit(NULL);
}

Context::Context() {

  //audio           = NULL;

  /* initialize fps counter */
  //  framecount      = 0; 
  clear_all       = false;
  start_running   = true;
  quit            = false;
  pause           = false;
  save_to_file    = false;
  interactive     = true;
  poll_events     = true;

  fps_speed       = 25;

  js = NULL;
  main_javascript[0] = 0x0;

  layers_description = (char*)
" .  - ImageLayer for image files (png, jpeg etc.)\n"
" .  - GeometryLayer for scripted vectorial primitives\n"
#ifdef WITH_V4L
" .  - Video4Linux devices as of BTTV cards and webcams\n"
" .    you can specify the size  /dev/video0%160x120\n"
#endif
#ifdef WITH_FFMPEG
" .  - MovieLayer for movie files, urls and firewire devices\n"
#endif
#if defined WITH_FT2 && defined WITH_FC
" .  - TextLayer for text rendered with freetype2 library\n"
#endif
#ifdef WITH_FLASH
" .  - FlashLayer for SWF flash v.3 animations\n"
#endif
#ifdef WITH_OPENCV
" .  - OpenCV for camera capture\n"
#endif
"\n";
    
  default_layertypes();
  default_controllertypes();
  
  assert( init() );

}

Context::~Context() {

  Controller *ctrl;
  ViewPort *scr;

  //  reset();


  func("deleting current controllers");
  ctrl = controllers.begin();
  while(ctrl) {
    if( ! ctrl->indestructible ) {

      ctrl->rem();
      delete(ctrl);
      
    }
    ctrl = controllers.begin();
  }

  func("deleting current screens");
  scr = screens.begin();
  while(scr) {

    scr->rem();
    delete(scr);
    scr = (ViewPort *)screens.begin();
    
  }

  //   invokes JSGC and all gc call on our JSObjects
  if(js) js->reset();

  notice ("cu on http://freej.dyne.org");
}


Layer *Context::get_layer_instance(const char *classname, const char *tag)
{
    return layer_factory.new_instance(classname, tag);
}

Layer *Context::get_layer_instance(const char *classname)
{
    return layer_factory.new_instance(classname, default_layertypes_map.find(classname)->second);
}


Controller *Context::get_controller_instance(const char *classname, const char *tag)
{
    return controller_factory.new_instance(classname, tag);
}

Controller *Context::get_controller_instance(const char *classname)
{
    return controller_factory.new_instance(classname, default_controllertypes_map.find(classname)->second);
}


int Context::register_layer_instantiator(const char *id, Instantiator func)
{
    return layer_factory.register_instantiator(id, func);
}

int Context::register_controller_instantiator(const char *id, Instantiator func)
{
    // create on first use idiom
    return controller_factory.register_instantiator(id, func);
}

bool Context::add_screen(ViewPort *scr) {

  scr->env = this;
  
  screens.prepend(scr);
  screens.sel(0);
  scr->sel(true);
  func("screen %s succesfully added", scr->name);

  // selected layer auxiliary pointer
  screen = scr;

  return(true);
}

bool Context::init() {

  notice("Initializing the FreeJ engine");

  // a fast benchmark to select the best memcpy to use
  find_best_memcpy ();
  

  fps.init(fps_speed);

#ifdef WITH_JAVASCRIPT
  // create javascript object
  js = new JsParser (this);
#endif

#ifdef WITH_FFMPEG
  /** init ffmpeg libraries: register all codecs, demux and protocols */
  av_register_all();
  /** make ffmpeg silent */
  av_log_set_level(AV_LOG_QUIET);
  //av_log_set_level(AV_LOG_DEBUG);
  act("FFmpeg initialized all codec and format");
#endif


  // register SIGPIPE signal handler (stream error)
  got_sigpipe = false;
  if (signal (SIGPIPE, fsigpipe) == SIG_ERR) {
    error ("Couldn't install SIGPIPE handler"); 
    //   exit (0); lets not be so drastical...
  }
  
  return true;
}


void Context::start() {
	quit = false;
	running = true;
	while(!quit) {
		cafudda(0.0);
	}
	running = false;
}

void Context::start_threaded(){
	if(!running)
		pthread_create(&cafudda_thread, 0, run_context, this);
}

/*
 * Main loop called fps_speed times a second
 */
void Context::cafudda(double secs) {
  
  ///////////////////////////////
  //// process controllers
  if(poll_events)
    handle_controllers();
  ///////////////////////////////
  
  /////////////////////////////
  // blit layers on screens
  ViewPort *scr;
  scr = screens.begin();
  while(scr) {

    if (clear_all) scr->clear();
    
    // Change resolution if needed 
    if (scr->changeres) scr->handle_resize();
    
    scr->blit_layers();

    // show the new painted screen
    scr->show();

    scr = (ViewPort*)scr->next;

  }
  /////////////////////////////

  /// FPS calculation
  fps.calc();
  fps.delay();
  
}

#define SDL_KEYEVENTMASK (SDL_KEYDOWNMASK|SDL_KEYUPMASK)

void Context::handle_controllers() {
  int res;
  Controller *ctrl;

  event.type = SDL_NOEVENT;

  SDL_PumpEvents();

  // peep if there are quit or fullscreen events
  res = SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_KEYEVENTMASK|SDL_QUITMASK);

  // force quit when SDL does
  if (event.type == SDL_QUIT) {
    quit = true;
    return;
  }
  
  // fullscreen switch (ctrl-f)
  if(event.type == SDL_KEYDOWN)
    if(event.key.state == SDL_PRESSED)
      if(event.key.keysym.mod & KMOD_CTRL)
	if(event.key.keysym.sym == SDLK_f) {
	  ViewPort *scr = screens.selected();
	  scr->fullscreen();
	  res = SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_KEYEVENTMASK|SDL_QUITMASK);  
	}
  
  ctrl = (Controller *)controllers.begin();
  if(ctrl) {
    controllers.lock();
    while(ctrl) {
      if(ctrl->active) ctrl->poll();
      ctrl = (Controller*)ctrl->next;
    }
    controllers.unlock();
  }

  // flushes all events that are leftover
  while( SDL_PeepEvents(&event,1,SDL_GETEVENT, SDL_ALLEVENTS) > 0 ) continue;
  memset(&event, 0x0, sizeof(SDL_Event));

}

bool Context::register_controller(Controller *ctrl) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(!ctrl) {
    error("%s called on a NULL object", __PRETTY_FUNCTION__);
    return false;
  }

  if(! ctrl->initialized ) {
    func("initialising controller %s (%p)",ctrl->name, ctrl);

    ctrl->init(this);

  } else if(ctrl->env != this) {

    error("controller is already initialised with another context: %x", ctrl->env);

  } else 
    warning("controller was already initialised on this context");

  func("controller %s initialized", ctrl->name);

  ctrl->active = true;

  controllers.append(ctrl);
  
  act("registered %s controller", ctrl->name);
  return true;
}

bool Context::rem_controller(Controller *ctrl) {
  func("%s",__PRETTY_FUNCTION__);
  if(!ctrl) {
    error("%s called on a NULL object", __PRETTY_FUNCTION__);
    return false;
  }

  if(js) js->gc(); // ?!

  ctrl->rem();
  // mh, the JS_GC callback does the delete ...
  if (ctrl->jsobj == NULL) {
    func("controller JSObj is null, deleting ctrl");
    delete ctrl;
  } else {
    ctrl->active = false;
    notice("removed controller %s, deactivated it but not deleting!", ctrl->name);
  }
  return true;
}

bool Context::add_encoder(VideoEncoder *enc) {
  func("%s",__PRETTY_FUNCTION__);

  ViewPort *scr;
  scr = screens.selected();
  if(!scr) {
    error("no screen initialized, can't add encoder %s", enc->name);
    return(false);
  }
  return( scr->add_encoder(enc) );
}

bool Context::add_layer(Layer *lay) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  ViewPort *scr = screens.selected();
  if(!scr) {
    error("no screen initialized, can't add layer %s", lay->name);
    return(false);
  }
  return( scr->add_layer(lay) );
  
}

void Context::rem_layer(Layer *lay) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    
  ViewPort *scr = screens.selected();
  if (scr)
    scr->rem_layer(lay);
}


int Context::open_script(char *filename) {
  if(!js) {
    error("can't open script %s: javascript interpreter is not initialized", filename);
    return 0;
  }
  return js->open(filename);
}


int Context::parse_js_cmd(const char *cmd) {
  if(!js) {
    error("javascript interpreter is not initialized");
    error("can't parse script \"%s\"",cmd);
    return 0;
  }
  return js->parse(cmd);
}

int Context::reset() {
  func("%s",__PRETTY_FUNCTION__);

  func("context deleting %u screens", screens.len() );
  ViewPort *scr;
  scr = screens.begin();
  while(scr) {
    scr->rem();
    delete(scr);
    scr = screens.begin();
  }

  // javascript garbage collector
  if(js) js->gc(); 

  // invokes JSGC and all gc call on our JSObjects
  //  if(js) js->reset();
  notice("FreeJ engine reset");
  // should return 1 on success? 
  //does anyone care about rese() return address?
  return 1; 
}

bool Context::config_check(const char *filename) {
  char tmp[512];

  if(!js) {
    warning("javascript is not initialized");
    warning("no configuration is loaded");
    return(false);
  }

  snprintf(tmp, 512, "%s/.freej/%s", getenv("HOME"), filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/etc/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

#ifdef HAVE_DARWIN
  snprintf(tmp, 512, "%s/%s", "CHANGEME", filename);
#else
  snprintf(tmp, 512, "%s/%s", DATADIR, filename);
#endif
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/usr/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/usr/local/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  snprintf(tmp, 512, "/opt/video/lib/freej/%s", filename);
  if( filecheck(tmp) ) {
    js->open(tmp);
    return(true);
  }

  return(false);
}
	  
void Context::resize(int w, int h) {
  ViewPort *scr = screens.selected();
  scr->resize_w = w;
  scr->resize_h = h;
  scr->resizing = true;
  scr->changeres = true;
}

void Context::magnify(int algo) {
  ViewPort *scr = screens.selected();  
  if(scr->magnification == algo) return;
  scr->magnification = algo;
  scr->changeres = true;
}

void *Context::coords(int x, int y) {
  ViewPort *scr = screens.selected();
  return( scr->coords(x,y) );
}


void fsigpipe (int Sig) {
  if (!got_sigpipe)
    warning ("SIGPIPE - Problems streaming video :-(");
  got_sigpipe = true;
}


//////// implemented layers



Layer *Context::open(char *file) {
  func("%s",__PRETTY_FUNCTION__);
  char *end_file_ptr,*file_ptr;
  FILE *tmp;
  Layer *nlayer = NULL;

  /* check that file exists */
  if(strncasecmp(file,"/dev/",5)!=0
     && strncasecmp(file,"http://",7)!=0
     && strncasecmp(file,"layer_",6)!=0) {
    tmp = fopen(file,"r");
    if(!tmp) {
      error("can't open %s to create a Layer: %s",
	    file,strerror(errno));
      return NULL;
    } else fclose(tmp);
  }
  /* check file type, add here new layer types */
  end_file_ptr = file_ptr = file;
  end_file_ptr += strlen(file);
//  while(*end_file_ptr!='\0' && *end_file_ptr!='\n') end_file_ptr++; *end_file_ptr='\0';

#ifdef HAVE_DARWIN

#endif
  /* ==== Unified caputure API (V4L & V4L2) */
  if( strncasecmp ( file_ptr,"/dev/video",10)==0) {
#ifdef WITH_UNICAP
    unsigned int w, h;
    while(end_file_ptr!=file_ptr) {
      if(*end_file_ptr!='%') {

	// uses the size of currently selected screen
	ViewPort *screen = screens.selected();
	w = screen->w; h = screen->h;
	end_file_ptr--;

      } else { /* size is specified */

        *end_file_ptr='\0'; end_file_ptr++;
        sscanf(end_file_ptr,"%ux%u",&w,&h);
        end_file_ptr = file_ptr; 

      }
    }
    nlayer = new UnicapLayer();
    if(! ((UnicapLayer*)nlayer)->init( this, (int)w, (int)h) ) {
      error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
      delete nlayer; return NULL;
    }
    if(nlayer->open(file_ptr)) {
      notice("video camera source opened");
    //  ((V4lGrabber*)nlayer)->init_width = w;
    //  ((V4lGrabber*)nlayer)->init_heigth = h;
    } else {
      error("create_layer : V4L open failed");
      delete nlayer; nlayer = NULL;
    }
#else
    error("Video4Linux layer support not compiled");
    act("can't load %s",file_ptr);
#endif

  } else /* VIDEO LAYER */

    if( ( ( IS_VIDEO_EXTENSION(end_file_ptr) ) | ( IS_FIREWIRE_DEVICE(file_ptr) ) ) ) {
      func("is a movie layer");

#ifdef WITH_FFMPEG
       nlayer = new VideoLayer();
       if(!nlayer->init( this )) {
 	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
 	delete nlayer; return NULL;
       }
       if(!nlayer->open(file_ptr)) {
 	error("create_layer : VIDEO open failed");
 	delete nlayer; nlayer = NULL;
       }
 #else
      error("VIDEO and AVI layer support not compiled");
      act("can't load %s",file_ptr);
#endif
  } else /* IMAGE LAYER */
      if( (IS_IMAGE_EXTENSION(end_file_ptr))) {
//		strncasecmp((end_file_ptr-4),".png",4)==0) 
	      nlayer = new ImageLayer();
              if(!nlayer->init( this )) {
                error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
                delete nlayer; return NULL;
              }
	      if(!nlayer->open(file_ptr)) {
		  error("create_layer : IMG open failed");
		  delete nlayer; nlayer = NULL;
	      }
  } else /* TXT LAYER */
    if(strncasecmp((end_file_ptr-4),".txt",4)==0) {
#if defined WITH_FT2 && defined WITH_FC
	  nlayer = new TextLayer();

      if(!nlayer->init( this )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	  if(!nlayer->open(file_ptr)) {
	    error("create_layer : TXT open failed");
	    delete nlayer; nlayer = NULL;
	  }
#else
	  error("TXT layer support not compiled");
	  act("can't load %s",file_ptr);
	  return(NULL);
#endif

  } else /* XHACKS LAYER */
    if(strstr(file_ptr,"xscreensaver")) {
#ifdef WITH_XHACKS
	    nlayer = new XHacksLayer();

      if(!nlayer->init( this )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	    if (!nlayer->open(file_ptr)) {
	      error("create_layer : XHACK open failed");
	      delete nlayer; nlayer = NULL;
	    }
#else
	    error("no xhacks layer support");
	    act("can't load %s",file_ptr);
	    return(NULL);
#endif
	  }  else if(strncasecmp(file_ptr,"layer_goom",10)==0) {

#ifdef WITH_GOOM
            nlayer = new GoomLayer();

      if(!nlayer->init( this )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }
#else
      error("goom layer not supported");
      return(NULL);
#endif


  } 
#ifdef WITH_FLASH
  else if(strncasecmp(end_file_ptr-4,".swf",4)==0) {

	    nlayer = new FlashLayer();
      if(!nlayer->init( this )) {
	error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	delete nlayer; return NULL;
      }

	    if(!nlayer->open(file_ptr)) {
	      error("create_layer : SWF open failed");
	      delete nlayer; nlayer = NULL;
	    }

  }
#endif

#ifdef WITH_OPENCV
  else if(strcasecmp(file_ptr,"layer_opencv_cam")==0) {
    func("creating a cam layer using OpenCV");
    nlayer = new OpenCVCamLayer();
    if(!nlayer->init(this)) {
      error("failed initialization of webcam with OpenCV");
      delete nlayer; return NULL;
    }
  }
#endif

  if(!nlayer)
    error("can't create a layer with %s",file);
  else
    func("create_layer succesful, returns %p",nlayer);
  return nlayer;
}

