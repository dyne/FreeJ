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
 *
 * "$Id$"
 *
 */

#include <inttypes.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>
#include <context.h>
#include <jutils.h>
#include <config.h>

/* flags can be:
   SDL_ANYFORMAT
   SDL_HWPALETTE
   SDL_HWSURFACE | SDL_SWSURFACE
   SDL_DOUBLEBUF
   SDL_FULLSCREEN */

Context::Context(int wx, int hx, int bppx, Uint32 flagsx) {

  surf = NULL;

  notice("Context: Simple Direct Media Layer");

  /* initialize SDL */
  if( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    error("Can't initialize SDL: %s",SDL_GetError());
    exit(1);
  }

  dbl = false;

  init(wx,hx,bppx,flagsx);
  
  /* context can be only one so we assign here a const id */
  id = 1;

  /* discover about the system and windowmanager
  if(!SDL_GetWMInfo(&sys)) {
    error("can't gather information about the display system");
    error("it might be dangerous to go on, like it could hang the machine");
    exit(1);
    } */

  /* be nice with the window manager */
  char temp[120];
  sprintf(temp,"%s %s",PACKAGE,VERSION);
  SDL_WM_SetCaption (temp, temp);
  
  /* ignore events :: only keys and wm_quit */
  for ( int i=SDL_NOEVENT; i<SDL_NUMEVENTS; ++i )
    if( !(i & (SDL_KEYDOWN|
	       SDL_MOUSEMOTION|
	       SDL_QUIT)) )
      SDL_EventState(i, SDL_IGNORE);


  SDL_ShowCursor(0);

  osd = NULL;

  doubletab = NULL;
  doublebuf = NULL;


  /* initialize fps counter */
  framecount=0;
  gettimeofday( &lst_time, NULL);
  fps=0.0;
  set_fps_interval(24);
  track_fps = false;

  clear_all = false;
  quit = false;

}  

bool Context::init(int wx, int hx, int bppx, Uint32 flagsx) {
  int res;
  func("Context::init(%u %u %u %u)",wx,hx,bppx,flagsx);

  bpp = bppx;
  flags = flagsx;

  /* EXPERIMENTAL
     setup GL context attributes 
  SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 5 );
  SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 5 );
  //  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
  SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  */

  res = setres(wx,hx);
    
  /* save context geometry */
  w = wx; h = hx;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);
  flags = flagsx;
  
  /* precalculate y lookup tables */
  for(int c=0;c<h;c++)
    prec_y[c] = surface + (pitch*c);

  return true;
}

void *Context::coords(int x, int y) {

  return( 
	 (Uint8 *)prec_y[y] + 
	 (x<<(bpp>>4)) 
	 );
  //  return ( surface + (pitch*y) + (x<<(bpp>>4)) );
}
int Context::setres(int wx, int hx) {
  /* check and set available videomode */
  int res = SDL_VideoModeOK(wx, hx, bpp, flags);

  act("screen geometry w[%u] h[%u] bpp[%u]",wx,hx,res);

  surf = SDL_SetVideoMode(wx, hx, bpp, flags);
  if( surf == NULL ) {
    error("can't set w[%u] h[%u] bpp[%u] video mode: %s\n",
	  wx, hx, bpp, SDL_GetError());
    exit(1);
  }

  if(res!=bpp) {
    SDL_Surface *surfemu;
    warning("your screen does'nt support %ubpp",bpp);
    act("doing video surface software conversion");
    
    surfemu = SDL_GetVideoSurface();
    act("emulated surface geometry w[%u] h[%u] bpp[%u]",
	surfemu->w,surfemu->h,surfemu->format->BitsPerPixel);
  }  
  surface = (Uint8*)SDL_GetVideoSurface()->pixels;

  return res;
}


bool Context::doublesize(bool val) {

  if(!val) {

    if(!dbl) return true;
    setres(w,h);

    surface = (Uint8*)SDL_GetVideoSurface()->pixels;

    dbl = false;

  } else {

    if(dbl) return true;
    setres(w<<1,h<<1);

    /* allocate video buffer to be doubled */
    if(doublebuf) free(doublebuf);
    doublebuf = (Uint8*)malloc(size);
    surface = doublebuf;

    /* precalculate tables for double-size transform */
    uint64_t *srftmp = (uint64_t*)SDL_GetVideoSurface()->pixels;
    if(doubletab) free(doubletab);
    doubletab = (uint64_t**)malloc(h*2*sizeof(uint64_t*));
    for(int cy=0;cy<h*2;cy++)
      doubletab[cy] = (uint64_t*) &srftmp[cy*w];
    

    notice("screen magnified X2");
    dbl = true;

  }

  for(int c=0; c<h; c++)
    prec_y[c] = surface + (pitch*c);
  


  return true;
}

