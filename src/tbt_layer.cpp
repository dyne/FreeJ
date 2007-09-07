/*  FreeJ
 *  (c) Copyright 2006 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 *
 */

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <config.h>
#include <tbt_layer.h>
#include <context.h>
#include <jutils.h>

TBTConsole::TBTConsole()
  :TextConsole() {
  
  // is there scrolling or not?
  scroll = true;

}

TBTConsole::~TBTConsole() {
  // rows are deleted in the parent class destructor
}

void TBTConsole::blank() {

  if(!layer) return;

  SDL_FillRect(layer->surf, NULL, 0x0);

}
    

int TBTConsole::putnch(CHAR *str, int x, int y, int nchars) {
  SDL_Surface *tmp_surf;
  SDL_Rect tmp_rect;

  int strw, strh;

  if(!str) {
    warning("putnch called with NULL string");
    return 0;
  }

  //  func("TBTConsole::putnch '%s' at %i,%i",str,x,y);

  /// render the text into a surface
  tmp_surf = TTF_RenderText_Blended(layer->font, str, layer->fgcolor);
  if(!tmp_surf) {
    error("TTF_RenderText_Blended returns NULL");
    return 0;
  }

  // get the size of the surface
  TTF_SizeText(layer->font, str, &strw, &strh);
  //  posy = strh * y;

  tmp_rect.x = x;
  tmp_rect.y = y * strh;
  tmp_rect.w = strw;
  tmp_rect.h = strh;
  
  // print out the line of text on the final layer surface
  SDL_BlitSurface( tmp_surf, NULL, layer->surf, &tmp_rect );
  
  // delete the temporary surface
  SDL_FreeSurface( tmp_surf );

  return nchars;
}


TBTLayer::TBTLayer()
  :Layer() {

  tbt = NULL;
  font = NULL;
  sel_font=0;
  
  TTF_Init(); // SDL_ttf initialization

  
  
  
  // set defaults
  fgcolor.r = 0xff;
  fgcolor.g = 0xff;
  fgcolor.b = 0xff;
  
  bgcolor.r = 0x00;
  bgcolor.g = 0x00;
  bgcolor.b = 0x00;
  
  set_name("TBT");
  surf = NULL;
  
  console.layer = this;
  
  interactive = true;
  // we poll for keypresses until a .tbt file is loaded
  
}

TBTLayer::~TBTLayer() {
  close();
}

bool TBTLayer::init(Context *freej) {

  env = freej;

  int width  = freej->screen->w;
  int height = freej->screen->h;

  func("TBTLayer::init(%i,%i)",width, height);
  _init(width, height);  

  surf = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA,
			      geo.w, geo.h, 32,
			      red_bitmask, green_bitmask, blue_bitmask, alpha_bitmask);
  SDL_FillRect(surf, NULL, 0x0);

  console.w = width;
  console.h = height;
  console.cur_x = 0;
  console.cur_y = 0;


  func("TBTLayer has %i fonts available",env->num_fonts);
  
  // choose first font and initialize ready for printing
  size = 30;
  
  font = TTF_OpenFont(env->font_files[sel_font], size);
  if (!font)
    error("Couldn't load %d pt font from %s: %s\n",
	  size, env->font_files[sel_font], SDL_GetError());
  
  TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
  // here can be also: TTF_STYLE_BOLD _ITALIC _UNDERLINE

  return true;
}

bool TBTLayer::open(char *file) {

  func("TBTLayer::open(%s)",file);

  tbt = new TBT();
  if( ! tbt->load(file) ) {
    error("can't open TBT file %s",file);
    return false;
  }

  interactive = false;

  return true;
}

void *TBTLayer::feed() {

  if(!interactive)
    console.feed( tbt->getkey() );

  return surf->pixels;
}

void TBTLayer::close() {
  if(surf)   SDL_FreeSurface(surf);
  //  if(letter) SDL_FreeSurface(letter);
  if(font)   TTF_CloseFont(font);
  TTF_Quit();

  if(tbt) delete tbt;

}

bool TBTLayer::keypress(int key) {

  if(interactive) {
    console.feed(key);
    return true;
  }
  
  return false;
}


