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

#include <iostream.h>

#include <context.h>
#include <jutils.h>
#include <keyboard.h>
#include <filter.h>
#include <plugger.h>

#define DELAY 300

bool KbdListener::init(Context *context, Plugger *plug) {
  /* saves the pointer to the scren */
  this->screen = context;
  context->kbd = this;
  
  /* saves the pointer to the plugger */
  this->plugger = plug;

  layer = (Layer *)context->layers.begin();

  filter = NULL;
  filtersel = 0;
  _filt = NULL;
  _lastsel = -1;

  quit = false;
  return(true);
}

void KbdListener::run() {
  while(!quit) {

    if(SDL_PollEvent(&event)) {
      if(event.type==SDL_KEYDOWN) {

	switch(event.key.keysym.sym) {
	case SDLK_ESCAPE:
	  quit = true;
	  break;
	case SDLK_SPACE:
	  SDL_WM_ToggleFullScreen(screen->surf);
	  break;
	case SDLK_TAB:
	  if(event.key.keysym.mod == KMOD_LCTRL)
	    screen->osd->calibrate();
	  else
	    screen->osd->active();
	  break;
	  /* commented out until layer is just one
	     case SDLK_PAGEUP:
	     if(layer->next!=NULL)
	     layer = (Layer *) layer->next;
	     break;
	     case SDLK_PAGEDOWN:
	     if(layer->prev!=NULL)
	     layer = (Layer *) layer->prev;
	     break; */
	default:
	  if(event.key.keysym.mod!=KMOD_CAPS) {
	    if(_context_op(&event.key.keysym)) break;
	    if(layer->keypress(&event.key.keysym)) break;
	  }
	  if(filter!=NULL) filter->kbd_input(&event.key.keysym);
	  break;
	}

      } else if(event.type==SDL_QUIT) quit = true;
    }
    SDL_Delay(DELAY);
  }
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
    if(_lastsel==(keysym->sym - SDLK_F1)) break;
    _lastsel = (keysym->sym - SDLK_F1);
    _filt = (*plugger)[_lastsel];
    if(_filt) show_osd("add %s filter?",_filt->getname());
    break;
  default:
    _lastsel = -1;
    break;
  }
  
  switch(keysym->sym) {
  case SDLK_RETURN:
    if(!_filt) break;
    if(_filt->inuse) break;
    newfilt = true;
    _filt->inuse = true;
    break;
  case SDLK_UP:
    if(filter==NULL) return false;
    if(filter->prev==NULL) return false;

    if(keysym->mod == KMOD_RCTRL) {
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
    if(filter==NULL) return false;
    if(filter->next==NULL) return false;

    if(keysym->mod == KMOD_RCTRL) {
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
    if(filter==NULL) return false;
    /* switch filter on/off */
    layer->active_filter(filtersel);
    return true;
    break;

  case SDLK_PRINT:
    screen->osd->credits();
    break;

  case SDLK_DELETE:
    if(filter==NULL) return false;
    if(keysym->mod==KMOD_RCTRL) {
      /* clear ALL FILTERS */
      layer->clear_filters();
      filter = NULL;
      filtersel = layer->filters.len();
    } else {
      /* clear SELECTED FILTER */
      Filter *tmp = (filter->prev==NULL) ? (Filter *)filter->next : (Filter *)filter->prev;
      layer->del_filter(filtersel);
      filter = tmp;
      filtersel = (filter==NULL) ? 0 : (filtersel>1) ? filtersel-1 : 1;
    }
    return true;
    break;
  default:
    break;
  }

  if(newfilt) {
    func("keyboard says newfilt");
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
