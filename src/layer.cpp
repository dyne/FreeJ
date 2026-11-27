  /*  FreeJ
   *  (c) Copyright 2001-2002 Denis Rojo aka jaromil <jaromil@dyne.org>
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

#include <string.h>

//#include <SDL3/SDL.h>

#include <layer.h>
#include <blitter.h>
#include <filter.h>
#include <iterator.h>
#include <closure.h>

#include <context.h>
#include <jutils.h>
#include <config.h>
#include "engine/frame_storage.h"

#include <jsparser_data.h>

//#include <fps.h>

Layer::Layer()
  :Entry(), JSyncThread(), frames_(new freej_engine::FrameStore) {
  func("%s this=%p",__PRETTY_FUNCTION__, this);
  active = false;
  hidden = false;
  fade = false;
  use_audio = false;
  need_crop = true;
  audio = NULL;
  opened = false;
  bgcolor = 0;
  set_name("???");
  filename[0] = 0;
  buffer = NULL;
  screen = NULL;
  is_native_sdl_surface = false;
  jsclass = &layer_class;

  zoom_x = 1.0;
  zoom_y = 1.0;
  rotate = 0.0;
  zooming = false;
  rotating = false;

  antialias = false;

  blitter = NULL;
  current_blit = NULL;

  null_feeds = 0;
  max_null_feeds = 10;

  parameters = NULL;

  priv_data = NULL;
      
  fps.set(25);

}

Layer::~Layer() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);

  stop();
  if(screen)
    screen->rem_layer(this);
  deferred_calls->close();
  active = false;
  FilterInstance *f = (FilterInstance*)filters.begin();
  while(f) {
    f->rem(); // rem is contained in delete for Entry
    delete f;
    f = (FilterInstance*)filters.begin();
  }

  // free all parameters
  if(parameters) {
    Parameter *par;
    par = parameters->begin();
    while(par) {
      par->rem();
      delete par;
      par = parameters->begin();
    }
    delete parameters;
  }
  
  if(blitter) delete blitter;
}

void *Layer::get_data() {
    return priv_data;
}

void Layer::set_data(void *data) {
    priv_data = data;   
}
bool Layer::init(int wdt, int hgt, int bpp) {
  
  geo.init(wdt, hgt, bpp);
  
  func("initialized %s layer %ix%i",
       get_name(), geo.w, geo.h);
  
  if (!geo.bytesize) {
    // if  the   size  is  still  unknown   at  init  then   it  is  the
    // responsability for the layer implementation to create the buffer
    // (see for instance text_layer)
    act("initialized %s layer with dynamic size", get_name());
    
  }
  
  // default fps
  fps.set(25);
  
  // call the specific layer's implementation _init()
  return _init();

}

void Layer::thread_setup() {
  func("ok, layer %s in rolling loop",get_name());
 
  //while(!feed()) fps.calc();

  func(" layer %s entering loop",get_name());
}

void Layer::thread_loop() {
  void *source = feed();
  freej_engine::FrameLayout layout{};
  {
    std::lock_guard<Layer> lock(*this);
    if (geo.bpp == 32)
      layout = {geo.w, geo.h, geo.bytewidth, freej_engine::FrameFormat::rgba32};
  }

  std::shared_ptr<freej_engine::Frame> frame;
  if (source && layout.width && layout.height) {
    std::lock_guard<Linklist<FilterInstance>> lock(filters);
    frame = frames_->prepare(do_filters(source), layout);
  }
  {
    std::lock_guard<Layer> lock(*this);
    buffer = frame ? frame->pixels.data() : nullptr;
    frames_->publish(std::move(frame));
  }
  fps.calc();
  fps.delay();
}

void Layer::clear_frame() {
  buffer = nullptr;
  frames_->clear();
}

void Layer::thread_teardown() {
  func("%s this=%p thread end: %p %s",__PRETTY_FUNCTION__, this, pthread_self(), name);
}

char *Layer::get_blit() {

  if(!current_blit) {
    error("no blit currently selected for layer %s",name);
    return((char*)"unknown");
  }
  return current_blit->name;
}

bool Layer::set_blit(const char *bname) {
  std::lock_guard<Layer> lock(*this);
  Blit *b;
  int idx;
  
  if (screen && blitter) {
	  b = (Blit*)blitter->blitlist.search(bname, &idx);

	  if(!b) {
		error("blit %s not found in screen %s",bname, screen->name);
		return(false);
	  }

	  func("blit for layer %s set to %s",name, b->name);

	  current_blit = b; // start using
	  need_crop = true;
	  blitter->blitlist.sel(0);
	  b->sel(true);
	  act("blit %s set for layer %s", current_blit->name, name);
  } else {
    warning("can't set blit for layer %s: not added on any screen yet", name);
    return(false);
  } 

  return(true);
}

void Layer::blit() {
  std::lock_guard<Layer> state_lock(*this);
  if (!screen)
    return;
  if(!buffer) {
    // check threshold of tolerated null feeds
    // deactivate the layer when too many
    func("feed returns NULL on layer %s",get_name());
    null_feeds++;
    if(null_feeds > max_null_feeds) {
      warning("layer %s has no video, deactivating", get_name());
      active = false;
      return;
    }

  } else {

    null_feeds = 0;

    const auto frame = freej_engine::FrameAccess::layer(*this);
    screen->blit(this);
  }
}

bool Layer::cafudda() {
  if(!opened) return false;

  if(!fade)
    if(!active || hidden)
      return false;

  do_iterators();


  //  signal_feed();

  return(true);
}

void *Layer::do_filters(void *tmp_buf) {
  std::lock_guard<Linklist<FilterInstance>> lock(filters);
  for (FilterInstance *filter = filters.begin(); filter;
       filter = static_cast<FilterInstance *>(filter->next)) {
    std::lock_guard<Linklist<Parameter>> parameters_lock(filter->parameters);
    if (filter->active && filter->frame_width_ == geo.w && filter->frame_height_ == geo.h)
      tmp_buf = filter->process(fps.fps, static_cast<uint32_t *>(tmp_buf));
  }
  return tmp_buf;
}

int Layer::do_iterators() {

  /* process thru iterators */
  if(iterators.len()) {
    iterators.lock();
    iter = (Iterator*)iterators.begin();
    while(iter) {
      res = iter->cafudda(); // if cafudda returns -1...
      itertmp = iter;
      iter = (Iterator*) ((Entry*)iter)->next;
      if(res<0) {
	iterators.unlock();
	delete itertmp; // ...iteration ended
	iterators.lock();
	if(!iter)
	  if(fade) { // no more iterations, fade out deactivates layer
	    fade = false;
	    active = false;
	  }
      }
    }
    iterators.unlock();
  }
  return(1);
}

