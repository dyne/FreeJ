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
#if defined WITH_TEXTLAYER

#include <fontconfig/fontconfig.h>

#include <context.h>
#include <text_layer.h>
#include <jutils.h>
#include <jsparser_data.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, TextLayer, TextLayer, truetype);

TextLayer::TextLayer()
  :Layer() {
  func("%s this=%p",__PRETTY_FUNCTION__, this);

  // set defaults

  font = NULL;
  fontfile = NULL;
  fontname = NULL;

  size = 30;

  fgcolor.r = 0xff;
  fgcolor.g = 0xff;
  fgcolor.b = 0xff;

  bgcolor.r = 0x00;
  bgcolor.g = 0x00;
  bgcolor.b = 0x00;

  type = Layer::TEXT;
  set_name("TXT");
  jsclass = &txt_layer_class;

  { // setup specific layer parameters
    parameters = new Linklist<Parameter>();
    Parameter *param;
    
    param = new Parameter(Parameter::NUMBER);
    strcpy(param->name, "size");
    strcpy(param->description, "set the size of the font");
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

bool TextLayer::_init() {
  // width/height is skipped for its functionality
  // in fact the size is changing at every new print
  // so we'll call the Layer::_init(wdt,hgt) many times

  if( ! TTF_WasInit() )
    TTF_Init();

  set_font("sans"); // just try one..

  opened = true;

  return true;
}

void TextLayer::close() {
  stop();
  deferred_calls->cancel_jobs();
  std::lock_guard<Layer> lock(*this);
  clear_frame();
  surf.reset();
  if (font)
    TTF_CloseFont(font);
  font = nullptr;
  free(fontfile);
  free(fontname);
  fontfile = fontname = nullptr;
  opened = false;
}

void TextLayer::calculate_string_size(char *text, int *w, int *h) {
  TTF_SizeText(font, text, w, h);
}

void TextLayer::set_fgcolor(int r, int g, int b) {
  fgcolor.r = r;
  fgcolor.g = g;
  fgcolor.b = b;
}

void TextLayer::set_bgcolor(int r, int g, int b) {
  bgcolor.r = r;
  bgcolor.g = g;
  bgcolor.b = b;
}

char *TextLayer::_get_fontfile(const char *name) {
  char *path = NULL;
  FcPattern *pat, *match;
  FcFontSet *fs;
  FcResult result;
  FcChar8 *file;

  if (!FcInit ()) { // if already initialized returns immediately
    error("Can't init font config library\n");
    return NULL;
  }

  pat = FcNameParse ((FcChar8 *)name);

  FcConfigSubstitute (0, pat, FcMatchPattern);
  FcDefaultSubstitute (pat);

  fs = FcFontSetCreate ();

  match = FcFontMatch (0, pat, &result);
  if (match) FcFontSetAdd (fs, match);

  FcPatternDestroy (pat);

  if (fs && fs->nfont > 0)
    if (FcPatternGetString(fs->fonts[0], FC_FILE, 0, &file) == FcResultMatch)
      path = strdup((const char *)file);

  FcFontSetDestroy (fs);
  
  return path;

}

bool TextLayer::set_font(const char *name, int sz) {
  char *path = _get_fontfile(name);
  if(!path) {
    error("Couldn't find a suitable font for name '%s'\n", name);
    return false;
  }
  // check font
  TTF_Font *tmpfont = TTF_OpenFont(path, sz);
  if(!tmpfont) {
    error("Couldn't load font file %s with size %d: %s\n", path, sz, SDL_GetError());
    free(path);
    return false;
  }
  // update object state
  if(fontname) free(fontname);
  fontname = strdup(name);
  if(fontfile) free(fontfile);
  fontfile = path;
  if(font) TTF_CloseFont(font);
  font = tmpfont;
  // TODO(shammash): the user should be able to set a style
  TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
  // here can be also: TTF_STYLE_BOLD _ITALIC _UNDERLINE
  size = sz;
  return true;
}

bool TextLayer::set_font(const char *name) {
  return set_font(name, size);
}

bool TextLayer::set_fontsize(int sz) {
  if(!fontname) {
    error("You must specify a font before setting its size");
    return false;
  }
  char *tmpfn = strdup(fontname); // dup two times, but code should be clearer
  bool rv = set_font(tmpfn, sz);
  free(tmpfn);
  return rv;
}

void TextLayer::_display_text(std::shared_ptr<SDL_Surface> newsurf) {
  std::lock_guard<Layer> lock(*this);
  clear_frame();
  geo.init(newsurf->w, newsurf->h, 32);
  need_crop = true;
  surf = std::move(newsurf);
}

void TextLayer::write(const char *str) {
  // choose first font and initialize ready for printing
  
  if(!font) {
    error("no font selected on text layer %s, please choose one!", this->name);
    return;
  }

  // surf = TTF_RenderText_Blended(font, str, fgcolor);
  std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> tmp(
      TTF_RenderText_Shaded(font, str, fgcolor, bgcolor), SDL_FreeSurface);
  if(!tmp) {
  	error("Error render text: %s", SDL_GetError());
	return;
  }
  std::shared_ptr<SDL_Surface> converted(SDL_DisplayFormat(tmp.get()), SDL_FreeSurface);
  if (converted)
    deferred_calls->add_job(NewClosure(this, &TextLayer::_display_text, converted));

}

void *TextLayer::feed() {
	if(!surf)
		return NULL;
	else
		return surf->pixels;
}

#endif
