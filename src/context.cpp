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
#ifdef WITH_AUDIO
#include <audio_pipewire.h>
#endif
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <context.h>
#include <exception>
#include <mutex>
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
#include <exceptions.h>

#ifdef WITH_JAVASCRIPT
#include <jsparser_data.h>
#endif

#include <impl_layers.h>
#include <impl_video_encoders.h>
#include <factory.h>

#include "engine/frame_step.h"
#include "engine/frame_storage.h"
#include "media/movie_registration.h"

void fsigpipe (int Sig);
int got_sigpipe;

bool Context::factory_initialized = false;

static void init_factory() {
    Factory<Controller>::set_default_classtype("KeyboardController", "sdl"); // singleton
    Factory<Controller>::set_default_classtype("MouseController", "sdl"); // singleton
    Factory<Controller>::set_default_classtype("TriggerController", "core"); // singleton
    Factory<Controller>::set_default_classtype("OscController", "core");

    Factory<ViewPort>::set_default_classtype("Screen", "sdl"); // singleton
    
    Factory<Layer>::set_default_classtype("GeometryLayer", "basic");
    freej_media::register_movie_layer();
    Factory<Layer>::set_default_classtype("GeneratorLayer","ff_f0r");
    Factory<Layer>::set_default_classtype("ImageLayer","sdl");
#ifdef WITH_UNICAP
    Factory<Layer>::set_default_classtype("CamLayer", "unicap");
#endif
#ifdef WITH_OPENCV
    Factory<Layer>::set_default_classtype("CamLayer", "opencv");
#endif
#ifdef WITH_V4L
    Factory<Layer>::set_default_classtype("CamLayer", "v4l2");
#endif

#ifdef WITH_CAIRO
    Factory<Layer>::set_default_classtype("VectorLayer", "cairo");
#endif
#ifdef WITH_TEXTLAYER
    Factory<Layer>::set_default_classtype("TextLayer", "truetype");
#endif    
#ifdef WITH_FREI0R
    Factory<Filter>::set_default_classtype("Frei0rFilter", "core");
#endif
#ifdef WITH_FREEFRAME
    Factory<Filter>::set_default_classtype("FreeframeFilter", "core");
#endif
    Factory<FilterInstance>::set_default_classtype("FilterInstance", "core");
}

void * run_context(void * data){
	Context * context = (Context *)data;
	context->start();
	pthread_exit(NULL);
}

Context::Context() : initialized(false), running(false) {

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
  screen          = NULL;

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
#if defined WITH_TEXTLAYER
" .  - TextLayer for text rendered with freetype2 library\n"
#endif
#ifdef WITH_FLASH
" .  - FlashLayer for SWF flash v.3 animations\n"
#endif
#ifdef WITH_OPENCV
" .  - OpenCV for camera capture\n"
#endif
"\n";

  screens_description = (char*)
" .  - [ sdl   ] screen - Simple Directmedia Layer\n"
" .  - [ soft  ] screen - internal shared buffer\n"
#ifdef WITH_OPENGL
" .  - [ sdlgl ] screen - SDL 3d opengl\n"
" .  - [ gl    ] screen - 3d opengl surface\n"
#endif
#ifdef WITH_AALIB
" .  - [ aa    ] screen - Ascii (aalib) text output\n"
#endif
"\n";

  if (!factory_initialized) {
    init_factory();
    factory_initialized = true;
  }

  if (!init())
    throw FreejError("Could not initialize the FreeJ engine");

}

Context::~Context() {

  //Controller *ctrl;
  //ViewPort *scr;

  reset();


  //   invokes JSGC and all gc call on our JSObjects
  //  if(js) js->reset();

  notice ("cu on %s", PACKAGE_URL);
}


// 
// Factory-related methods
//


//
// End of Factory-related methods
//

bool Context::add_screen(ViewPort *scr) {

  if(!scr->initialized) {
    error("can't add screen %s - not initialized yet",scr->name);
    error("use init( width, height, bits_per_pixel )");
    return false;
  }
  screens.prepend(scr);
  screens.sel(0);
  scr->sel(true);
  screen = screens.begin();
  func("screen %s successfully added", scr->name);
  act("screen %s now on top",screen->name);

  return(true);
}

