/*  FreeJ
 *  (c) Copyright 2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
#include <text_layer.h>
#include <jutils.h>

TTFLayer::TTFLayer()
  :Layer() {

  font = NULL;
  num_fonts=0;
  sel_font=0;

  TTF_Init();

  // parse all font directories  
  scanfonts("/usr/X11R6/lib/X11/fonts/TTF");
  scanfonts("/usr/X11R6/lib/X11/fonts/truetype");
  scanfonts("/usr/X11R6/lib/X11/fonts/TrueType");
  scanfonts("/usr/share/truetype");

  if(!num_fonts) {
    error("no truetype fonts found on your system, dirs searched:");
    error("/usr/X11R6/lib/X11/fonts/TTF");
    error("/usr/X11R6/lib/X11/fonts/truetype");
    error("/usr/X11R6/lib/X11/fonts/TrueType");
    error("/usr/share/truetype");
    error("you should install .ttf fonts in one of the directories above.");
  } else {
    func("TxtLayer fonts %i",num_fonts);

  // choose first font and initialize ready for printing
    size = 30;
    
    font = TTF_OpenFont(font_files[sel_font], size);
    if (!font)
      error("Couldn't load %d pt font from %s: %s\n",
	    size, font_files[sel_font], SDL_GetError());
    
    TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
    // here can be also: TTF_STYLE_BOLD _ITALIC _UNDERLINE
  }

  // set defaults
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

bool TTFLayer::init(int width, int height) {
  // this is skipped for its functionality
  // in fact the size is changing at every new print
  // so we'll call the Layer::_init(wdt,hgt) many times

  return true;
}

void TTFLayer::close() {
  if(surf) SDL_FreeSurface(surf);
  if(font) TTF_CloseFont(font);
  TTF_Quit();
}

bool TTFLayer::keypress(char key) { return false; };

void TTFLayer::print(char *str) {
  SDL_Surface *tmp;
  int x, y;

  // lock everything here
  lock();

  if(surf) SDL_FreeSurface(surf);
  
  surf = TTF_RenderText_Shaded(font, str, fgcolor, bgcolor);
  tmp = SDL_DisplayFormat(surf);
  if(tmp) {
    SDL_FreeSurface(surf);
    surf = tmp;
  }

  //  SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0)
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
  // just return the surface
  return surf->pixels;
}

int ttf_dir_selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".ttf")) return(1);
  if(strstr(dir->d_name,".TTF")) return(1);
  return(0);
}
int TTFLayer::scanfonts(char *path) {
  /* add to the list of available fonts */
  struct dirent **filelist;
  char temp[256];
  int found;
  int num_before = num_fonts;
  found = scandir(path,&filelist,ttf_dir_selector,alphasort);
  if(found<0) {
    func("no fonts found in %s : %s",path, strerror(errno)); return(false); }
  while(found--) {
    if(num_fonts>=MAX_FONTS) break;
    snprintf(temp,255,"%s/%s",path,filelist[found]->d_name);
    font_files[num_fonts] = strdup(temp);
    num_fonts++;
  }
  func("scanfont found %i fonts in %s",num_fonts-num_before,path);
  return(num_fonts - num_before);
}
