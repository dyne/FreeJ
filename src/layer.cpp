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
 */

#include <string.h>
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
  alpha_blit = false;
  bgcolor = 0;
  bgmatte = NULL;
  blit_offset = 0;
  _blit_algo = 1;
  setname("???");
  buffer = NULL;
}

Layer::~Layer() {
  filters.clear();
  if(bgmatte) jfree(bgmatte);
}

void Layer::_init(Context *screen, int wdt, int hgt, int bpp) {
  this->screen = screen;

  geo.w = (wdt == 0) ? screen->w : wdt;
  geo.h = (hgt == 0) ? screen->h : hgt;
  geo.bpp = (bpp) ? bpp : screen->bpp;
  geo.size = geo.w*geo.h*(geo.bpp>>3);
  geo.pitch = geo.w*(geo.bpp>>3);
  geo.fps = screen->fps;
  geo.x = (screen->w - geo.w)/2;
  geo.y = (screen->h - geo.h)/2;

  crop();

  /* allocate memory for the matte background */
  bgmatte = jalloc(bgmatte,geo.size);
  
  notice("initialized %s layer %ix%i %ibpp",getname(),geo.w,geo.h,geo.bpp);
}

void Layer::run() {
  void *res;

  while(!feed()) jsleep(0,50);
  func("ok, layer %s in rolling loop",getname());
  running = true;
  wait_feed();
  while(!quit) {
    if(bgcolor==0) {
      res = feed();
      if(!res) error("feed error on layer %s",_name);
      else buffer = res;
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

void Layer::set_blit(int b) {
  _blit_algo = b;
}

bool Layer::cafudda() {

  if((!active) || (hidden))
    return false;

  offset = (bgcolor) ? bgmatte : get_buffer();
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

  blit(offset);

  filters.unlock();

  signal_feed();

  return(true);
}

void Layer::crop() {
  Uint32 blit_xoff=0;
  Uint32 blit_yoff=0;

  blit_x = geo.x;
  blit_y = geo.y;
  blit_width = geo.w;
  blit_height = geo.h;
  blit_xoff = 0;
  blit_yoff = 0;

  func("CROP layer x[%i] y[%i] w[%i] h[%i] on screen w[%i] h[%i]",
       geo.x,geo.y,geo.w,geo.h,screen->w,screen->h);

  /* left bound 
     affects x-offset and width */
  if(geo.x<0) {
    blit_xoff = (-geo.x);
    blit_x = 1;

    if(blit_xoff>geo.w) {
      func("layer out of screen to the left");
      hidden = true; /* out of the screen */
      geo.x = -(geo.w+1); /* don't let it go far */      
    } else {
      hidden = false;
      blit_width -= blit_xoff;
    }
  }

  /* right bound
     affects width */
  if((geo.x+geo.w)>screen->w) {
    if(geo.x>screen->w) { /* out of screen */
      func("layer out of screen to the right");
      hidden = true; 
      geo.x = screen->w+1; /* don't let it go far */
    } else {
      hidden = false;
      blit_width -= (geo.w - (screen->w - geo.x));
    }
  }

  /* upper bound
     affects y-offset and height */
  if(geo.y<0) {
    blit_yoff = (-geo.y);
    blit_y = 1;

    if(blit_yoff>geo.h) {
      func("layer out of screen up");
      hidden = true; /* out of the screen */
      geo.y = -(geo.h+1); /* don't let it go far */      
    } else {
      hidden = false;
      blit_height -= blit_yoff;
    }
  }

  /* lower bound
     affects height */
  if((geo.y+geo.h)>screen->h) {
    if(geo.y>screen->h) { /* out of screen */
      func("layer out of screen down");
      hidden = true; 
      geo.y = screen->h+1; /* don't let it go far */
    } else {
      hidden = false;
      blit_height -= (geo.h - (screen->h - geo.y));
    }
  }
  

  blit_offset = (blit_xoff<<2) + (blit_yoff*geo.pitch);
  func("LAY BLIT x[%i] y[%i] w[%i] h[%i] xoff[%i] yoff[%i]",
       blit_x, blit_y, blit_width, blit_height, blit_xoff, blit_yoff);
}

#define BLIT(op) \
    { \
      scr = pscr = (uint32_t*) screen->coords(blit_x,blit_y); \
      off = poff = (uint32_t*) ((uint8_t*)video+blit_offset); \
      for(c=blit_height;c>0;c--) { \
	for(cc=blit_width;cc>0;cc--) { \
	  *scr op *off; \
	  scr++; off++; \
	} \
	off = poff = poff + geo.w; \
	scr = pscr = pscr + screen->w; \
      } \
    }

#define BLIT_ALPHA(op) \
    { \
      scr = pscr = (uint32_t *) screen->coords(blit_x,blit_y); \
      off = poff = (uint32_t *) ((uint8_t*)video+blit_offset); \
      for(c=blit_height;c>0;c--) { \
	for(cc=blit_width;cc>0;cc--) { \
	  alpha = (uint8_t *) off; \
	  if(*(alpha+4)) *scr op *off; \
	  scr++; off++; \
	} \
	off = poff = poff + geo.w; \
	scr = pscr = pscr + screen->w; \
      } \
    }


void Layer::blit(void *video) {
  /* transparence aware blits:
     if alpha channel is 0x00 then pixel is not blitted
     works with 32bpp */
  
  if(hidden) return;

  if(alpha_blit) {

    /* ALPHA CHANNEL AWARE BLIT */
    switch(_blit_algo) {
    case 1:
      BLIT_ALPHA(=); return;
      return;
      
    case 2:
    case 3:
    case 4:
      {
	chan = _blit_algo-2;
	char *scr, *pscr, *off, *poff;
	scr = pscr = (char *) screen->coords(blit_x,blit_y);
	off = poff = (char *) ((Uint8*)video+blit_offset);
	for(c=blit_height;c>0;c--) {
	  for(cc=blit_width;cc>0;cc--) {
	    alpha = (Uint8 *) off;
	    if(*(alpha+3)) *(scr+chan) = *(off+chan);
	    scr+=4; off+=4;
	  }
	  off = poff = poff + geo.pitch;
	  scr = pscr = pscr + screen->pitch;
	}
      }
      return;
      
    case 5: BLIT_ALPHA(+=); return;
      
    case 6: BLIT_ALPHA(-=); return;
      
    case 7: BLIT_ALPHA(&=); return;
      
    case 8: BLIT_ALPHA(|=); return;
      
    default: return;
    }

    
  } else {

    /* SOLID COLOR BLIT (NO ALPHA TRANSPARENCE) */
    switch(_blit_algo) {
      
    case 1:
      BLIT(=);
      return;
      
    case 2:
    case 3:
    case 4:
    {
      chan = _blit_algo-2;
      char *scr, *pscr, *off, *poff;
      scr = pscr = (char *) screen->coords(blit_x,blit_y);
      off = poff = (char *) ((Uint8*)video+blit_offset);
      for(c=blit_height;c>0;c--) {
	for(cc=blit_width;cc>0;cc--) {
	  *(scr+chan) = *(off+chan);
	  scr+=4; off+=4;
	}
	off = poff = poff + geo.pitch;
	scr = pscr = pscr + screen->pitch;
      }
    }
    return;
    
    case 5:
      BLIT(+=);
      return;

    case 6:
      BLIT(-=);
      return;

    case 7:
      BLIT(&=);
      return;

    case 8:
      BLIT(|=);
      return;
      
    default: return;
    }
  }
}


/* SIMPLE C CODE
   (where all this blit trip started)
    
  char *scr, *pscr;
  scr = pscr = (char *) screen->coords(geo.x,geo.y);
  char *off, *poff;
  off = poff = (char *)video;
  int c,cc;
  for(c=geo.h;c>0;c--) {
    off = poff = poff + geo.pitch;
    scr = pscr = pscr + screen->pitch;
    for(cc=geo.pitch;cc>0;cc--) *scr++ = *off++;
  }
  */

void Layer::setname(char *s) {
  snprintf(_name,4,"%s",s);
}
char *Layer::getname() { return _name; }

char *Layer::get_blit() {
  switch(_blit_algo) {
  case 1: return "RGB";
  case 2: return "BLU";
  case 3: return "GRE";
  case 4: return "RED";
  case 5: return "ADD";
  case 6: return "SUB";
  case 7: return "AND";
  case 8: return "OR";
  default: return "???";
  }
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
  crop();
  unlock();
}
