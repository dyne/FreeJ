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
#include <keyboard.h>
#include <filter.h>
#include <plugger.h>
#include <config.h>

#define DELAY 100

bool KbdListener::init(Context *context, Plugger *plug) {
  /* saves the pointer to the screen */
  this->screen = context;
  context->kbd = this;
  
  /* saves the pointer to the plugger */
  this->plugger = plug;

  layer = (Layer *)context->layers.begin();
  layersel = 1;

  filter = NULL;
  filtersel = 0;
  _filt = NULL;
  _lastsel = -1;
  plugin_bank = 0;

  quit = false;
  return(true);
}

void KbdListener::run() {
  while(!quit) {

    if(SDL_PollEvent(&event)) {
      switch(event.type) {

      case SDL_MOUSEMOTION:
	if(!layer) break;
	if(event.motion.state & SDL_BUTTON_LEFT) {
	  layer->lock();
	  layer->geo.x += event.motion.xrel;
	  layer->geo.y += event.motion.yrel;
	  layer->crop();
	  layer->unlock();
	}
	break;

      case SDL_KEYDOWN:
	switch(event.key.keysym.sym) {
	  
	case SDLK_ESCAPE:
	  if(event.key.keysym.mod & KMOD_CTRL) {
	    quit = true;
	    screen->quit = true;
	  }
	  else
	    show_osd("press CTRL+ESC if you really want to quit");
	  break;
	  
	case SDLK_SPACE:
	  if(event.key.keysym.mod & KMOD_CTRL)
	    SDL_WM_ToggleFullScreen(screen->surf);
	  else
	    show_osd("press CTRL+SPACE to switch fullscreen");
	  break;
	  
	case SDLK_TAB:
	  if(event.key.keysym.mod & KMOD_CTRL)
	    screen->osd->calibrate();
	  else
	    screen->osd->active();
	  break;

	case SDLK_f:
	  if(event.key.keysym.mod & KMOD_CTRL) {
	    screen->osd->fps();
	    break;
	  }

	case SDLK_UP:
	  if(!layer) break;
	  if(event.key.keysym.mod & KMOD_SHIFT) {
	    layer->lock();
	    layer->geo.y -= 5;
	    layer->crop();
	    layer->unlock();
	    break;
	  }
	case SDLK_DOWN:
	  if(!layer) break;
	  if(event.key.keysym.mod & KMOD_SHIFT) {
	    layer->lock();
	    layer->geo.y += 5;
	    layer->crop();
	    layer->unlock();
	    break;
	  }
	case SDLK_LEFT:
	  if(!layer) break;
	  if(event.key.keysym.mod & KMOD_SHIFT) {
	    layer->lock();
	    layer->geo.x -= 5;
	    layer->crop();
	    layer->unlock();
	    break;
	  }
	case SDLK_RIGHT:
	  if(!layer) break;
	  if(event.key.keysym.mod & KMOD_SHIFT) {
	    layer->lock();
	    layer->geo.x += 5;
	    layer->crop();
	    layer->unlock();
	    break;
	  }

	default:

	  if(!layer) break;

	  /* generic layer operations */
	  if(_layer_op(&event.key.keysym)) break;
	  
	  /* generic filter and context operations */
	  if(_context_op(&event.key.keysym)) break;
	  
	  /* specific layer operations */
	  if(layer->keypress(&event.key.keysym)) break;
	  
	  if(filter!=NULL) {
	    layer->lock();
	    filter->kbd_input(&event.key.keysym);
	    layer->unlock();
	  }
	  break;
	}
	break;

      case SDL_QUIT:
	quit = true;
	screen->quit = true;
	break;
	
      default: break;
      }
    }
    SDL_Delay(DELAY); 
  }
}

