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

#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <context.h>
#include <jutils.h>
#include <config.h>

/* flags can be:
   SDL_ANYFORMAT
   SDL_HWPALETTE
   SDL_DOUBLEBUF
   SDL_FULLSCREEN */

Context::Context(int wx, int hx, int bppx, Uint32 flags) {
  int res;
  surf = NULL;
  func("Context::Context(%u %u %u %u)",wx,hx,bppx,flags);

  /* initialize SDL */
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    error("Can't initialize SDL: %s",SDL_GetError());
    exit(1);
  }

  /* check and set available videomode */
  assert( res = SDL_VideoModeOK(wx, hx, bppx, flags) );
  notice("Context: Simple Direct Media Layer");
  if(res!=bppx)
    warning("your screen does'nt support %ubpp, using %ubpp instead",bppx,res);
  act("screen geometry w[%u] h[%u] bpp[%u]",wx,hx,res);
  surf = SDL_SetVideoMode(wx, hx, res, flags);
  if( surf == NULL ) {
    error("can't set w[%u] h[%u] bpp[%u] video mode: %s\n",
	  wx, hx, res, SDL_GetError());
    exit(1);
  }

  /* save context geometry */
  w = wx; h = hx;
  bpp = surf->format->BitsPerPixel;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);
  
  /* precalculate y lookup tables */
  for(int c=0;c<h;c++)
    prec_y[c] = (Uint8*)surf->pixels + (pitch*c);

  /* context can be only one so we assign here a const id */
  id = 1;

  /* be nice with the window manager */
  char temp[120];
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);
  
  /* ignore events :: only keys and wm_quit */
  for ( int i=SDL_NOEVENT; i<SDL_NUMEVENTS; ++i )
    if( (i != SDL_KEYDOWN) && ( i != SDL_QUIT) )
      SDL_EventState(i, SDL_IGNORE);

  SDL_ShowCursor(0);

  /* initialize fps counter */
  fps_frame_interval = -1;

  clear_all = false;
  quit = false;
}  

void Context::close() {
  Layer *tmp, *lay = (Layer *)layers.begin();
  func("Context::close()");
  
  while(lay!=NULL) {
    tmp = (Layer *)lay->next;
    lay->_close();
    lay->_delete();
    lay = tmp;
  }
  
  SDL_Quit();
  act("clean exiting. be nice ;)");
}

bool Context::add_layer(Layer *newlayer) {
  bool res = false;
  
  if(!newlayer) {
    warning("Context::add_layer - invalid NULL layer");
    return(res);
  }
  
  layers.add(newlayer);

  res = true;
  return(res);
}   

bool Context::moveup_layer(int sel) {
  bool res = layers.moveup(sel);
  if(res)
    show_osd("MOVE UP layer %u -> %u",sel,sel-1);
  return res;
}

bool Context::movedown_layer(int sel) {
  bool res = layers.movedown(sel);
  if(res)
    show_osd("MOVE DOWN layer %u -> %u",sel,sel+1);
  return res;
}

Layer *Context::active_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  lay->active = !lay->active;
  show_osd("%s layer %s pos %u",
	   lay->active ? "ACTIVATED" : "DEACTIVATED",
	   lay->getname(), sel);
  return lay;
}

bool Context::flip() {
  osd->print();
  SDL_Flip(surf);
  return(true);
}

void *Context::get_surface() {
  return((void *)surf->pixels);
}

/* FPS */

void Context::set_fps_interval(int interval) {
  fps_frame_interval = interval;
  framecount = -1;
}

bool Context::calc_fps() {
  
  /* Check interval */
  if (fps_frame_interval<1) {
    fps=0.0;
    return(true);
  }
  
  if ( (framecount<0) || (framecount>fps_frame_interval)) {
    /* Initialize counter */
    framecount=0;
    lst_time=dtime();
    fps=0.0;
    return(true);
  }

  framecount++;
  
  if (framecount==fps_frame_interval) {
    cur_time=dtime();
    fps=(double)framecount/(cur_time-lst_time);
    lst_time=cur_time;
    framecount=0;
    return(false);
  }
  
  return(true);
}

void Context::rocknroll() {
  Layer *l = (Layer *)layers.begin();
  while(l) {
    l->start();
    l = (Layer *)l->next;
  }
}
