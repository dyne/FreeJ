/*  FreeJ
 *  (c) Copyright 2005-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: layer.h 670 2005-08-21 18:49:32Z jaromil $"
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
#include <context.h>
#include <text_layer.h>
#include <jutils.h>

TTFLayer::TTFLayer()
  :Layer() {

  // set defaults

  font = NULL;
  sel_font = 20;

  size = 30;

  fgcolor.r = 0xff;
  fgcolor.g = 0xff;
  fgcolor.b = 0xff;

  bgcolor.r = 0x00;
  bgcolor.g = 0x00;
  bgcolor.b = 0x00;

  set_name("TTF");
  surf = NULL;

}


TTFLayer::~TTFLayer() { 
  close();
}

bool TTFLayer::open(char *file) {
  // open a file and read its contents here?
  // skip it for now, this layer is mostly used for realtime printing
  // would be cool to implement something like justification and paragraph
  // formatting for long texts, left for later on...

  return true;
}

bool TTFLayer::init(Context *freej) {
  // width/height is skipped for its functionality
  // in fact the size is changing at every new print
  // so we'll call the Layer::_init(wdt,hgt) many times

  if( ! TTF_WasInit() )
    TTF_Init();

  env = freej;

  return true;
}

void TTFLayer::close() {
  // free sdl font surface
  if(surf) SDL_FreeSurface(surf);

  // close up sdl ttf
  if(font) TTF_CloseFont(font);

  if( TTF_WasInit() ) TTF_Quit();
}

void TTFLayer::set_size(int nsize) {
  TTF_Font *tmp;

  tmp = TTF_OpenFont(env->font_files[sel_font], nsize);

  if(!tmp) {

    error("Couldn't load %d pt font from %s: %s\n",
	  size, env->font_files[sel_font], SDL_GetError());

  } else {

    lock();

    size = nsize;
    if(font) TTF_CloseFont(font);
    font = tmp;
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);

    unlock();

  }

}

void TTFLayer::calculate_string_size(char *text, int *w, int *h) {
  TTF_SizeText(font, text, w, h);
}

bool TTFLayer::keypress(int key) { return false; };

void TTFLayer::print(char *str) {
  SDL_Surface *tmp;
  int x, y;
  
  // choose first font and initialize ready for printing
  
  if(!env) {
    error("TextLayer: can't print, environment is not yet assigned neither a font is selected");
    error("call add_layer or choose a font for the layer");
    return;
  }

  if(!font) {
    func("no font selected on layer %s, using default %s",
	    this->name, env->font_files[sel_font]);

    font = TTF_OpenFont(env->font_files[sel_font], size);
    if (!font) {
      error("Couldn't load %d pt font from %s: %s\n",
	    size, env->font_files[sel_font], SDL_GetError());
      return;
    }
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    // here can be also: TTF_STYLE_BOLD _ITALIC _UNDERLINE
  }

  // lock everything here
  lock();

  if(surf) SDL_FreeSurface(surf);
  
  // surf = TTF_RenderText_Blended(font, str, fgcolor);
  surf = TTF_RenderText_Shaded(font, str, fgcolor, bgcolor);
  tmp = SDL_DisplayFormat(surf);
  if(tmp) {
    SDL_FreeSurface(surf);
    surf = tmp;
  }

  //  SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0);
    //    error("TTFLayer::print : couldn't set text colorkey: %s", SDL_GetError());

  // save original positions
  x = geo.x;
  y = geo.y;
  _init(surf->w, surf->h);  
  geo.x = x;
  geo.y = y;

  unlock();

}

void *TTFLayer::feed() {
  if(!surf) {
    warning("Text layer has no rendered surface");
    return NULL;
  }
  // just return the surface
  return surf->pixels;
}

static int ttf_dir_selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".ttf")) return(1);
  if(strstr(dir->d_name,".TTF")) return(1);
  return(0);
}
int Context::scanfonts(char *path) {
  /* add to the list of available fonts */
  struct dirent **filelist;
  char temp[1024];
  int found, c;
  int num_before = num_fonts;

  found = scandir(path,&filelist,ttf_dir_selector,alphasort);

  if(found<0) {
    func("no fonts found in %s : %s",path, strerror(errno));
    return(false);
  } else
    act("%u fonts found in %s", found, path);

  if(!font_files) { // first allocation
    font_files = (char**) calloc(found, sizeof(char*));
  } else {
    font_files = (char**) realloc(font_files, (found + num_fonts)*sizeof(char*) );
  }

  for(c=0; c<found; c++) {

    if(c>=MAX_FONTS) break;
    
    snprintf(temp,1024,"%s/%s",path,filelist[c]->d_name);
    font_files[num_fonts] = (char*)calloc(strlen(temp) + 5, sizeof(char));
    strcpy(font_files[num_fonts], temp);

    free(filelist[c]);
	 
    num_fonts++;

  }

  free(filelist);

  return(num_fonts - num_before);
}
