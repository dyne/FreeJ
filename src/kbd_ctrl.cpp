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

#define SDL_REPEAT_DELAY	200
#define SDL_REPEAT_INTERVAL	10

#define DELAY 50 //100

#define LISTEN_EVENT_TYPES (SDL_KEYDOWN|SDL_MOUSEMOTION|SDL_MOUSEBUTTONDOWN)

KbdListener::KbdListener() {
  _filt = NULL;
  _lastsel = -1;
  plugin_bank = 0;
  drag_value = false;
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

  /* enable key repeat */
  SDL_EnableKeyRepeat(SDL_REPEAT_DELAY, SDL_REPEAT_INTERVAL);

  return(true);
}

void KbdListener::run() {

  //  while(!quit) {

  //    SDL_Delay(DELAY); 

    if(!SDL_PollEvent(&event)) return;
	
    if(event.type == SDL_QUIT) {
      func("SDL QUIT!");

      env->quit = true;
      return; 
    }

    if(event.type == SDL_VIDEORESIZE) {
      env->resize(event.resize.w, event.resize.h);
      return;
    }
      
    if(event.type == SDL_MOUSEBUTTONDOWN) {

      if(event.button.button == SDL_BUTTON_LEFT) {

	layer->blitter.set_colorkey(event.button.x, event.button.y);

      } else if(event.button.button == SDL_BUTTON_RIGHT) {
	
	if(SDL_ShowCursor(-1)==0) SDL_ShowCursor(1);
	else SDL_ShowCursor(0);

      }

      return;
      
    } else if (drag_value) {
      if(event.type == SDL_MOUSEMOTION
	 && layer->blitter.current_blit->has_value) {

	layer->blitter.set_value
	  // make the value proportional to the screen height
	  // fit it into a 0-255 bound
	  // swap the direction (up higher, down lower)
	  (0xff-((event.motion.y<<8)/env->screen->h));

      return;
      }
    }      
    
    if(event.key.state != SDL_PRESSED) return;
    
    /* ENVIRONMENT CONTROLS */
    keysym = &event.key.keysym; /* just to type less */

      
    switch(keysym->sym) {
    case SDLK_ESCAPE:
      if(keysym->mod & KMOD_CTRL) {
        env->quit = true;
      } else
	show_osd("press CTRL+ESC if you really want to quit");
      break;
      
      // removed the global pause because of key confusion
      // with the space in the console, not de/activating layers
      // jrml 19 oct04 thanks to Robert
      //    case SDLK_SPACE:
      //      env->pause = !env->pause;
      //      break;

      /* BROKEN ATM TODO QUAA      
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
      if(keysym->mod & KMOD_CTRL) {
	env->magnify(keysym->sym-SDLK_0-1);
	return;
      }
      break;
      */
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

    case SDLK_GREATER:
      env->fps_speed++;
//      printf("+ %d\n",env->fps_speed);
      env->set_fps_interval(env->fps_speed);
      break;
    case SDLK_LESS:
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

    /* mouse drag 
    if(event.type & SDL_MOUSEMOTION)
      if(event.motion.state & SDL_BUTTON_LEFT)
	layer->set_position
	  ( layer->geo.x + event.motion.xrel,
	    layer->geo.y + event.motion.yrel );
      else if(event.motion.state & SDL_BUTTON_RIGHT)
	layer->set_position( event.motion.x, event.motion.y );
    i'm not sure why, but this doesn't works now */

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

    case SDLK_SPACE:
      layer->active = !layer->active;
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
      if(keysym->mod & KMOD_SHIFT)
	layer->set_position(layer->geo.x,layer->geo.y-1);
      break;
    case SDLK_DOWN:
      if(keysym->mod & KMOD_SHIFT)
	layer->set_position(layer->geo.x,layer->geo.y+1);
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
      layer->blitter.set_blit("RGB");
      break;
    case SDLK_2: // RED CHAN
      layer->blitter.set_blit("RED");
      break;
    case SDLK_3: // GREEN CHAN
      layer->blitter.set_blit("GREEN");
      break;
    case SDLK_4: // BLUE CHAN
      layer->blitter.set_blit("BLUE");
      break;
    case SDLK_5: // PACKED ADD BYTEWISE
      layer->blitter.set_blit("ADD");
      break;
    case SDLK_6: // PACKED SUB BYTEWISE
      layer->blitter.set_blit("SUB");
      break;
    case SDLK_7: // PACKED AND BYTEWISE
      layer->blitter.set_blit("AND");
      break;
    case SDLK_8: // PACKED OR BYTEWISE
      layer->blitter.set_blit("OR");
      break;
    case SDLK_9: // ALPHA TRANSPARENT SDL BLIT
      layer->blitter.set_blit("ALPHA");
      break;

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
      if(keysym->mod & KMOD_SHIFT)
	layer->set_position(layer->geo.x-1,layer->geo.y);
      break;
    case SDLK_RIGHT:
      if(keysym->mod & KMOD_SHIFT)
	layer->set_position(layer->geo.x+1,layer->geo.y);
      break;

    case SDLK_v:
      if(keysym->mod & KMOD_CTRL) {
	drag_value = !drag_value;
	show_osd("Mouse fade %s",
		 (drag_value)?"ON":"OFF");
	break;
      }

    default:
      /* layer implementation opcode
	 TODO: here should pass opcodes instead of SDL keys */
      layer->lock();
      layer->keypress(keysym->sym);
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

    case SDLK_PLUS:
      filter->up();
      break;
    case SDLK_MINUS:
      filter->down();
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
      //      if(event.key.keysym.mod & KMOD_SHIFT)
      //	filter->kbd_input(event.key.keysym.sym+'A');
      //      else
      filter->kbd_input(event.key.keysym.sym);
      layer->unlock();
      break;
    }

    //  }
}

