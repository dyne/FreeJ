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

#include <stdio.h>
#include <string.h>

#include <context.h>
#include <lubrify.h>
#include <osd.h>
#include <config.h>
#include <font_pearl_8x8.h>

void Osd::_write16(char *text, int xpos, int ypos, int hsize, int vsize) {
  int y,x,i,len,f,v,ch,cv;
  Uint16 *ptr;
  Uint16 *diocrap = (Uint16 *)screen->coords(xpos,ypos);
  unsigned char *buffer = (unsigned char *)screen->get_surface();
  v = screen->w*vsize;
  
  len = strlen(text);
  
  /* quest'algoritmo di rastering a grandezza variabile delle font
     e' una cosa di cui vado molto fiero, ogni volta che lo vedo il
     petto mi si gonfia e mi escono sonore scorregge. */
  for (y=0; y<CHAR_HEIGHT; y++) {
    ptr = diocrap += v;
  
    /* control screen bounds */
    if(diocrap-(Uint16 *)buffer>(screen->size-screen->pitch)) return; /* low bound */
    while(diocrap-(Uint16 *)buffer<screen->pitch) ptr = diocrap += v;

    for (x=0; x<len; x++) {
      f = fontdata[text[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH-1; i >= 0; i--)
	if (f & (CHAR_START << i))
	  for(ch=0;ch<hsize;ch++) {
	    for(cv=0;cv<v;cv+=screen->w) ptr[cv] = _color16;
	    ptr++; }
        else ptr+=hsize; 
    }
  }
}

void Osd::_write32(char *text, int xpos, int ypos, int hsize, int vsize) {
  int y,x,i,len,f,v,ch,cv;
  Uint32 *ptr;
  Uint32 *diocrap = (Uint32 *)screen->coords(xpos,ypos);
  unsigned char *buffer = (unsigned char *)screen->get_surface();
  v = screen->w*vsize;
  
  len = strlen(text);
  
  /* quest'algoritmo di rastering a grandezza variabile delle font
     e' una cosa di cui vado molto fiero, ogni volta che lo vedo il
     petto mi si gonfia e mi escono sonore scorregge. */
  for (y=0; y<CHAR_HEIGHT; y++) {
    ptr = diocrap += v;
  
    /* control screen bounds */
    if(diocrap-(Uint32 *)buffer>(screen->size-screen->pitch)) return; /* low bound */
    while(diocrap-(Uint32 *)buffer<screen->pitch) ptr = diocrap += v;

    for (x=0; x<len; x++) {
      f = fontdata[text[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH-1; i >= 0; i--)
	if (f & (CHAR_START << i))
	  for(ch=0;ch<hsize;ch++) {
	    for(cv=0;cv<v;cv+=screen->w) ptr[cv] = _color32;
	    ptr++; }
        else ptr+=hsize; 
    }
  }
}

Osd::Osd() {
  _active = false;
  _calibrate = false;
  _credits = false;
  _fps = false;
  _layersel = 1;
  _filtersel = 0;
  screen = NULL;
}

Osd::~Osd() { }

void Osd::init(Context *screen) {
  this->screen = screen;
  _hbound = screen->w / HBP;
  _vbound = screen->h / VBP;
  screen->osd = this;
  switch(screen->bpp) {
  case 16:
    write = &Osd::_write16;
    break;
  case 32:
    write = &Osd::_write32;
    break;
  }
  _set_color(white);
}

void Osd::print() {
  if(!_active) return;
  
  /* clean up for refresh */
  if(!screen->clear_all) {
    unsigned char *p;
    int c;
    /* clear upper section */
    bzero(screen->get_surface(),screen->pitch*_vbound);
    
    /* clear left section */

    p = (unsigned char *)screen->coords(1,_vbound+TOPLIST);
    for(c=screen->h-(_vbound<<1);c>0;c--) {
      bzero(p,(_hbound<<(screen->bpp>>4))>>1);
      p = (unsigned char *) p+screen->pitch;
    }
    
    /* clear right section */
    p = (unsigned char *)screen->coords(screen->w-17,_vbound+TOPLIST);
    for(c=screen->h-(_vbound<<1);c>0;c--) {
      bzero(p,(_hbound<<(screen->bpp>>4))>>1);
      p = (unsigned char *) p+screen->pitch;
    }

    /* clear lower section */
    bzero(screen->coords(_hbound,screen->h-_vbound),screen->pitch*_vbound);
  }

  if(_calibrate) {
    /* vert up left */
    vline(screen->coords(_hbound,_vbound>>2),VBP<<1,screen->pitch,screen->bpp);
    /* vert up right */
    vline(screen->coords(screen->w-_hbound,_vbound>>2),VBP<<1,screen->pitch,screen->bpp);
    /* vert down left */
    vline(screen->coords(_hbound,screen->h-(_vbound<<1)),VBP<<1,screen->pitch,screen->bpp);
    /* vert down right */
    vline(screen->coords(screen->w-_hbound,screen->h-(_vbound<<1)),VBP<<1,screen->pitch,screen->bpp);
    /* horiz up left */
    hline(screen->coords(_hbound-HBP,_vbound),HBP<<1,screen->bpp);
    /* horiz up right */
    hline(screen->coords(screen->w-_hbound-HBP,_vbound),HBP<<1,screen->bpp);
    /* horiz down left */
    hline(screen->coords(_hbound-HBP,screen->h-_vbound),HBP<<1,screen->bpp);
    /* horiz down right */
    hline(screen->coords(screen->w-_hbound-HBP,screen->h-_vbound),HBP<<1,screen->bpp);
  }
  
  _print_credits();
  _selection();
  if(_fps) _show_fps();
  _filterlist();
  _layerlist();
  _print_status();
}

void Osd::_print_status() {
  _set_color(yellow);
  (this->*write)(status_msg,_hbound,screen->h-_vbound+5,1,1);
}

bool Osd::active() {
  if(_active) clearscr(screen->get_surface(),screen->size);
  _active = !_active;
  return _active;
}

bool Osd::calibrate() {
  if(_calibrate) clearscr(screen->get_surface(),screen->size);
  _calibrate = !_calibrate;
  return _calibrate;
}

bool Osd::fps() {
  if(_fps) clearscr(screen->get_surface(),screen->size);
  _fps = !_fps;
  screen->track_fps = _fps;
  return _fps;
}

void Osd::_show_fps() {
  char fps[10];
  _set_color(white);
  sprintf(fps,"%.1f",screen->fps);
  (this->*write)(fps,screen->w-50,1,1,1);
}

void Osd::_selection() {
  char msg[128];

  _set_color(yellow);

  /* we have only one layer until now */
  if(screen->kbd->filter==NULL)
    sprintf(msg,"%u %s::(none) [%s]",
	    screen->kbd->layersel,
	    screen->kbd->layer->getname(),
	    screen->kbd->layer->get_blit());
  else
    sprintf(msg,"%u %s::%s [%s]",
	    screen->kbd->layersel,
	    screen->kbd->layer->getname(),
	    screen->kbd->filter->getname(),
	    screen->kbd->layer->get_blit());
  (this->*write)(msg,70,1,1,1);
}

void Osd::_layerlist() {
  unsigned int vpos = _vbound+TOPLIST;
  
  _set_color(red);

  Layer *l = (Layer *)screen->layers.begin();
  while(l!=NULL) {
    if( l == screen->kbd->layer) {
      if(l->active)
	(this->*write)(">+",screen->w-17,vpos,1,1);
      else
	(this->*write)(">-",screen->w-17,vpos,1,1);
    } else {
      if(l->active)
	(this->*write)(" +",screen->w-17,vpos,1,1);
      else
	(this->*write)(" -",screen->w-17,vpos,1,1);
    }
    vpos += CHAR_HEIGHT+1;
    l = (Layer *)l->next;
  }
}

void Osd::_filterlist() {
  unsigned int vpos = _vbound+TOPLIST;

  _set_color(red);

  Filter *f = (Filter *)screen->kbd->layer->filters.begin();

  while(f!=NULL) {
    if(f == screen->kbd->filter) {
      if(f->active)
	(this->*write)("*<",1,vpos,1,1);
      else
    	(this->*write)("+<",1,vpos,1,1);
    } else {
      if(f->active) 
	(this->*write)("*",1,vpos,1,1);
      else
	(this->*write)("+",1,vpos,1,1);
    }
    
    vpos += CHAR_HEIGHT+1;
    f = (Filter *)f->next;
  }
}

void Osd::splash_screen() {
  _set_color(white);
  int vpos = _vbound+15;
  (this->*write)("[ d y n e . o r g ] presents:",_hbound+60,vpos,1,1);
  vpos += CHAR_HEIGHT+30;
  (this->*write)(PACKAGE,_hbound+90,vpos,2,2);
  (this->*write)(VERSION,_hbound+180,vpos,2,2);
  vpos += CHAR_HEIGHT+10;
  (this->*write)("PRATERHIMMEL",_hbound+80,vpos,2,2);
  vpos += CHAR_HEIGHT+10;
  (this->*write)(":: set the veejay free ::",_hbound+75,vpos,1,1);
  vpos += CHAR_HEIGHT+30;
  (this->*write)("100% free software for",_hbound+80,vpos,1,2);
  vpos += CHAR_HEIGHT+8;
  (this->*write)("realtime video processing",_hbound+70,vpos,1,2);
  vpos += CHAR_HEIGHT+30;
  (this->*write)("concept and coding by",_hbound+80,vpos,1,1);
  vpos += CHAR_HEIGHT+2;
  (this->*write)("jaromil",_hbound+160,vpos,2,2);
  vpos += CHAR_HEIGHT+30;
  (this->*write)("http://freej.dyne.org",_hbound+85,vpos,1,2);
}

bool Osd::credits() {
  if(_credits) clearscr(screen->get_surface(),screen->size);
  _credits = !_credits;
  return _credits;
}

void Osd::_print_credits() {
  _set_color(green);
  (this->*write)(PACKAGE,6,0,1,1);
  (this->*write)(VERSION,6,9,1,1);
  _set_color(white);
  if(_credits) splash_screen();
}

void Osd::_set_color(colors col) {
  switch(screen->bpp) {
  case 16:
    switch(col) {
    case black:
      _color16 = 0x0000;
      break;
    case white:
      _color16 = 0xffff;
      break;
    case green:
      _color16 = 0x0f0f;
      break;
    case red:
      _color16 = 0xd0e0;
      break;
    case blue:
      _color16 = 0x007f;
      break;
    case yellow:
      _color16 = 0xcea0;
      break;
    }
    break;
  case 32:
    switch(col) {
    case black:
      _color32 = 0x000000;
      break;
    case white:
      _color32 = 0xfefefe;
      break;
    case green:
      _color32 = 0x00ee00;
      break;
    case red:
      _color32 = 0xee0000;
      break;
    case blue:
      _color32 = 0x0000fe;
      break;
    case yellow:
      _color32 = 0xffef00;
      break;
    }
    break;
  }
}