bool Layer::set_parameter(int idx) {

  Parameter *param;
  param = parameters ? parameters->pick(idx) : nullptr;
  if( ! param) {
    error("parameter [%d] not found in layer %s", idx, name);
    return false;
  } else 
    func("parameter %s found in layer %s at position %u",
	 param->name, name, idx);

  if(!param->layer_set_f) {
    error("no layer callback function registered in this parameter");
    return false;
  }

  (*param->layer_set_f)(this, param, idx);

  return true;
}


void Layer::set_filename(const char *f) {
  const char *slash = strrchr(f, '/');
  strncpy(filename, slash ? slash + 1 : f, sizeof(filename) - 1);
  filename[sizeof(filename) - 1] = '\0';
}


void Layer::_set_position(int x, int y) {
  std::lock_guard<Layer> lock(*this);
  geo.x = x;
  geo.y = y;
  need_crop = true;
}

void Layer::set_position(int x, int y) {
  Closure *job = NewClosure(this, &Layer::_set_position, x, y);
  deferred_calls->add_job(job);
}


void Layer::_set_zoom(double x, double y) {
  std::lock_guard<Layer> lock(*this);
  if ((x == 1) && (y == 1)) {
    zooming = false;
    zoom_x = zoom_y = 1.0;
    act("%s layer %s zoom deactivated", name, filename);
  } else {
    zoom_x = x;
    zoom_y = y;
    zooming = true;
    func("%s layer %s zoom set to x%.2f y%.2f",
         name, filename, zoom_x, zoom_y);
  }
  need_crop = true;
}

void Layer::set_zoom(double x, double y) {
  Closure *job = NewClosure(this, &Layer::_set_zoom, x, y);
  deferred_calls->add_job(job);
}


void Layer::_set_rotate(double angle) {
  std::lock_guard<Layer> lock(*this);
  if(!angle) {
    rotating = false;
    rotate = 0;
    act("%s layer %s rotation deactivated", name, filename);
  } else {
    rotate = angle;
    rotating = true;
    func("%s layer %s rotation set to %.2f", name, filename, rotate);
  }
  need_crop = true;
}

void Layer::set_rotate(double angle) {
  Closure *job = NewClosure(this, &Layer::_set_rotate, angle);
  deferred_calls->add_job(job);
}


