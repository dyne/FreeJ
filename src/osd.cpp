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
#include <screen.h>
#include <font_pearl_8x8.h>
#include <config.h>

#define HBOUND 32
#define VBOUND 18

#define VBP 16 /* vertical bound proportion */
#define HBP 13 /* horizontal bound proportion */
#define TOPLIST 6 /* distance down from vbound where they start the vertical lists */


uint32_t *Osd::print(char *text, uint32_t *pos, int hsize, int vsize) {
  uint32_t *diocrap = pos; //(uint32_t *)env->coords(xpos,ypos);
  unsigned char *buffer = (unsigned char *)env->screen->get_surface();
  v = env->screen->w*vsize;
  
  //  len = strlen(text);
  
  /* quest'algoritmo di rastering a grandezza variabile delle font
     e' una cosa di cui vado molto fiero, ogni volta che lo vedo il
     petto mi si gonfia e mi escono sonore scorregge. */
  for (y=0; y<CHAR_HEIGHT; y++) {
    ptr = diocrap += v;
  
    /* control screen bounds */
    if(diocrap-(uint32_t *)buffer>(env->screen->size - env->screen->pitch)) 
      return diocrap-newline; /* low bound */
    while(diocrap-(uint32_t *)buffer<env->screen->pitch) ptr = diocrap += v;

    //    for (x=0; x<len; x++) {
    x=0;
    while(text[x]!='\0') {
      f = fontdata[text[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH-1; i >= 0; i--)
	if (f & (CHAR_START << i))
	  for(ch=0;ch<hsize;ch++) {
	    for(cv=0;cv<v;cv+=env->screen->w)
	      ptr[cv] = _color32;
	    ptr++; }
        else ptr+=hsize; 
      x++;
    }
  }
  return(diocrap);
}

Osd::Osd() {
  active = false;
  _calibrate = false;
  _credits = false;
  _fps = false;
  _layersel = 1;
  _filtersel = 0;
  ipernaut = NULL;
  osd_vertigo = NULL;
  env = NULL;
}

Osd::~Osd() { }

void Osd::init(Context *screen) {
  this->env = screen;
  _set_color(white);

  /* setup coordinates for OSD information
     stored in offsets of video memory addresses
     to gain speed and limit computation during cycles */
  fps_offset = (uint32_t*)env->screen->coords(env->screen->w-50,1);
  selection_offset = (uint32_t*)env->screen->coords(80,1);
  status_offset = (uint32_t*)env->screen->coords(HBOUND,env->screen->h-12);
  layer_offset = (uint32_t*)env->screen->coords(env->screen->w-28,VBOUND+TOPLIST);
  filter_offset = (uint32_t*)env->screen->coords(3,VBOUND+6);
  hicredits_offset = (uint32_t*)env->screen->coords((env->screen->w/2)-140,VBOUND+5);
  locredits_offset1 = (uint32_t*)env->screen->coords((env->screen->w/2)+10,env->screen->h-VBOUND-30);
  locredits_offset2 = (uint32_t*)env->screen->coords((env->screen->w/2)-150,env->screen->h-VBOUND-33);
  hilogo_offset = (uint32_t*)env->screen->coords(3,0);
  topclean_offset = (uint64_t*)env->screen->coords(0,0);
  downclean_offset = (uint64_t*) env->screen->coords(0,env->screen->h-VBOUND);

  newline = env->screen->pitch*(CHAR_HEIGHT);
  snprintf(title,64,"%s v%s codename MONTEVIDEO",PACKAGE,VERSION);

  /* add ipernaut logo layer */
  if(!ipernaut) {
    char tmp[512];
    sprintf(tmp,"%s/freej/ipernav.png",DATADIR);
    ipernaut = create_layer(tmp);    
    if(!ipernaut) ipernaut = create_layer("../doc/ipernav.png");
    if(!ipernaut) ipernaut = create_layer("doc/ipernav.png");
    if(!ipernaut) ipernaut = create_layer("ipernav.png");
    if(ipernaut) {
      if(ipernaut->init(env)) {
	ipernaut->set_blit(9);
	ipernaut->set_alpha(128);
      } else {
	delete ipernaut;
	ipernaut = NULL;
      }
    }
  }

  func("OSD initialized");
  
}

/* this function is called by the core routine
   at every cycle */
void Osd::print() {

  if(!active) return;
  
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

  env->screen->lock();
  
  Layer *lay = (Layer*)env->layers.selected();
  if(lay) _filterlist();
  _selection();
  _layerlist();
  
  if(_credits) {
    _print_credits();
    _show_fps();
  }
  
  _print_status();
  
  env->screen->unlock();
}

void Osd::_print_status() {
  unsigned char c;
  int wstride = env->screen->w-CHAR_WIDTH;
  //  _set_color(yellow);
  //  print(status_msg,status_offset,1,1);
  ptr = status_offset;

  for (x=0; status_msg[x]!='\0'; x++) {
    for (y=0; y<CHAR_HEIGHT; y++) {
      c = fontdata[status_msg[x] * CHAR_HEIGHT + y];
      for (i = CHAR_WIDTH; i > 0; i--) {
	if (c & (CHAR_START << i))
	  *ptr = 0x00ffef00;
	ptr++;
      }
      ptr += wstride;
    }
    ptr = status_offset + ((x+1)*CHAR_WIDTH);
  }
}

bool Osd::calibrate() {
  //  if(_calibrate) env->clear(); //scr(screen->get_surface(),screen->size);
  _calibrate = !_calibrate;
  return _calibrate;
}

bool Osd::fps() {
  //  if(_fps) env->clear(); //scr(screen->get_surface(),screen->size);
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
  sprintf(msg,"%s::%s [%s][%s]",
	  lay->get_name(),
	  (filt)?filt->getname():" ",
	  lay->get_blit(),
	    (env->clear_all)?"0":" ");
  
  print(msg,selection_offset,1,1);
  
}
/*
void Osd::statusmsg(char *format, ...) {
  if(_credits) credits();
  va_list arg;
  va_start(arg,format);
  vsnprintf(status_msg,49,format,arg);
  status_msg[50] = '\0';
  va_end(arg);
}
*/
void Osd::_layerlist() {
  char *lname;
  //  unsigned int vpos = VBOUND+TOPLIST;
  uint32_t *pos = layer_offset;

  /* controls to turn off credits */
  bool credits_on = false;

  _set_color(red);

  env->layers.lock();
  Layer *l = (Layer *)env->layers.begin(),
    *laysel = (Layer*) env->layers.selected();

  while(l) {

    /* turn off credits if there are other layers */
    if(l==ipernaut) {
      credits_on = true;
    } else if(credits_on) {
      env->layers.unlock();
      credits(false);
      env->layers.lock();
      l = (Layer *)l->next;
      continue;
    }
      
    lname = l->get_name();

    
    if( l == laysel) {

      if(l->active) {
	/* red color */ _color32 = 0xee0000;
	pos = print(lname,pos-4,1,1) + 4;
      } else {
	/* dark red color */ _color32 = 0x880000;	
	pos = print(lname,pos-4,1,1) + 4;
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
    l = (Layer *)l->next;
  }
  env->layers.unlock();
}

void Osd::_filterlist() {
  //  unsigned int vpos = VBOUND+6;
  uint32_t *pos = filter_offset;
  _set_color(red);

  char fname[4];
  Layer *lay = (Layer*) env->layers.selected();
  if(!lay) return;

  lay->filters.lock();
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
    f = (Filter *)f->next;
  }
  lay->filters.unlock();
}

void Osd::draw_credits() {
  uint32_t *pos;
  _set_color(white);
  pos = hicredits_offset;
  pos = print(title,pos,1,2);
  pos = print("free realtime video processing",pos,1,1);
  pos = locredits_offset1;
  pos = print("GNU GPL (c)2001-04",pos,1,1);
  pos = print("jaromil @ dyne.org",pos,1,1);
  pos = locredits_offset2;
  print("freej.org",pos,2,2);

}

bool Osd::credits() { return credits(!_credits); }
bool Osd::credits(bool s) {

  //  env->clear_once = true; //scr(env->get_surface(),env->size);
  _credits = s;

  if(_credits) {
    env->track_fps = true;
    if(ipernaut) {

      /* add first vertigo effect on logo */
      if(!osd_vertigo)
	osd_vertigo = env->plugger.pick("vertigo");
      if(osd_vertigo)
	if(!osd_vertigo->list) {
	  osd_vertigo->init(&ipernaut->geo);
	  ipernaut->filters.prepend(osd_vertigo);
	}
      env->layers.prepend(ipernaut);
    }

#if 0
    /* add a second water effect on logo */
    osd_water = env->plugger.pick("water");
    if(osd_water) {
      osd_water->init(&ipernaut->geo);
      keysym.sym = SDLK_y; osd_water->kbd_input(&keysym);
      keysym.sym = SDLK_w; osd_water->kbd_input(&keysym);
      keysym.sym = SDLK_w; osd_water->kbd_input(&keysym);
      keysym.sym = SDLK_w; osd_water->kbd_input(&keysym);
      ipernaut->filters.add(osd_water);
    } 
#endif
      
    draw_credits();

  }  else {
    env->track_fps = false;
    if(ipernaut) ipernaut->rem();
    if(osd_vertigo) {
      osd_vertigo->rem();
      osd_vertigo->clean();
    }
  }
  return _credits;
}

void Osd::_print_credits() {
  if(_credits) draw_credits();
  else {
    _set_color(green);
    uint32_t *pos = print(PACKAGE,hilogo_offset,1,1);
    print(VERSION,pos,1,1);
  }
}

void Osd::_set_color(colors col) {
  switch(col) {
  case black:
    _color32 = 0x00000000;
    break;
  case white:
    _color32 = 0x00fefefe;
    break;
  case green:
    _color32 = 0x0000ee00;
    break;
  case red:
    _color32 = 0x00ee0000;
    break;
  case blue:
    _color32 = 0x000000fe;
    break;
  case yellow:
    _color32 = 0x00ffef00;
    break;
  }
}

void Osd::clean() {
  //  if(!active) return;
  int c,cc;
  int jump = (env->screen->w - HBOUND - HBOUND) / 2;
  uint64_t *top = topclean_offset;
  uint64_t *down = downclean_offset;

  env->screen->lock();
  for(c=(VBOUND*(env->screen->w>>1));c>0;c--) {
    *top = 0x0; *down = 0x0;
    top++; down++;
  }
  for(c = env->screen->h-VBOUND-VBOUND; c>0; c--) {
    for(cc = HBOUND>>1; cc>0; cc--) { *top = 0x0; top++; }
    top+=jump;
    for(cc = HBOUND>>1; cc>0; cc--) { *top = 0x0; top++; }
  }
  env->screen->unlock();
}
