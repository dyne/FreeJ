/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <layer.h>
#include <context.h>
#include <lubrify.h>

void Layer::_delete() {
  Filter *tmp, *filt = (Filter *)filters.begin();

  func("Layer::_delete()");
  
  while(filt!=NULL) {
    tmp = (Filter *)filt->next;
    filt->_delete();
    filt = tmp;
  }
}

void Layer::_init(Context *screen) {
  _w = w; _h = h;
  _pitch = pitch;
  _size = size;
  quit = false;
  this->screen = screen;
  screen->add_layer(this);
}

void Layer::run() {
  while(!quit) {
    feed();
    //    signal_feed();
    wait_feed();
  }
}

bool Layer::add_filter(Filter *newfilt) {

  if(newfilt==NULL) {
    warning("Layer::add_filter called with an invalid NULL filter");
    return(false);
  }

  func("Layer::add_filter(%s)",newfilt->name);

  if(!newfilt->bpp_ok(bpp)) {
    warning("%s filter does'nt support %ubpp",newfilt->name,bpp);
    return(false);
  }

  /* here pointers are used to let filters change the size of the layer*/
  newfilt->w = &w; newfilt->h = &h;
  newfilt->pitch = &pitch; newfilt->size = &size;
  /* --- not yet fully supported as a possibility for the filters */
  /* --- needs work */

  newfilt->bpp = bpp;
  newfilt->fps = fps; /* this is a pointer from the screen */

  notice("%s filter registered :: w[%u] h[%u] bpp[%u] size[%u]",newfilt->name,w,h,bpp,size);

  /* let the filter initialize */
  if(!newfilt->initialized)
    if(!newfilt->init()) {
      error("Layer::add_filter can't initialize filter");
      return(false);
    }

  /* add the filter to the linklist */
  lock();
  filters.add(newfilt);
  unlock();
  
  screen->osd->status("NEW filter %s pos %u",newfilt->name,filters.len());

  return(true);
}

bool Layer::del_filter(int sel) {
  func("Layer::del_filter(%u)",sel);

  Filter *filt = (Filter *) filters.pick(sel);
  /* PARANOIA */
  if(filt==NULL) {
    warning("Layer::del_filter - filters.pick(%u) returned a NULL Filter",sel);
    return(false);
  };

  lock();
  filters.rem(sel);
  unlock();

  screen->osd->status("DEL filter %s pos %u",filt->name,sel);

  delete(filt);
  return(true);
}

void Layer::clear_filters() {
  int c = 0;
  func("Layer::clear_filters()");

  lock();
  Filter *f = (Filter *)filters.begin();

  while(f!=NULL) {
    c++;
    filters.rem(1);
    f->_delete();
    f = (Filter *)filters.begin();
  }
  unlock();

  screen->osd->status("CLEARED %u filters",c);
}

bool Layer::moveup_filter(int sel) {
  bool res = filters.moveup(sel);
  if(res)
    screen->osd->status("MOVE UP filter %u -> %u",sel,sel-1);
  return(res);
}

bool Layer::movedown_filter(int sel) {
  bool res = filters.movedown(sel);
  if(res)
    screen->osd->status("MOVE DOWN filter %u -> %u",sel,sel+1);
  return(res);
}

Filter *Layer::active_filter(int sel) {
  Filter *filt = (Filter *)filters.pick(sel);
  filt->active = !filt->active;
  screen->osd->status("%s filter %s pos %u",
		      filt->active ? "ACTIVATED" : "DEACTIVATED",
		      filt->name, sel);
  return(filt);
}

Filter *Layer::listen_filter(int sel) {
  Filter *filt = (Filter *)filters.pick(sel);
  filt->listen = !filt->listen;
  return(filt);
}

bool Layer::cafudda() {
  void *res = get_buffer();
  /* restore original size info so that it
     can be changed from the filters */
  w = _w; h = _h; pitch = _pitch; size = _size;

  lock();

  Filter *filt = (Filter *)filters.begin();
  
  while(filt!=NULL) {
    if(filt->active) res = filt->process(res);
    filt = (Filter *)filt->next;
  }

  lock_feed();

  if(pitch==screen->pitch)
    mmxcopy(res,screen->get_surface(),size);
  else
    mmxblit(res,screen->coords(x,y),h,pitch,screen->pitch); 
  /* pitch is width in bytes */

  unlock();
  unlock_feed();
  signal_feed();

  return(true);
}
