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
#include <jutils.h>

void Layer::_delete() {
  /* Filters are now cleaned into the Plugger::_delete() (plugger.cpp) 
     func("Layer::_delete()");
     
     Filter *tmp, *filt = (Filter *)filters.begin();
     
     while(filt!=NULL) {
     tmp = (Filter *)filt->next;
     filt->clean();
     filt = tmp;
     }
  */
}

void Layer::_init(Context *screen, int wdt, int hgt) {
  geo.w = (wdt == 0) ? screen->w : wdt;
  geo.h = (hgt == 0) ? screen->h : hgt;
  geo.bpp = screen->bpp;
  geo.size = geo.w*geo.h*(geo.bpp>>3);
  geo.pitch = geo.w*(geo.bpp>>3);
  geo.fps = screen->fps;

  _w = geo.w; _h = geo.h;
  _pitch = geo.pitch;
  _size = geo.size;
  quit = false;
  this->screen = screen;
  screen->add_layer(this);
}

void Layer::run() {
  while(!quit) {
    feed();
    wait_feed();
  }
}

bool Layer::add_filter(Filter *newfilt) {

  /* PARANOIA */
  if(!newfilt) {
    warning("Layer::add_filter called with an invalid NULL filter");
    return(false);
  }

  func("Layer::add_filter(%s)",newfilt->getname());

  /* bpp support of the filter is checked by plugger */

  /* let the filter initialize */
  if(!newfilt->initialized) {
    newfilt->init(&geo);
    newfilt->initialized = true;
  }

  /* add the filter to the linklist */
  lock();
  filters.add(newfilt);
  unlock();

  show_osd("NEW filter %s pos %u",newfilt->getname(),filters.len());
  
  return(true);
}

bool Layer::del_filter(int sel) {
  func("Layer::del_filter(%u)",sel);

  Filter *filt = (Filter *) filters[sel];
  /* PARANOIA */
  if(!filt) {
    warning("Layer::del_filter - filters.pick(%u) returned NULL",sel);
    return(false);
  };

  lock();
  filters.rem(sel);
  filt->inuse = false;
  unlock();

  show_osd("DEL filter %s pos %u",filt->getname(),sel);

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
    f->inuse = false;
    f = (Filter *)filters.begin();
  }
  unlock();

  show_osd("CLEARED %u filters",c);
}

bool Layer::moveup_filter(int sel) {
  bool res = filters.moveup(sel);
  if(res)
    show_osd("MOVE UP filter %u -> %u",sel,sel-1);
  return(res);
}

bool Layer::movedown_filter(int sel) {
  bool res = filters.movedown(sel);
  if(res)
    show_osd("MOVE DOWN filter %u -> %u",sel,sel+1);
  return(res);
}

Filter *Layer::active_filter(int sel) {
  Filter *filt = (Filter *)filters.pick(sel);
  filt->active = !filt->active;
  show_osd("%s filter %s pos %u",
      filt->active ? "ACTIVATED" : "DEACTIVATED",
      filt->getname(), sel);
  return(filt);
}

bool Layer::cafudda() {
  void *res = get_buffer();
  if(!res) {
    signal_feed();
    return(false);
  }
  /* restore original size info so that it
     can be changed from the filters
     geo.w = _w; geo.h = _h; geo.pitch = _pitch; geo.size = _size;
  */

  lock();

  Filter *filt = (Filter *)filters.begin();
  
  while(filt!=NULL) {
    if(filt->active) res = filt->process(res);
    filt = (Filter *)filt->next;
  }

  lock_feed();

  /*  if(pitch==screen->pitch)
    mmxcopy(res,screen->get_surface(),size);
    else */
  mmxblit(res,screen->coords(geo.x,geo.y),geo.h,geo.pitch,screen->pitch); 
 
  /* pitch is width in bytes */

  unlock();
  unlock_feed();
  signal_feed();

  return(true);
}
