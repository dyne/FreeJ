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

#ifndef __LAYER_H__
#define __LAYER_H__

#include <inttypes.h>
#include <SDL.h>

#include <filter.h>
#include <jsync.h>

class Context;

class Layer: public Entry, public JSyncThread {
 private:
  char _name[5];
  char filename[256];

  int _blit_algo;
  void blit(void *offset);
  int blit_x, blit_y;
  int blit_width, blit_height;
  int blit_offset;
  /* small vars used in blits */
  int chan, c, cc;
  uint32_t *scr, *pscr, *off, *poff;
  Uint8 *alpha;

  
  void *offset;
  void *bgmatte;

 public:

  Layer();
  ~Layer();

  void run();
  void _init(Context *screen, int wdt, int hgt, int bpp=0);
  void set_filename(char *f);
  char *get_filename() { return filename; };
  void set_position(int x, int y);

  /* these must be defined in layer implementations */
  virtual bool open(char *file) =0;
  virtual bool init(Context *scr) =0;
  virtual void close() =0;

  /* these has to be defined into layer instances
     (pure virtual functions) */
  virtual void *feed() = 0; /* feeds in the image source */
  void *get_buffer() { return buffer; }; /* returns a pointer to the image source */

  void set_blit(int b);
  char *get_blit();
  bool alpha_blit;

  void crop();

  virtual bool keypress(SDL_keysym *keysym) { return(false); };

  void setname(char *s);
  char *getname();
  				   
  bool cafudda();

  Linklist filters;

  Context *screen;

  ScreenGeometry geo;

  bool active;
  bool quit;
  bool running;
  bool hidden;
  int bgcolor;

 protected:
  void *buffer;
};

/* function for type detection of implemented layers */
Layer *create_layer(char *file);
extern const char *layers_description;

#endif
