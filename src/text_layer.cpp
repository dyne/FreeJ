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

#include <config.h>
#ifdef WITH_FT2

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>

#include <context.h>
#include <text_layer.h>
#include <jutils.h>
#include <jsparser_data.h>

TextLayer::TextLayer()
  :Layer() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);

  // set defaults

  font = NULL;
  sel_font = 0;

  size = 30;

  fgcolor.r = 0xff;
  fgcolor.g = 0xff;
  fgcolor.b = 0xff;

  bgcolor.r = 0x00;
  bgcolor.g = 0x00;
  bgcolor.b = 0x00;

  type = TEXT_LAYER;
  set_name("TTF");
  surf = NULL;
  surf_new = NULL;
  jsclass = &txt_layer_class;

  { // setup specific layer parameters
    parameters = new Linklist<Parameter>();
    Parameter *param;
    
    param = new Parameter(PARAM_NUMBER);
    strcpy(param->name, "size");
    param->description = "set the size of the font";
    parameters->append(param);
  }
}


TextLayer::~TextLayer() { 
  func("%s this=%p",__PRETTY_FUNCTION__, this);
  close();
}

bool TextLayer::open(const char *file) {
  // open a file and read its contents here?
  // skip it for now, this layer is mostly used for realtime printing
  // would be cool to implement something like justification and paragraph
  // formatting for long texts, left for later on...

  return true;
}

bool TextLayer::init(Context *freej) {
  // width/height is skipped for its functionality
  // in fact the size is changing at every new print
  // so we'll call the Layer::_init(wdt,hgt) many times
  if(freej->num_fonts<1) {
    error("no fonts found on this system");
    return false;
  }

  _init(0,0);

  if( ! TTF_WasInit() )
    TTF_Init();

  env = freej;

  return true;
}

void TextLayer::close() {
  // close up sdl ttf
  // mh, this call randomly crashes on my machine ...
  //if(font) TTF_CloseFont(font);

  if( TTF_WasInit() ) TTF_Quit();
  // free sdl font surface
  if(surf) SDL_FreeSurface(surf);
  if(surf_new) SDL_FreeSurface(surf_new);
}

void TextLayer::set_size(int nsize) {
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

void TextLayer::calculate_string_size(char *text, int *w, int *h) {
  TTF_SizeText(font, text, w, h);
}

bool TextLayer::keypress(int key) { return false; };

void TextLayer::print(const char *str) {
  SDL_Surface *tmp;
  
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

  // surf = TTF_RenderText_Blended(font, str, fgcolor);
  tmp = TTF_RenderText_Shaded(font, str, fgcolor, bgcolor);
  if(!tmp) {
  	error("Error render text: %s", SDL_GetError());
	return;
  }

  lock();
  surf_new = SDL_DisplayFormat(tmp);
  // oh no!! plz do not call _init here !!! It messes up the blitter!
  // _init(surf_new->w, surf_new->h);  
  geo_new.w = surf_new->w;
  geo_new.h = surf_new->h;
  geo_new.x = geo.x;
  geo_new.y = geo.y;
  geo_new.bpp = 32;
  geo_new.size = geo_new.w*geo_new.h*(geo_new.bpp/8);
  geo_new.pitch = geo_new.w*(geo_new.bpp/8);

  unlock();
  SDL_FreeSurface(tmp);

}

void *TextLayer::feed() {
	if(surf_new) {
	// just return the surface
		if(surf) SDL_FreeSurface(surf);
		surf = surf_new;
		surf_new = NULL;
		geo = geo_new;
	}
	if(!surf)
		return NULL;
	else
		return surf->pixels;
}

int dirent_dir_selector(const struct dirent *dir) {
	if ((dir->d_type == DT_DIR) &&
	    (strcmp(dir->d_name,".") || strcmp(dir->d_name,"..")))
		return 1;
	return 0;
}
static int ttf_dir_selector(const struct dirent *dir) {
  if(strstr(dir->d_name,".ttf")) return(1);
  if(strstr(dir->d_name,".TTF")) return(1);
  return(0);
}
int Context::scanfonts(const char *path, int depth) {
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
    func("%u fonts found in %s", found, path);

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
  filelist=NULL;

  if(depth > 0){
    depth--;
    found = scandir(path,&filelist,dirent_dir_selector,alphasort);
    while(found > 0) {
      found--;
      snprintf(temp,255,"%s/%s",path,filelist[found]->d_name);
      free(filelist[found]);
      scanfonts(temp, depth);
    }
    free(filelist);
  }
  return(num_fonts - num_before);
}
/*
Program received signal SIGSEGV, Segmentation fault.
[Switching to Thread 805452288 (LWP 6938)]
0x0f020150 in FT_Done_Face () from /usr/lib/libfreetype.so.6
(gdb) bt
#0  0x0f020150 in FT_Done_Face () from /usr/lib/libfreetype.so.6
#1  0x100838f4 in TTF_CloseFont ()
#2  0x10033768 in TextLayer::close (this=0x1032eb80) at text_layer.cpp:102
#3  0x100337e8 in ~TextLayer (this=0x1032eb80) at text_layer.cpp:72
#4  0x10013fd4 in js_layer_gc (cx=0x101f7538, obj=0x10202f98) at layer_js.cpp:640
#5  0x100cd6c0 in js_FinalizeObject ()
#6  0x100b6710 in js_GC ()
#7  0x100b6a10 in js_ForceGC ()
#8  0x10088160 in JS_GC ()
#9  0x10029210 in JsParser::gc (this=0x101f2930) at jsparser.cpp:57
#10 0x1000bcb8 in Context::rem_layer (this=0x101b62e8, lay=0x103ac880) at context.cpp:485
#11 0x10022960 in Console::parser_default (this=0x10255e30, key=4) at console.cpp:1282
#12 0x100233fc in Console::getkey (this=0x10255e30) at console.cpp:772
#13 0x10023424 in Console::cafudda (this=0x10255e30) at console.cpp:778
#14 0x1000c6f8 in Context::cafudda (this=0x101b62e8, secs=1) at context.cpp:251
#15 0x10009ed0 in main (argc=4, argv=0x7fef5424) at freej.cpp:345


*/

#endif