bool Context::init() {
  if (initialized)
    return true;

  notice("Initializing the FreeJ engine");
#ifdef FREEJ_NATIVE_PROFILE
  act("Native profile %s selects %s", FREEJ_NATIVE_PROFILE, FREEJ_CAPABILITIES);
#endif

  // a fast benchmark to select the best memcpy to use
  find_best_memcpy ();
  

  fps.init(fps_speed);

#ifdef WITH_JAVASCRIPT
  // create javascript object
  try {
    js = new JsParser(this);
  } catch (const FreejError& failure) {
    error("%s", failure.what());
    return false;
  }
#endif

  // register SIGPIPE signal handler (stream error)
  got_sigpipe = false;
  if (signal (SIGPIPE, fsigpipe) == SIG_ERR) {
    error ("Couldn't install SIGPIPE handler"); 
    //   exit (0); lets not be so drastical...
  }

  // refresh the list of available plugins
  plugger.refresh(this);

  initialized = true;
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

void Context::cafudda(double /*secs*/) {
  freej_engine::FrameHost host{
    {},
    [](ViewPort &screen) {
      freej_engine::FrameAccess::capture(screen);
      screen.show();
    },
    [] {
      timeval now;
      gettimeofday(&now, NULL);
      return now;
    }
  };
  if (poll_events) {
    event.type = SDL_EVENT_FIRST;
    SDL_PumpEvents();
    host.dispatch_input = [this] { handle_controllers(); };
  }
  fps.calc_at(freej_engine::step_frame(*this, host));
#ifdef WITH_AUDIO
  PipeWireClient::ReportPlaybackUnderruns();
#endif
  fps.delay();
}

void Context::handle_controllers() {
  {
    std::lock_guard<BaseLinklist> registry(controllers);
    if (polling_controllers_ || resetting_ || reset_requested_)
      return;
  }

  if (SDL_HasEvent(SDL_EVENT_QUIT)) {
    quit = true;
    return;
  }

  // Reserve Ctrl-F for the active viewport.
  if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_DOWN) == 1 &&
      !event.key.repeat && (event.key.mod & SDL_KMOD_CTRL) && event.key.key == SDLK_F) {
    if (auto *screen = screens.selected())
      screen->fullscreen();
    SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_DOWN);
  }

  std::vector<Controller *> frame_controllers;
  {
    std::lock_guard<BaseLinklist> registry(controllers);
    if (polling_controllers_ || resetting_ || reset_requested_)
      return;
    for (auto *ctrl = controllers.begin(); ctrl; ctrl = static_cast<Controller *>(ctrl->next))
      frame_controllers.push_back(ctrl);
    polling_controllers_ = true;
  }

  // Poll may run user code that unlinks any controller. Detached entries
  // stay alive until this traversal has released its snapshot.
  std::exception_ptr failure;
  try {
    for (auto *ctrl : frame_controllers)
      if (can_poll_controller(*ctrl))
        ctrl->poll();
  } catch (...) {
    failure = std::current_exception();
  }

  bool reset_after_poll;
  for (;;) {
    std::vector<Controller *> retired;
    {
      std::lock_guard<BaseLinklist> registry(controllers);
      if (retired_controllers_.empty()) {
        polling_controllers_ = false;
        reset_after_poll = reset_requested_;
        break;
      }
      retired.swap(retired_controllers_);
    }
    for (auto *ctrl : retired)
      delete ctrl;
  }
  if (reset_after_poll)
    reset();
  if (failure)
    std::rethrow_exception(failure);

  // flushes all events that are leftover
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_FIRST, SDL_EVENT_LAST) > 0)
    if (event.type == SDL_EVENT_QUIT)
      quit = true;
  memset(&event, 0x0, sizeof(SDL_Event));
}

bool Context::can_poll_controller(const Controller &ctrl) {
  std::lock_guard<BaseLinklist> registry(controllers);
  return !quit && !resetting_ && !reset_requested_ &&
         ctrl.active && ctrl.list == &controllers;
}