void Layer::_fit(bool maintain_aspect_ratio) {
  std::lock_guard<Layer> lock(*this);
  double width_zoom, height_zoom;
  int new_x = 0;
  int new_y = 0;
  if (this->screen == NULL) {
    error("Cannot fit layer without a screen, add layer to a screen first");
    return;
  }
  std::lock_guard<std::mutex> screen_lock(screen->geometry_mutex_);
  width_zoom = (double)screen->geo.w / geo.w;
  height_zoom = (double)screen->geo.h / geo.h;
  if (maintain_aspect_ratio){
    //to maintain the aspect ratio we simply zoom to the smaller of the
    //two zoom values
    if(width_zoom > height_zoom) {
      width_zoom = height_zoom;
    } else {
      height_zoom = width_zoom;
    }
  }
  _set_zoom(width_zoom, height_zoom);
  // reposition layer upper corner
  new_x = ((double)(width_zoom * geo.w - geo.w) / 2.0);
  new_y = ((double)(height_zoom * geo.h - geo.h) / 2.0);
  // center layer
  new_x += ((double)(screen->geo.w - width_zoom * geo.w) / 2.0);
  new_y += ((double)(screen->geo.h - height_zoom * geo.h) / 2.0);
  _set_position(new_x, new_y);
}

void Layer::fit(bool maintain_aspect_ratio) {
  Closure *job = NewClosure(this, &Layer::_fit, maintain_aspect_ratio);
  deferred_calls->add_job(job);
}

#ifdef WITH_JAVASCRIPT
void *Layer::js_constructor(Context *env, JSContext *cx, JSObject *obj,
                            int argc, void *aargv, char *err_msg) {
    
    char *filename;
    void *ret = NULL;
    int check_thread;
    uint16_t width  = geo.w;
    uint16_t height = geo.h;
    
    jsval *argv = (jsval*)aargv;
    check_thread = JS_GetContextThread(cx);
    if (!check_thread) // set the context thread only if none is already set
        JS_SetContextThread(cx);
    JS_BeginRequest(cx);
    if(argc==0) {
        if(!init()) {
            sprintf(err_msg, "Layer constructor failed initialization");
            return NULL;    }
        
    } else if(argc==1) {
        filename = js_get_string(argv[0]);
        if(!init()) {
            sprintf(err_msg, "Layer constructor failed initialization");
            return NULL;    }
        
        if(!open(filename)) {
            snprintf(err_msg, MAX_ERR_MSG, "Layer constructor failed open(%s): %s",
                     filename, strerror(errno));
            return NULL;    }
        
    } else if(argc==2) {
        JS_ValueToUint16(cx, argv[0], &width);
        JS_ValueToUint16(cx, argv[1], &height);
        if(!init(width, height, 32)) {
            snprintf(err_msg, MAX_ERR_MSG,
                     "Layer constructor failed initialization w[%u] h[%u]", width, height);
            return NULL;
        }
        
    } else if(argc==3) {
        JS_ValueToUint16(cx, argv[0], &width);
        JS_ValueToUint16(cx, argv[1], &height);
        filename = js_get_string(argv[2]);
        if(!init(width, height,32)) {
            snprintf(err_msg, MAX_ERR_MSG,
                     "Layer constructor failed initializaztion w[%u] h[%u]", width, height);
            return NULL;
        }
        if(!open(filename)) {
            snprintf(err_msg, MAX_ERR_MSG,
                     "Layer constructor failed initialization (%s): %s", filename, strerror(errno));
            return NULL;
        }
        
    } else {
        sprintf(err_msg,
                "Wrong numbers of arguments\n use (\"filename\") or (width, height, \"filename\") or ()");
        return NULL;
    }
    if(JS_SetPrivate(cx,obj,(void*)this)) {
        jsobj = obj; // save the JS instance object into the C++ instance object
        ret = (void*)OBJECT_TO_JSVAL(obj);
    } else {
        sprintf(err_msg, "%s", "JS_SetPrivate failed");
    }
    JS_EndRequest(cx);
    if (!check_thread)
        JS_ClearContextThread(cx);
    return ret;
    
}
#endif

bool Layer::_init()
{
    // Don't do anything in base implementation
    return true;
}

bool Layer::open(const char *file) {
    // do nothing , our subclass shoud override this method if they implement an open
    func("base Layer::open(%s) called passing", file);
    return false;
}

void Layer::close() {
    // do nothing , our subclass shoud override this method if they implement an open
    func("base Layer::close() called passing");
    return;
}