bool KbdListener::_layer_op(SDL_keysym *keysym) {
  /* LAYER OPERATIONS */

  if(!layer) return false;
  
  switch(keysym->sym) {

  case SDLK_PAGEUP:
    if(!layer->prev) return false;
    if(keysym->mod & KMOD_CTRL) {
      /* move layer up in chain */
      if(screen->moveup_layer(layersel))
	layersel--;
    } else {
      /* select filter up */
      layer = (Layer *)layer->prev;
      filter = (Filter *)layer->filters.begin();
      filtersel = (filter)?1:0;
      layersel--;
      show_osd("%s <- %u",layer->get_filename(),layersel);
    }
    return true;
    
  case SDLK_PAGEDOWN:
    if(!layer->next) return false;
    if(keysym->mod & KMOD_CTRL) {
      /* move layer down in chain */
      if(screen->movedown_layer(layersel))
	layersel++;
    } else {
      /* select layer down */
      layer = (Layer *)layer->next;
      filter = (Filter *)layer->filters.begin();
      filtersel = (filter)?1:0;
      layersel++;
      show_osd("%s <- %u",layer->get_filename(),layersel);
    }
    return true;

  case SDLK_HOME:
    screen->active_layer(layersel);
    return true;

    /* BLIT ALGOS */
  case SDLK_1: // RGB straight blit
  case SDLK_2: // BLUE CHAN
  case SDLK_3: // GREEN CHAN
  case SDLK_4: // RED CHAN
  case SDLK_5: // PACKED ADD BYTEWISE
  case SDLK_6: // PACKED SUB BYTEWISE
  case SDLK_7: // PACKED AND BYTEWISE
  case SDLK_8: // PACKED OR BYTEWISE
    layer->set_blit(keysym->sym - SDLK_0);
    return true;

  case SDLK_9:
    layer->alpha_blit = !layer->alpha_blit;
    return true;
    
  case SDLK_0:
    screen->clear_all = !screen->clear_all;
    return true;

  case SDLK_BACKSPACE:
    if(keysym->mod & KMOD_SHIFT) /* go to white */
      layer->bgcolor = 1;
    else if(keysym->mod & KMOD_CTRL) /* go to black */
      layer->bgcolor = 2;
    else layer->bgcolor = 0; /* back to layer feed */
    break;
  default: break;
  }

  return false;
}

/* manages keys for operations on the filter chain:
   add/delete/move a filter */

bool KbdListener::_context_op(SDL_keysym *keysym) {
  bool newfilt = false;

  switch(keysym->sym) {
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
      _filt = (*plugger)[_lastsel];
      if(_filt) show_osd("add %s filter?",_filt->getname());
      else show_osd("no filter on bank %X position %u",plugin_bank/10,_sel);

    }
    break;

  default:
    _lastsel = -1;
    break;
  }
  

  switch(keysym->sym) {
    /* FILTER OPERATIONS */
  case SDLK_RETURN:
    if(!_filt) break;
    if(_filt->inuse) break;
    newfilt = true;
    _filt->inuse = true;
    break;

  case SDLK_UP:
    if(!filter) return false;
    if(!filter->prev) return false;

    if(keysym->mod & KMOD_CTRL) {
    /* move filter up in chain */
      if(layer->moveup_filter(filtersel))
	filtersel--;
    } else {
      /* select filter up */
      filter = (Filter *)filter->prev;
      filtersel--;
    }
    return true;
    break;

  case SDLK_DOWN:
    if(!filter) return false;
    if(!filter->next) return false;

    if(keysym->mod & KMOD_CTRL) {
      /* move filter down in chain */
      if(layer->movedown_filter(filtersel))
	filtersel++;
    } else {
      /* select filter down */
      filter = (Filter *)filter->next;
      filtersel++;
    }
    return true;
    break;

  case SDLK_INSERT:
    if(!filter) return false;
    /* switch filter on/off */
    layer->active_filter(filtersel);
    return true;
    break;

  case SDLK_PRINT:
    screen->osd->credits();
    break;

  case SDLK_DELETE:
    if(!filter) return false;
    if(keysym->mod & KMOD_CTRL) {
      /* clear ALL FILTERS */
      layer->clear_filters();
      filter = NULL;
      filtersel = layer->filters.len();
    } else {
      /* clear SELECTED FILTER */
      Filter *tmp = (!filter->prev) ?
	(Filter *)filter->next : (Filter *)filter->prev;
      layer->del_filter(filtersel);
      filter = tmp;
      filtersel = (!filter) ? 0 : (filtersel>1) ? filtersel-1 : 1;
    }
    return true;
    break;
  default:
    break;
  }

  if(newfilt) {
    if(layer->add_filter(_filt)) {
      /* the filter has been accepted on the layer chain */
      filter = _filt;
      filtersel = layer->filters.len();
    } else {
      /* something went wrong... bpp not supported or so */
      // delete filt;
      newfilt = false;
    }
  }
  
  return newfilt;
}
