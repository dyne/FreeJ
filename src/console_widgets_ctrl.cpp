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

#include <slang_console_ctrl.h>
#include <console_widgets_ctrl.h>

#include <context.h>
#include <layer.h>
#include <blitter.h>
#include <video_encoder.h>

#include <jutils.h>


SlwTitle::SlwTitle()
  : SLangWidget() {
  env = NULL;
}

SlwTitle::~SlwTitle() { }

bool SlwTitle::init() {

  snprintf(title, 256,
	   " %s version %s | set the veejay free! | freej.dyne.org | ", PACKAGE, VERSION);
  titlelen = strlen(title);
  initialized = true;
  return(true);
}

bool SlwTitle::feed(int key) { return(false); }

bool SlwTitle::refresh() {
  color = TITLE_COLOR+20;
  putnch(title, 0, 0, titlelen);
   /* this is RASTA SOFTWARE! */
  color = 32;  putnch("RAS", titlelen, 0, 3);
  color = 34;  putnch("TAS", titlelen+3, 0, 3);
  color = 33;  putnch("OFT", titlelen+6, 0, 3);
  color = PLAIN_COLOR;

  return(true);
}


////////////////////////////////////////////////



SlwSelector::SlwSelector() 
  : SLangWidget() {
  
  env = NULL;
  tmp = NULL;
  
}

SlwSelector::~SlwSelector() {
  if(tmp) free(tmp);
}

bool SlwSelector::init() {
  tmp = (char*)calloc(w, sizeof(char));
  initialized = true;
    return true;
}

bool SlwSelector::feed(int key) {
  Entry *le, *fe;

  bool res = false;

  if(env->screens.selected()->layers.len() > 0) { // there are layers

    res = true;

    // get the one selected
    le = env->screens.selected()->layers.selected();
    if(!le) {
      env->screens.selected()->layers.begin();
      le->sel(true);
    }

    fe = ((Layer*)le)->filters.selected();
    
    // switch over operations and perform
    switch(key) {
      
    case SL_KEY_UP:
      
      if(!fe) break; // no filter
      
      fe = fe->prev; // take the upper one
      ((Layer*)le)->filters.sel(0); // deselect all filters
      if(fe) fe->sel(true); // select only the current
      
      break;
      
    case SL_KEY_DOWN:
      
      if(!fe) {
	fe = ((Layer*)le)->filters.begin();
	if(!fe) break; // no filters
	else fe->sel(true);
      } else if(fe->next) {
	fe = fe->next;
	((Layer*)le)->filters.sel(0);
	fe->sel(true);
      }
      break;
      
    case SL_KEY_LEFT:
    
      if(!fe) { // no filter selected, move across layers
	
	// move to the previous or the other end
	if(!le->prev)
	  le = env->screens.selected()->layers.end();
	else
	  le = le->prev;
	
	// select only this layer
	env->screens.selected()->layers.sel(0);
	le->sel(true);
	
      } else { // a filter is selected: move across filter parameters
	
	// TODO
	
      }
      
      break;
      
    case SL_KEY_RIGHT:

      if(!fe) { // no filter selected, move across layers
	
	// move to the next layer or the other end
	if(!le->next)
	  le = env->screens.selected()->layers.begin();
	else le = le->next;
	
	// select only the current
	env->screens.selected()->layers.sel(0);
	le->sel(true);
	
      } else { // move across filter parameters
	
	// TODO
	
      }
      break;
      
    case SL_KEY_PPAGE:
    case KEY_PLUS:
      if(fe) fe->up();
      else   le->up();
      break;
      
    case SL_KEY_NPAGE:
    case KEY_MINUS:
      if(fe) fe->down();
      else   le->down();
      break;
      
    case SL_KEY_DELETE:
    case KEY_CTRL_D:
      if(fe) {
	fe->rem();
	delete fe;
      } else {
	//	le->rem();
	//	((Layer*)le)->close();
	env->rem_layer( (Layer*)le );
      }
      break;
      
    case KEY_SPACE:
      if(fe) ((FilterInstance*)fe)->active =
	       !((FilterInstance*)fe)->active;
      else  ((Layer*)le)->active =
	      !((Layer*)le)->active;
    break;

    default:
      res = false;
      break;
    }
  }
  return(res);
}

