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

#include <layer.h>
#include <context.h>
#include <lubrify.h>
#include <jutils.h>
#include <config.h>

Layer::Layer() {
  paused = false;
  quit = false;
  active = false;
  hidden = false;
  alpha_blit = false;
  blit_offset = 0;
  _blit_algo = 1;
  setname("???");
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

  _w = geo.w; _h = geo.h;
  _pitch = geo.pitch;
  _size = geo.size;
  screen->add_layer(this);
}

void Layer::run() {
  active = true;
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
    if(!newfilt->init(&geo)) {
      error("cannot initialize plugin %s",newfilt->getname());
      return false;
    }
    func("initialization OK");
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
  filt->initialized = false;
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
    f->initialized = false;
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

void Layer::set_blit(int b) {
  _blit_algo = b;
}

bool Layer::cafudda() {

  if((!active) || (hidden))
    return false;

  void *offset = get_buffer();
  if(!offset) {
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
    if(filt->active) offset = filt->process(offset);
    filt = (Filter *)filt->next;
  }

  lock_feed();

  blit(offset);

  /* pitch is width in bytes */

  unlock();
  unlock_feed();

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

  func("CROP layer x[%i] y[%i] w[%i] h[%i] on screen w[%i] h[%i]",geo.x,geo.y,geo.w,geo.h,screen->w,screen->h);

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
      Uint8 *alpha; \
      Uint32 *scr, *pscr; \
      scr = pscr = (Uint32 *) screen->coords(blit_x,blit_y); \
      Uint32 *off, *poff; \
      off = poff = (Uint32 *) ((Uint8*)offset+blit_offset); \
      int c, cc; \
      for(c=blit_height;c>1;c--) { \
	for(cc=blit_width;cc>1;cc--) { \
	  alpha = (Uint8 *) off; \
	  if(*(alpha+4)) *scr op *off; \
	  scr++; off++; \
	} \
	off = poff = poff + geo.w; \
	scr = pscr = pscr + screen->w; \
      } \
    }

void Layer::blit(void *offset) {
  /* transparence aware blits:
     if alpha channel is 0x00 then pixel is not blitted
     works with 32bpp */
  
  if(hidden) return;

  if(alpha_blit) {


    /* ALPHA CHANNEL AWARE BLIT */
    switch(_blit_algo) {
    case 1:
      BLIT(=); return;
      return;
      
    case 2:
    case 3:
    case 4:
      {
	Uint8 *alpha;
	int chan = _blit_algo-2;
	char *scr, *pscr;
	scr = pscr = (char *) screen->coords(blit_x,blit_y);
	char *off, *poff;
	off = poff = (char *) ((Uint8*)offset+blit_offset);
	int c,cc;
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
      
    case 5: BLIT(+=); return;
      
    case 6: BLIT(-=); return;
      
    case 7: BLIT(&=); return;
      
    case 8: BLIT(|=); return;
      
    default: return;
    }

    
  } else {


    /* SOLID COLOR BLIT (NO ALPHA TRANSPARENCE) */
    switch(_blit_algo) {
      
    case 1:
      mmxblit((Uint8*)offset+blit_offset,screen->coords(blit_x,blit_y),
	      blit_width,blit_height,geo.pitch,screen->pitch); 
      return;
      
    case 2:
    case 3:
    case 4:
    {
      int chan = _blit_algo-2;
      char *scr, *pscr;
      scr = pscr = (char *) screen->coords(blit_x,blit_y);
      char *off, *poff;
      off = poff = (char *) ((Uint8*)offset+blit_offset);
      int c,cc;
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
      mmxblit_add(offset,screen->coords(geo.x,geo.y),geo.h,geo.pitch,screen->pitch);
      return;

    case 6:
      mmxblit_sub(offset,screen->coords(geo.x,geo.y),geo.h,geo.pitch,screen->pitch);
      return;

    case 7:
      mmxblit_and(offset,screen->coords(geo.x,geo.y),geo.h,geo.pitch,screen->pitch);
      return;

    case 8:
      mmxblit_or(offset,screen->coords(geo.x,geo.y),geo.h,geo.pitch,screen->pitch);
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
  off = poff = (char *)offset;
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
  case 1: return "MMX";
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