void Context::close() {
  func("Context::~Context()");

  Layer *lay = (Layer *)layers.begin();

  if(osd) osd->splash_screen();
  flip();
  
  while(lay) {
    lay->lock();
    layers.rem(1);
    lay->quit = true;
    lay->signal_feed();
    lay->unlock();
    SDL_Delay(500);
    delete lay;
    lay = (Layer *)layers.begin();
  }

  SDL_Delay(1000);

  if(doublebuf) free(doublebuf);
  if(doubletab) free(doubletab);

  SDL_Quit();

}

int Context::add_layer(Layer *newlayer) {
  int res = -1;
  
  if(!newlayer) {
    warning("Context::add_layer - invalid NULL layer");
    return(res);
  }
  
  if( newlayer->init(this) ) {
    layers.add(newlayer);
    res = layers.len();
  }
  return(res);
}   

int Context::del_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  act("Context::del_layer : TODO");
  return sel;
}

int Context::clear_layers() {
  int ret = layers.len();
  act("Context::clear_layers : TODO");
  return ret;
}

int Context::moveup_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  if( layers.moveup(sel) )
    show_osd("MOVE UP layer %u -> %u",sel,sel-1);
  return(sel-1);
}

int Context::movedown_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  if ( layers.movedown(sel) )
    show_osd("MOVE DOWN layer %u -> %u",sel,sel+1);
  return(sel+1);
}

int Context::active_layer(int sel) {
  Layer *lay = (Layer *)layers.pick(sel);
  if(!lay) return 0;
  lay->active = !lay->active;
  show_osd("%s layer %s pos %u",
	   lay->active ? "ACTIVATED" : "DEACTIVATED",
	   lay->getname(), sel);
  return lay->active;
}

bool Context::flip() {

  osd->print();

  if(dbl) /* double size of the screen */
    for(cy=0; cy<h; cy++ ) {
      dcy = cy<<1;
      for(cx=0;cx<w; cx++) {
	eax = *(uint64_t*)coords(cx,cy);
	*(doubletab[dcy+1]+cx) = *(doubletab[dcy]+cx) = eax;
      }
    }

  SDL_Flip(surf);

  return(true);
}

void Context::clear() {
  /* 
 register uint32_t ecx; 
 register uint64_t ebx = 0x0000000000000000;
 register uint64_t *edx = (uint64_t*)surface;
 for(ecx=size>>4;ecx>0;ecx--) *edx-- = ebx;
  */
 memset(surface,0x0,size);
}

/* FPS */

void Context::set_fps_interval(int interval) {
  fps_frame_interval = interval*1000000;
  min_interval = (long)1000000/interval;
}

void Context::calc_fps() {
  /* 1frame : elapsed = X frames : 1000000 */
  gettimeofday( &cur_time, NULL);
  elapsed = cur_time.tv_usec - lst_time.tv_usec;
  if(cur_time.tv_sec>lst_time.tv_sec) elapsed+=1000000;
  
  if(track_fps) {
    framecount++;
    if(framecount==24) {
      fps=(double)1000000/elapsed;
      framecount=0;
    }
  }

  if(elapsed<=min_interval) {
    usleep( min_interval - elapsed ); /* this is not POSIX, arg */
    lst_time.tv_usec += min_interval;
    if( lst_time.tv_usec > 999999) {
      lst_time.tv_usec -= 1000000;
      lst_time.tv_sec++;
    }
  } else {
    lst_time.tv_usec = cur_time.tv_usec;
    lst_time.tv_sec = cur_time.tv_sec;
  }

}

void Context::rocknroll(bool state) {
  Layer *l = (Layer *)layers.begin();
  while(l) {
    if(!l->running) {
      l->start();
      //    l->signal_feed();
      while(!l->running) jsleep(0,500);
      l->active = state;
    }
    l = (Layer *)l->next;
  }
}
