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

#include <iostream>

#include <context.h>
#include <jutils.h>
#include <filter.h>
#include <plugger.h>
#include <kbd_ctrl.h>
#include <config.h>

#define DELAY 50 //100

#define LISTEN_EVENT_TYPES (SDL_KEYDOWN|SDL_MOUSEMOTION)

static bool alpha = false;

KbdListener::KbdListener() {
  _filt = NULL;
  _lastsel = -1;
  plugin_bank = 0;
  active = false;
}

KbdListener::~KbdListener() {

}

bool KbdListener::init(Context *context) {

  if(!context) return false;

  /* saves the pointer to the environment */
  this->env = context;
  
  //  start();

  active = true;

  return(true);
}

void KbdListener::run() {

  //  while(!quit) {

  //    SDL_Delay(DELAY); 

    if(!SDL_PollEvent(&event)) return;
	
    if(event.type == SDL_QUIT) {
      func("SDL QUIT!");

      env->quit = true;
      return; }
	
    if(event.type != SDL_KEYDOWN) return;

    
    /* ENVIRONMENT CONTROLS */
    keysym = &event.key.keysym; /* just to type less */

      
    switch(keysym->sym) {
    case SDLK_ESCAPE:
      if(keysym->mod & KMOD_CTRL) {
        env->quit = true;
      } else
	show_osd("press CTRL+ESC if you really want to quit");
      break;
      
    case SDLK_SPACE:
      env->pause = !env->pause;
      break;
      
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
      if(keysym->mod & KMOD_CTRL) {
	env->magnify(keysym->sym-SDLK_0-1);
	return;
      }
      break;

    case SDLK_TAB:
      if(keysym->mod & KMOD_CTRL)
	env->osd.calibrate();
      else
	env->osd.active = !env->osd.active;
      break;

    case SDLK_PRINT:
      env->osd.credits();
      break;
    
    case SDLK_f:
      if(keysym->mod & KMOD_CTRL)
	env->screen->fullscreen();
      break;

    case SDLK_0:
      env->clear_all = !env->clear_all;
      break;

    case SDLK_KP_PLUS:
      env->fps_speed++;
//      printf("+ %d\n",env->fps_speed);
      env->set_fps_interval(env->fps_speed);
      break;
    case SDLK_KP_MINUS:
      env->fps_speed--;
//      printf("- %d\n",env->fps_speed);
      env->set_fps_interval(env->fps_speed);
      break;

    default:
      _lastsel = -1;
      break;
    }

    /* LAYER CONTROLS */
    layer = (Layer *)env->layers.selected();
    if(!layer)
      layer = (Layer*) env->layers.begin();
    if(!layer) return; /* there are no layers */
    else {
      env->layers.sel(0);
      layer->sel(true); /* select the first */
    }

    /* mouse drag */
    if(event.type & SDL_MOUSEMOTION)
      if(event.motion.state & SDL_BUTTON_LEFT)
	layer->set_position
	  ( layer->geo.x + event.motion.xrel,
	    layer->geo.y + event.motion.yrel );
      else if(event.motion.state & SDL_BUTTON_RIGHT)
	layer->set_position( event.motion.x, event.motion.y );
    
    switch(keysym->sym) {

      /* filter selection */
    case SDLK_F1:
    case SDLK_F2:
    case SDLK_F3:
    case SDLK_F4:
    case SDLK_F5:
    case SDLK_F6:
    case SDLK_F7:
    case SDLK_F8:
    case SDLK_F9:
    case SDLK_F10:
    case SDLK_F11:
    case SDLK_F12:
      _sel = keysym->sym-SDLK_F1;    
      if(keysym->mod & KMOD_CTRL) { /* plugin bank shift */
	char tmp[] = "plugin bank [ ------------ ]";
	sprintf(&tmp[_sel+14],"%X",_sel); 
	tmp[_sel+15] = (_sel<11) ? '-' : ' ';
	plugin_bank = _sel*12;
	show_osd(tmp);
      } else {
	if(_lastsel==_sel+plugin_bank) break;
	_lastsel = (_sel+plugin_bank);
	_filt = env->plugger[_lastsel];
	if(_filt) show_osd("add %s filter?",_filt->getname());
	else show_osd("no filter on bank %X position %u",plugin_bank/10,_sel);
      }
      break;

    case SDLK_RETURN:
      /* add a new filter to the selected layer */
      if(!_filt) break;
      if(_filt->list) {
	show_osd("Filter %s is allready in use",_filt->getname());
	break;
      }
      if(!_filt->init(&layer->geo)) {
	show_osd("filter %s doesn't initializes",_filt->getname());
	break;
      }
      layer->filters.add(_filt);
      /* filter is automatically selected */
      filter = _filt;
      layer->filters.sel(0);
      filter->sel(true);
      break;
      
    case SDLK_UP:
      if(keysym->mod & KMOD_SHIFT) {
	layer->set_position(layer->geo.x,layer->geo.y-5);
	break;
      }
      break;
    case SDLK_DOWN:
      if(keysym->mod & KMOD_SHIFT) {
	layer->set_position(layer->geo.x,layer->geo.y+5);
	break;
      }
      break;

    case SDLK_PAGEUP:
      if(!layer->prev) break;
      if(keysym->mod & KMOD_CTRL) {
	/* move layer up in chain */
	layer->up();
      } else {
	/* select layer up */
	layer = (Layer *)layer->prev;
	env->layers.sel(0);
	layer->sel(true);
	show_osd("%s :: %s",layer->get_name(),layer->get_filename());
      }
      break;
      
    case SDLK_PAGEDOWN:
      if(!layer->next) break;
      if(keysym->mod & KMOD_CTRL) {
	layer->down();
      } else {
	/* select layer down */
	layer = (Layer *)layer->next;
	env->layers.sel(0);
	layer->sel(true);
	show_osd("%s :: %s",layer->get_name(),layer->get_filename());
      }
      break;
      
    case SDLK_HOME:
      layer->active = !layer->active;
      break;




      /* BLIT ALGOS */
    case SDLK_1: // RGB straight blit
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("MEMCPY");
      break;
    case SDLK_2: // RED CHAN
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("RED");
      break;
    case SDLK_3: // GREEN CHAN
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("GREEN");
      break;
    case SDLK_4: // BLUE CHAN
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("BLUE");
      break;
    case SDLK_5: // PACKED ADD BYTEWISE
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("ADD");
      break;
    case SDLK_6: // PACKED SUB BYTEWISE
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("SUB");
      break;
    case SDLK_7: // PACKED AND BYTEWISE
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("AND");
      break;
    case SDLK_8: // PACKED OR BYTEWISE
      if(alpha)
	layer->blitter.set_value( 255 * (keysym->sym - SDLK_0) /8 );
      else
	layer->blitter.set_blit("OR");
      break;
    case SDLK_9:
      if(alpha) {
	layer->blitter.set_blit("MEMCPY");
	alpha = false;
      } else {
	layer->blitter.set_blit("ALPHA");
	alpha = true;
      }

    case SDLK_BACKSPACE:
      if(keysym->mod & KMOD_SHIFT) /* go to white */
	layer->bgcolor = 1;
      else if(keysym->mod & KMOD_CTRL) /* go to black */
	layer->bgcolor = 2;
      else layer->bgcolor = 0; /* back to layer feed */
      break;


    case SDLK_DELETE:
      if(keysym->mod & KMOD_CTRL) {
	func("Keyboard CLEAR ALL FILTERS");
	/* clear ALL FILTERS */
	layer->filters.clear();
	filter = NULL;
	break;
      }

    case SDLK_LEFT:
      if(keysym->mod & KMOD_SHIFT) {
	layer->set_position(layer->geo.x-5,layer->geo.y);
	break;
      }
    case SDLK_RIGHT:
      if(keysym->mod & KMOD_SHIFT) {
	layer->set_position(layer->geo.x+5,layer->geo.y);
	break;
      }
      
    default:
      /* layer implementation opcode
	 TODO: here should pass opcodes instead of SDL keys */
      layer->lock();
      layer->keypress(keysym);
      layer->unlock();
      break;
      
    } /* END OF LAYER CONTROLS */


    /* FILTER CONTROLS */
    filter = (Filter *)layer->filters.selected();
    if(!filter) {
      filter = (Filter*) layer->filters.begin();
      if(!filter) return; /* there are no filters */
      else {
	layer->filters.sel(0);
	filter->sel(true); /* select the first */
      }
    }
    
    switch(keysym->sym) {
      /* FILTER OPERATIONS */

    case SDLK_UP:
      if(!filter->prev) break;
      if(keysym->mod & KMOD_CTRL) {
	/* move filter up in chain */
	filter->up();
      } else {
	/* select filter up */
	filter = (Filter *)filter->prev;
	layer->filters.sel(0);
	filter->sel(true);
      }
      break;
    case SDLK_DOWN:
      if(!filter->next) break;
      if(keysym->mod & KMOD_CTRL) {
	/* move filter down in chain */
	filter->down();
      } else {
	/* select filter down */
	filter = (Filter *)filter->next;
	layer->filters.sel(0);
	filter->sel(true);
      }
      break;
      
    case SDLK_INSERT:
    /* switch filter on/off */
      filter->active = !filter->active;
      break;

    case SDLK_DELETE:
      /* clear SELECTED FILTER
	 and move selection to the one up, or down
      */
      {
	Filter *tmp = (!filter->prev) ?
	  (Filter *)filter->next : (Filter *)filter->prev;
	filter->rem();
	layer->filters.sel(0);
	if(tmp) tmp->sel(true);
	filter->clean();
	filter = tmp;
	break;
      }

    default:
      /* passes opcode to layer implementation */
      layer->lock();
      filter->kbd_input(&event.key.keysym);
      layer->unlock();
      break;
    }

    //  }
}

