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
#include <font_pearl_8x8.h>
#include <config.h>

void Osd::_write(char *text, int xpos, int ypos, int hsize, int vsize) {
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
  screen->osd = this;
  _set_color(white);
  func("OSD initialized");
}

void Osd::print() {
  if(!_active) return;
  
  /* clean up for refresh
     mmxosd_clean(screen->get_surface(),0x0,screen->w,screen->h); */

  if(_calibrate) {
    /* vert up left */
    vline(screen->coords(HBOUND,VBOUND>>2),VBP<<1,screen->pitch,screen->bpp);
    /* vert up right */
    vline(screen->coords(screen->w-HBOUND,VBOUND>>2),VBP<<1,screen->pitch,screen->bpp);
    /* vert down left */
    vline(screen->coords(HBOUND,screen->h-(VBOUND<<1)),VBP<<1,screen->pitch,screen->bpp);
    /* vert down right */
    vline(screen->coords(screen->w-HBOUND,screen->h-(VBOUND<<1)),VBP<<1,screen->pitch,screen->bpp);
    /* horiz up left */
    hline(screen->coords(HBOUND-HBP,VBOUND),HBP<<1,screen->bpp);
    /* horiz up right */
    hline(screen->coords(screen->w-HBOUND-HBP,VBOUND),HBP<<1,screen->bpp);
    /* horiz down left */
    hline(screen->coords(HBOUND-HBP,screen->h-VBOUND),HBP<<1,screen->bpp);
    /* horiz down right */
    hline(screen->coords(screen->w-HBOUND-HBP,screen->h-VBOUND),HBP<<1,screen->bpp);
  }
  
  _print_credits();

  if(_fps) _show_fps();

  if(screen->kbd) {
    _layerlist();
    if(screen->kbd->layer) {
      _filterlist();
      _selection();
    }
  }

  _print_status();
}

void Osd::_print_status() {
  _set_color(yellow);
  _write(status_msg,HBOUND,screen->h-12,1,1);
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
  _write(fps,screen->w-50,1,1,1);
}

void Osd::_selection() {
  char msg[256];

  _set_color(yellow);

  sprintf(msg,"%u %s::%s [%s][%s][%s]",
	  screen->kbd->layersel,
	  screen->kbd->layer->getname(),
	  (screen->kbd->filter)?screen->kbd->filter->getname():"(null)",
	  screen->kbd->layer->get_blit(),
	  (screen->kbd->layer->alpha_blit)?"@":" ",
	  (screen->clear_all)?"0":" ");

  _write(msg,80,1,1,1);
}

void Osd::_layerlist() {
  unsigned int vpos = VBOUND+TOPLIST;
  
  _set_color(red);

  Layer *l = (Layer *)screen->layers.begin();
  while(l) {
    if( l == screen->kbd->layer) {
      if(l->active)
	_write(">+",screen->w-17,vpos,1,1);
      else
	_write(">-",screen->w-17,vpos,1,1);
    } else {
      if(l->active)
	_write(" +",screen->w-17,vpos,1,1);
      else
	_write(" -",screen->w-17,vpos,1,1);
    }
    vpos += CHAR_HEIGHT+1;
    l = (Layer *)l->next;
  }
}

void Osd::_filterlist() {
  unsigned int vpos = VBOUND+6;

  _set_color(red);

  Filter *f = (Filter *)screen->kbd->layer->filters.begin();

  while(f) {
    if(f == screen->kbd->filter) {
      if(f->active)
	_write("*<",1,vpos,1,1);
      else
    	_write("+<",1,vpos,1,1);
    } else {
      if(f->active) 
	_write("*",1,vpos,1,1);
      else
	_write("+",1,vpos,1,1);
    }
    
    vpos += CHAR_HEIGHT+1;
    f = (Filter *)f->next;
  }
}

void Osd::splash_screen() {
  _set_color(white);
  int vpos = VBOUND+15;
  _write(PACKAGE,HBOUND+100,vpos,2,2);
  _write(VERSION,HBOUND+190,vpos,2,2);
  vpos += CHAR_HEIGHT+10;
  _write("HURRIA",HBOUND+130,vpos,2,2);
  vpos += CHAR_HEIGHT+10;
  _write(":: set the veejay free ::",HBOUND+75,vpos,1,2);
  vpos += CHAR_HEIGHT+30;
  _write("100% free software for",HBOUND+80,vpos,1,2);
  vpos += CHAR_HEIGHT+8;
  _write("realtime video processing",HBOUND+70,vpos,1,2);
  vpos += CHAR_HEIGHT+30;
  _write("sourcecode available on",HBOUND+80,vpos,1,1);
  vpos += CHAR_HEIGHT+3;
  _write("http://freej.dyne.org",HBOUND+85,vpos,1,2);
  vpos += CHAR_HEIGHT+40;
  _write("| software by jaromil",HBOUND+90,vpos,1,1);
  vpos += CHAR_HEIGHT+2;
  _write("| copyleft 2001, 2002",HBOUND+90,vpos,1,1);
}

bool Osd::credits() {
  if(_credits) clearscr(screen->get_surface(),screen->size);
  _credits = !_credits;
  return _credits;
}

void Osd::_print_credits() {
  _set_color(green);
  _write(PACKAGE,6,0,1,1);
  _write(VERSION,6,9,1,1);
  _set_color(white);
  if(_credits) splash_screen();
}

void Osd::_set_color(colors col) {
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
}
