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

#include <SDL.h>

#include <layer.h>
#include <context.h>
#include <jutils.h>
#include <config.h>

Layer::Layer()
  :Entry(), JSyncThread() {
  quit = false;
  active = false;
  running = false;
  hidden = false;
  bgcolor = 0;
  bgmatte = NULL;
  blitter.set_blit("RGB");
  set_alpha(255);
  set_name("???");
  buffer = NULL;
  surf = NULL; // sdl surface. if used this is !NULL
}

Layer::~Layer() {
  filters.clear();
  if(bgmatte) jfree(bgmatte);
}

void Layer::_init(Context *freej, int wdt, int hgt, int bpp) {
  this->freej = freej;

  geo.w = (wdt == 0) ? freej->screen->w : wdt;
  geo.h = (hgt == 0) ? freej->screen->h : hgt;
  geo.bpp = (bpp) ? bpp : freej->screen->bpp;
  geo.size = geo.w*geo.h*(geo.bpp>>3);
  geo.pitch = geo.w*(geo.bpp>>3);
  geo.fps = freej->fps;
  geo.x = (freej->screen->w - geo.w)/2;
  geo.y = (freej->screen->h - geo.h)/2;

  /* initialize the blitter */
  blitter.init(this);
  blitter.crop( freej->screen );

  /* allocate memory for the matte background */
  bgmatte = jalloc(bgmatte,geo.size);
  
  notice("initialized %s layer %ix%i %ibpp",
	 get_name(),geo.w,geo.h,geo.bpp);
}

void Layer::run() {
  while(!feed()) jsleep(0,50);
  func("ok, layer %s in rolling loop",get_name());
  running = true;
  wait_feed();
  while(!quit) {
    if(bgcolor==0) {
      buffer = feed();
      if(!buffer) error("feed error on layer %s",name);
      wait_feed();
    } else if(bgcolor==1) { /* go to white */
      memset(bgmatte,0xff,geo.size);
      jsleep(0,10);
    } else if(bgcolor==2) { /* go to black */
      memset(bgmatte,0x0,geo.size);      
      jsleep(0,10);
    }
  }
  running = false;
}

bool Layer::cafudda() {

  if((!active) || (hidden))
    return false;

  offset = (bgcolor) ? bgmatte : buffer;
  if(!offset) {
    signal_feed();
    return(false);
  }

  filters.lock();

  Filter *filt = (Filter *)filters.begin();

  while(filt) {
    if(filt->active) offset = filt->process(offset);
    filt = (Filter *)filt->next;
  }
  
  blitter.blit();
  
  filters.unlock();

  signal_feed();

  return(true);
}


void Layer::set_name(char *s) {
  snprintf(name,4,"%s",s);
}
char *Layer::get_name() { return name; }

char *Layer::get_blit() {
  Entry *b = blitter.blitlist.selected();
  if(!b) {
    error("no blit selected for layer %s",name);
    error("this is dangerous, and should never happen!");
    return "???";
  }
  return b->name;
}

void Layer::set_alpha(int opaq) {
  alpha = (opaq>255) ? 255 : (opaq<0) ? 0 : opaq;
  snprintf(alphastr,4,"%03u",alpha);
}

void Layer::set_filename(char *f) {
  char *p = f + strlen(f);
  while(*p!='/' && (p > f)) p--;
  strncpy(filename,p+1,256);
}

void Layer::set_position(int x, int y) {
  lock();
  geo.x = x;
  geo.y = y;
  blitter.crop( freej->screen );
  unlock();
}
