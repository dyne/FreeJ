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
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include <context.h>
#include <plugger.h>
#include <osd.h>
#include <font_pearl_8x8.h>
#include <config.h>

uint32_t *Osd::print(char *text, uint32_t *pos, int hsize, int vsize) {
  int y,x,i,len,f,v,ch,cv;
  uint32_t *ptr;
  uint32_t *diocrap = pos; //(uint32_t *)env->coords(xpos,ypos);
  unsigned char *buffer = (unsigned char *)env->get_surface();
  v = env->w*vsize;
  
  len = strlen(text);
  
  /* quest'algoritmo di rastering a grandezza variabile delle font
     e' una cosa di cui vado molto fiero, ogni volta che lo vedo il
     petto mi si gonfia e mi escono sonore scorregge. */
  for (y=0; y<CHAR_HEIGHT; y++) {
    ptr = diocrap += v;
  
    /* control screen bounds */
    if(diocrap-(uint32_t *)buffer>(env->size - env->pitch)) 
      return diocrap-newline; /* low bound */
    while(diocrap-(uint32_t *)buffer<env->pitch) ptr = diocrap += v;

    for (x=0; x<len; x++) {
      f = fontdata[text[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH-1; i >= 0; i--)
	if (f & (CHAR_START << i))
	  for(ch=0;ch<hsize;ch++) {
	    for(cv=0;cv<v;cv+=env->w) ptr[cv] = _color32;
	    ptr++; }
        else ptr+=hsize; 
    }
  }
  return(diocrap);
}

Osd::Osd() {
  _active = false;
  _calibrate = false;
  _credits = false;
  _fps = false;
  _layersel = 1;
  _filtersel = 0;
  env = NULL;
}

Osd::~Osd() { }

void Osd::init(Context *screen) {
  this->env = screen;
  screen->osd = this;
  _set_color(white);

  /*
  ipernaut = create_layer("../doc/ipernav.png");
  sflescio = env->plugger->pick("vertigo");
  if(sflescio) ipernaut->add_filter(sflescio);
  if(ipernaut) env->add_layer(ipernaut);
  */
  fps_offset = (uint32_t*)env->coords(env->w-50,1);
  selection_offset = (uint32_t*)env->coords(80,1);
  status_offset = (uint32_t*)env->coords(HBOUND,env->h-12);
  layer_offset = (uint32_t*)env->coords(env->w-28,VBOUND+TOPLIST);
  filter_offset = (uint32_t*)env->coords(5,VBOUND+6);
  hicredits_offset = (uint32_t*)env->coords((env->w/2)-100,VBOUND+5);
  locredits_offset = (uint32_t*)env->coords((env->w/2)-100,env->h-70);
  hilogo_offset = (uint32_t*)env->coords(6,0);
  newline = env->pitch*(CHAR_HEIGHT);
  snprintf(title,64,"%s v.%s",PACKAGE,VERSION);
  func("OSD initialized");

}

void Osd::print() {
  if(!_active) return;

  /*  
  if(_calibrate) {
    // vert up left
    vline(screen->coords(HBOUND,VBOUND>>2),VBP<<1,screen->pitch,screen->bpp);
    // vert up right
    vline(screen->coords(screen->w-HBOUND,VBOUND>>2),VBP<<1,screen->pitch,screen->bpp);
    // vert down left
    vline(screen->coords(HBOUND,screen->h-(VBOUND<<1)),VBP<<1,screen->pitch,screen->bpp);
    // vert down right
    vline(screen->coords(screen->w-HBOUND,screen->h-(VBOUND<<1)),VBP<<1,screen->pitch,screen->bpp);
    // horiz up left
    hline(screen->coords(HBOUND-HBP,VBOUND),HBP<<1,screen->bpp);
    // horiz up right
    hline(screen->coords(screen->w-HBOUND-HBP,VBOUND),HBP<<1,screen->bpp);
    // horiz down left
    hline(screen->coords(HBOUND-HBP,screen->h-VBOUND),HBP<<1,screen->bpp);
    // horiz down right
    hline(screen->coords(screen->w-HBOUND-HBP,screen->h-VBOUND),HBP<<1,screen->bpp);
  } */
  
  _print_credits();

  if(_fps) _show_fps();

  //  if(screen->kbd) {
  _layerlist();

  Layer *lay = (Layer*)env->layers.selected();
  if(lay) {
    _filterlist();
    _selection();
  }

  _print_status();
}

void Osd::_print_status() {
  _set_color(yellow);
  print(status_msg,status_offset,1,1);
}

bool Osd::active() {
  if(_active) env->clear(); //scr(screen->get_surface(),screen->size);
  _active = !_active;
  return _active;
}

bool Osd::calibrate() {
  if(_calibrate) env->clear(); //scr(screen->get_surface(),screen->size);
  _calibrate = !_calibrate;
  return _calibrate;
}

bool Osd::fps() {
  if(_fps) env->clear(); //scr(screen->get_surface(),screen->size);
  _fps = !_fps;
  env->track_fps = _fps;
  return _fps;
}

void Osd::_show_fps() {
  char fps[10];
  _set_color(white);
  sprintf(fps,"%.1f",env->fps);
  print(fps,fps_offset,1,1);
}

void Osd::_selection() {
  char msg[256];

  _set_color(yellow);

  Layer *lay = (Layer*) env->layers.selected();
  if(!lay) return;
  
  Filter *filt = (Filter*) lay->filters.selected();
  sprintf(msg,"%s::%s [%s][%s][%s]",
	  lay->getname(),
	  (filt)?filt->getname():" ",
	  lay->get_blit(),
	  (lay->alpha_blit)?"@":" ",
	    (env->clear_all)?"0":" ");
  
  print(msg,selection_offset,1,1);
  
}

void Osd::statusmsg(char *format, ...) {
  va_list arg;
  va_start(arg,format);
  vsnprintf(status_msg,49,format,arg);
  va_end(arg);
}

void Osd::_layerlist() {
  //  unsigned int vpos = VBOUND+TOPLIST;
  uint32_t *pos = layer_offset;
  //  _set_color(red);

  Layer *l = (Layer *)env->layers.begin(),
    *laysel = (Layer*) env->layers.selected();

  while(l) {
    char *lname = l->getname();

    
    if( l == laysel) {

      if(l->active) {
	/* red color */ _color32 = 0xee0000;
	pos = print(lname,pos+4,1,1) - 4;
      } else {
	/* dark red color */ _color32 = 0x880000;	
	pos = print(lname,pos+4,1,1) - 4;
      }

    } else {

      if(l->active) {
	/* red color */ _color32 = 0xee0000;
	pos = print(lname,pos,1,1);
      } else {
	/* dark red color */ _color32 = 0x880000;	
	pos = print(lname,pos,1,1);
      }

    }
    //    pos += newline;
      //    vpos += CHAR_HEIGHT+1;
    l = (Layer *)l->next;
  }
}

void Osd::_filterlist() {
  //  unsigned int vpos = VBOUND+6;
  uint32_t *pos = filter_offset;

  char fname[4];
  Layer *lay = (Layer*) env->layers.selected();
  if(!lay) return;
  Filter *f = (Filter *)lay->filters.begin();
  Filter *filtsel = (Filter*)lay->filters.selected();
  while(f) {
    strncpy(fname,f->getname(),3); fname[3] = '\0';
    
    if(f == filtsel) {

      if(f->active) {
	/* red color */ _color32 = 0xee0000;
	pos = print(fname,pos+4,1,1) - 4;
      } else {
	/* dark red color */ _color32 = 0x880000;
    	pos = print(fname,pos+4,1,1) - 4;
      }

    } else {

      if(f->active) {
	/* red color */ _color32 = 0xee0000;
	pos = print(fname,pos,1,1);
      } else {
	/* dark red color */ _color32 = 0x880000;
	pos = print(fname,pos,1,1);
      }

    }
    
    //    vpos += CHAR_HEIGHT+1;
    //    pos += newline;
    f = (Filter *)f->next;
  }
}

void Osd::splash_screen() {
  _set_color(white);

  //  int vpos = VBOUND+5;
  uint32_t *pos = hicredits_offset;
  pos = print(title,pos,2,2);
  //  pos += newline;
  pos = print("MONTEVIDEO",pos,2,2);
  //  vpos += CHAR_HEIGHT+10;
  //  pos += newline;
  pos = print(":: set the veejay free ",pos,1,2);
  //  vpos += CHAR_HEIGHT+30;
  

  /*
  _write("100% free software for",HBOUND+40,vpos,1,2);
  vpos += CHAR_HEIGHT+8;
  _write("realtime video processing",HBOUND+40,vpos,1,2);
  vpos += CHAR_HEIGHT+30;
  _write("sourcecode available on",HBOUND+40,vpos,1,1);
  vpos += CHAR_HEIGHT+3;
  _write("http://freej.dyne.org",HBOUND+40,vpos,1,2);
  vpos += CHAR_HEIGHT+40;
  */
  pos = locredits_offset;
  pos = print("| by rastasoft.org",pos,1,2);
  //vpos += CHAR_HEIGHT+10;
  pos = print("| copyleft 2001 - 2003",pos,1,2);
  //  vpos += CHAR_HEIGHT+10;
  pos = print("| jaromil @ dyne.org",pos,1,2);
}

bool Osd::credits() {
  if(_credits) env->clear(); //scr(env->get_surface(),env->size);
  _credits = !_credits;
  return _credits;
}

void Osd::_print_credits() {
  _set_color(green);
  uint32_t *pos = print(PACKAGE,hilogo_offset,1,1);
  print(VERSION,pos,1,1);
  _set_color(white);
  if(_credits) splash_screen();
}

void Osd::_set_color(colors col) {
  switch(col) {
  case black:
    _color32 = 0x88000000;
    break;
  case white:
    _color32 = 0x88fefefe;
    break;
  case green:
    _color32 = 0x8800ee00;
    break;
  case red:
    _color32 = 0x88ee0000;
    break;
  case blue:
    _color32 = 0x880000fe;
    break;
  case yellow:
    _color32 = 0x88ffef00;
    break;
  }
}

void Osd::clean() {
  int c,cc;
  int jump = (env->w - HBOUND - HBOUND) / 2;
  uint64_t *top = (uint64_t*)env->get_surface();
  uint64_t *down = (uint64_t*)env->coords(0,env->h-VBOUND);
  
  for(c=(VBOUND*(env->w>>1));c>0;c--) {
    *top = 0x0; *down = 0x0;
    top++; down++;
  }
  for(c = env->h-VBOUND-VBOUND; c>0; c--) {
    for(cc = HBOUND>>1; cc>0; cc--) { *top = 0x0; top++; }
    top+=jump;
    for(cc = HBOUND>>1; cc>0; cc--) { *top = 0x0; top++; }
  }
}