bool Context::register_controller(Controller *ctrl) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

  if(!ctrl) {
    error("%s called on a NULL object", __PRETTY_FUNCTION__);
    return false;
  }

  std::lock_guard<BaseLinklist> registry(controllers);
  if (resetting_ || reset_requested_ || ctrl->retiring_)
    return false;

  if(! ctrl->initialized ) {
    func("initialising controller %s (%p)",ctrl->name, ctrl);

    ctrl->init(this);

  } else if(ctrl->env != this) {

    error("controller is already initialised with another context: %x", ctrl->env);
    return false;

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

  bool defer;
  {
    std::lock_guard<BaseLinklist> registry(controllers);
    if (ctrl->retiring_ || (ctrl->list && ctrl->list != &controllers))
      return false;
    defer = polling_controllers_;
    if (defer)
      retired_controllers_.push_back(ctrl);
    ctrl->retiring_ = true;
    ctrl->active = false;
    ctrl->rem();
    act("removed controller %s", ctrl->name);
  }
  if (!defer)
    delete ctrl;
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

  warning("use of Context::add_layer is DEPRECATED");
  warning("please use ViewPort::add_layer instead");
  warning("a list of screens (view ports) is available");
  warning("kijk in Context::screens Linklist");

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

  std::vector<Controller *> removed;
  {
    std::lock_guard<BaseLinklist> registry(controllers);
    if (resetting_)
      return 1;
    if (polling_controllers_) {
      reset_requested_ = true;
      return 1;
    }
    removed.reserve(controllers.len());
    reset_requested_ = false;
    resetting_ = true;
    for (auto *ctrl = controllers.begin(); ctrl;) {
      auto *next = static_cast<Controller *>(ctrl->next);
      if (ctrl->indestructible) {
        ctrl->reset();
      } else {
        ctrl->retiring_ = true;
        ctrl->active = false;
        ctrl->rem();
        removed.push_back(ctrl);
      }
      ctrl = next;
    }
  }

  notice("FreeJ engine reset");

  for (auto *ctrl : removed)
    delete ctrl;
  func("deleting %u screens", screens.len() );
  ViewPort *scr = screens.begin();
  while(scr) {
      scr->reset();
      if (scr->indestructible) {
          scr = (ViewPort *)scr->next;
      } else {
          scr->rem();
          delete(scr);
          scr = screens.begin();
      }
  }
  screen = screens.begin();
    
  if (js)
    js->reset();
  {
    std::lock_guard<BaseLinklist> registry(controllers);
    resetting_ = false;
  }
  //does anyone care about reset() return value?
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
  if (!scr || w <= 0 || h <= 0)
    return;
  scr->resize_w = w;
  scr->resize_h = h;
  scr->resizing = true;
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


// TODO - use factory to create instances for each class of layers
//        (cam-input, videofile-input, whatever)
Layer *Context::open(char *file, int w, int h) {
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

  if( !w || !h ) {
    // uses the size of currently selected screen
    ViewPort *screen = screens.selected();
    w = screen->geo.w; h = screen->geo.h;
  }

  /* ==== Unified caputure API (V4L & V4L2) */
  if( strncasecmp ( file_ptr,"/dev/video",10)==0) {
    unsigned int uw, uh;
    while(end_file_ptr!=file_ptr) {
      if(*end_file_ptr!='%') {

	// uses the size of currently selected screen
	uw = w; uh = h;
	end_file_ptr--;

      } else { /* size is specified */

        *end_file_ptr='\0'; end_file_ptr++;
        sscanf(end_file_ptr,"%ux%u",&uw,&uh);
        end_file_ptr = file_ptr; 

      }
    }
    nlayer = Factory<Layer>::new_instance("CamLayer");
    if(!nlayer) {
      error("cannot open camera on %s",file_ptr);
      error("no implementation found for CamLayer in this FreeJ binary");
      return NULL; }
    if(! nlayer->init( uw, uh, 32 ) ){
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

  } else /* VIDEO LAYER */

    if( ( ( IS_VIDEO_EXTENSION(end_file_ptr) ) | ( IS_FIREWIRE_DEVICE(file_ptr) ) ) ) {
      func("is a movie layer");

#ifdef WITH_FFMPEG
       nlayer = Factory<Layer>::new_instance("MovieLayer");
       if (!nlayer)
         return nullptr;
       if(!nlayer->init()) {
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
              if(!nlayer->init()) {
                error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
                delete nlayer; return NULL;
              }
	      if(!nlayer->open(file_ptr)) {
		  error("create_layer : IMG open failed");
		  delete nlayer; nlayer = NULL;
	      }
  } else /* TXT LAYER */
    if(strncasecmp((end_file_ptr-4),".txt",4)==0) {
#if defined WITH_TEXTLAYER
	  nlayer = new TextLayer();

      if(!nlayer->init()) {
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

  } else /* XSCREENSAVER LAYER */
    if(strstr(file_ptr,"xscreensaver")) {
#ifdef WITH_XSCREENSAVER
	    nlayer = new XScreenSaverLayer();

	    if(!nlayer->init(w, h, 32)) {
	      error("failed initialization of layer %s for %s", nlayer->name, file_ptr);
	      delete nlayer; return NULL;
	    }

	    if (!nlayer->open(file_ptr)) {
	      error("create_layer : XScreenSaver open failed");
	      delete nlayer; nlayer = NULL;
	    }
#else
	    error("no xscreensaver layer support");
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
      if(!nlayer->init( )) {
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
    if(!nlayer->init()) {
      error("failed initialization of webcam with OpenCV");
      delete nlayer; return NULL;
    }
  }
#endif

  if(!nlayer)
    error("can't create a layer with %s",file);
  else
    func("create_layer successful, returns %p",nlayer);
  return nlayer;
}

