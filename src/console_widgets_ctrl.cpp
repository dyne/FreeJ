/*  FreeJ - S-Lang console
 *
 *  (c) Copyright 2004-2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <config.h>

#include <slang.h>

#include <slw.h>

#include <console_ctrl.h>
#include <console_widgets_ctrl.h>

#include <context.h>
#include <layer.h>
#include <blitter.h>

#include <jutils.h>

SlwSelector::SlwSelector() 
  : SLangWidget() {
  
  env = NULL;
  
}

SlwSelector::~SlwSelector() {
  
}

bool SlwSelector::init() {

}

bool SlwSelector::feed(int key) {

}

bool SlwSelector::refresh() {
  int layercol, pos;

  /* print info the selected layer */

  layer = env->layers.selected();
  if(layer) {
    ///////////////
    // layer print
    SLsmg_gotorc(2,1);
    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_string((char *)"Layer: ");
    SLsmg_set_color(LAYERS_COLOR+10);
    SLsmg_write_string(layer->get_filename());
    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_char(' ');
    SLsmg_write_string((char *)"blit: ");
    SLsmg_set_color(LAYERS_COLOR+10);
    SLsmg_write_string(layer->current_blit->name);
    SLsmg_write_char(' ');
    SLsmg_printf((char *)"[%.0f]",layer->current_blit->value);
    SLsmg_write_char(' ');
    SLsmg_set_color(LAYERS_COLOR);
    SLsmg_write_string((char *)"geometry: ");
    SLsmg_set_color(LAYERS_COLOR+10);
    SLsmg_printf((char *)"x%i y%i w%u h%u",
		 layer->geo.x, layer->geo.y,
		 layer->geo.w, layer->geo.h);
    SLsmg_erase_eol();
  }

  if(env->layers.len()) {
    //////////////////
    // layer list
    SLsmg_gotorc(4,1);

    /* take layer selected and first */
    if(layer)
      filter = layer->filters.selected();
    
    Layer *l = env->layers.begin();  
    int color;

    while(l) { /* draw the layer's list */
      
      SLsmg_set_color(LAYERS_COLOR);
      SLsmg_write_string((char *)" -> ");
      color=LAYERS_COLOR;

      if( l == layer && !filter) {
	color+=20;
	layercol = SLsmg_get_column();
      }
      
      if(l->fade | l->active) color+=10;
      SLsmg_set_color (color);
      SLsmg_printf((char *)"%s",l->get_name());
      l = (Layer *)l->next;
    }

    SLsmg_set_color(PLAIN_COLOR);
    SLsmg_erase_eol();
    
  }
  

  if(layer) {
    FilterInstance *f;

    filter = layer->filters.selected();
    
    SLsmg_gotorc(3,1);
    SLsmg_set_color(FILTERS_COLOR);
    SLsmg_write_string((char *)"Filter: ");
    if(!filter) {
      SLsmg_write_string((char *)"none selected");
      SLsmg_set_color(PLAIN_COLOR);
      SLsmg_erase_eol();
    } else {
      SLsmg_set_color(FILTERS_COLOR+10);
      SLsmg_write_string(filter->name);
      SLsmg_erase_eol();
      SLsmg_forward(2);
      SLsmg_write_string((char *)filter->proto->description());
      SLsmg_set_color(PLAIN_COLOR);
    }
    
    f = layer->filters.begin();
    while(f) {
      
      SLsmg_set_color(PLAIN_COLOR);
      SLsmg_gotorc(pos,0);
      SLsmg_erase_eol();
      
      SLsmg_gotorc(pos,layercol);
      color=FILTERS_COLOR;
      if( f == filter ) color+=20;
      if( f->active) color+=10;
      SLsmg_set_color (color);
      
      SLsmg_printf((char *)"%s",f->name);
      
      pos++;
      f = (FilterInstance*)f->next;
    }

    SLsmg_set_color(PLAIN_COLOR);
    for(;pos<5;pos++) {
      SLsmg_gotorc(pos,0);
      SLsmg_erase_eol();
    }
    
  }
  
}