bool SlwSelector::refresh() {
  int sellayercol, layercol, pos;

  /* print info the selected layer */
  blank();

  // also put info from encoders, if active
  // so far supported only one encoder
  VideoEncoder *enc = env->screens.selected()->encoders.begin();
  if(enc) {
    snprintf(tmp, w, "Stream: video %u kb/s : audio %u kb/s : encoded %u kb",
	     enc->video_kbps, enc->audio_kbps, enc->bytes_encoded / 1024);
    putnch(tmp, 1, 0, 0);
  }
    
  layer = env->screens.selected()->layers.selected();
  if(layer) {
    snprintf(tmp, w, "Layer: %s blit: %s [%.0f] geometry x%i y%i w%u h%u",
	     layer->get_filename(), layer->current_blit->name, layer->current_blit->value,
	     layer->geo.x, layer->geo.y, layer->geo.w, layer->geo.h);
    //    SLsmg_erase_eol();
  } else {
    sprintf(tmp, "No Layer selected" );
  }
  ///////////////
  // layer print
  color = LAYERS_COLOR;
  putnch(tmp, 1, 1, 0);


  if(env->screens.selected()->layers.len()) {
    Layer *l = env->screens.selected()->layers.begin();  
    //    int color;
    int tmpsize = 0;
    layercol = 0;

    //////////////////
    // layer list
    //    SLsmg_gotorc(4,1);

    /* take layer selected and first */
    if(layer)
      filter = layer->filters.selected();
    
    while(l) { /* draw the layer's list */
      layercol += tmpsize + 4;
      //      SLsmg_set_color(LAYERS_COLOR);
      //      SLsmg_write_string((char *)" -> ");
      color = LAYERS_COLOR;
      putnch(" ->", layercol, 2, 3);

      if( l == layer && !filter) color+=20;
      if(l->fade | l->active) color+=10;      

      //      snprintf(tmp, w, " -> %s",l->get_name());
      tmpsize = strlen(l->get_name());
      putnch(l->get_name(), layercol+4, 2, tmpsize);
      // save position of selected layer
      if( l == layer) sellayercol = layercol;
      
      l = (Layer *)l->next;
    }

    
  }
  
  
  if(layer) {
    FilterInstance *f;
    
    filter = layer->filters.selected();
    
//     SLsmg_gotorc(3,1);
//     SLsmg_set_color(FILTERS_COLOR);
//     SLsmg_write_string((char *)"Filter: ");
    if(!filter) {
      snprintf(tmp, w, "No Filter selected");
      //      SLsmg_erase_eol();
    } else {
      snprintf(tmp, w, "Filter: %s :: %s", filter->name, filter->proto->description());
      //      SLsmg_set_color(FILTERS_COLOR+10);

      //      SLsmg_erase_eol();
    }
    
    f = layer->filters.begin();
    pos = 4;
    while(f) {
      
//       SLsmg_set_color(PLAIN_COLOR);
//       SLsmg_gotorc(pos,0);
//       SLsmg_erase_eol();

      color = FILTERS_COLOR;
//       SLsmg_gotorc(pos,layercol);
      if( f == filter ) color+=20;
      if( f->active) color+=10;
//       SLsmg_set_color (color);
      putnch(f->name, sellayercol+4, pos, 0);
      pos++;
      f = (FilterInstance*)f->next;
    }

//     SLsmg_set_color(PLAIN_COLOR);
//     for(;pos<5;pos++) {
//       SLsmg_gotorc(pos,0);
//       SLsmg_erase_eol();
//     }
    
  } else snprintf(tmp, w, "No Filter selected");

  color = FILTERS_COLOR;
  putnch(tmp,1,3,0);
  return true;
}

