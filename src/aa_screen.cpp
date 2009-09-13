/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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

#include <config.h>
#ifdef WITH_AALIB

#include <stdlib.h>

#include <layer.h>
#include <blitter.h>

#include <jutils.h>
#include <aa_screen.h>

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(ViewPort, AaScreen, Screen, ascii);




AaScreen::AaScreen()
  : ViewPort() {

  screen_buffer = NULL;

  ascii_context = NULL;
  set_name("ASCII");
}

AaScreen::~AaScreen() {
  func("%s",__PRETTY_FUNCTION__);
  if(ascii_context)
    aa_close(ascii_context);
  if(screen_buffer) free(screen_buffer);

}

void AaScreen::setup_blits(Layer *lay) {

  Blitter *b = new Blitter();

  setup_linear_blits(b);

  lay->blitter = b;

  lay->set_blit("RGB"); // default

}

bool AaScreen::_init() {

  /* width/height image setup */
  ascii_size = geo.pixelsize / 4;

  ascii_hwparms.width  = geo.w / 2;
  ascii_hwparms.height = geo.h / 2;

  ascii_rndparms = aa_getrenderparams();
  ascii_rndparms->bright = 60;
  ascii_rndparms->contrast = 4;
  ascii_rndparms->gamma = 3;


  ascii_context = aa_autoinit (&ascii_hwparms);
  if(!ascii_context) {
    error("cannot initialize ASCII screen (aa_autoinit failed)");
    return(false);
  }
  screen_buffer = malloc(geo.bytesize);

  return(true);
}

void AaScreen::blit(Layer *src) {
  int16_t c;
  Blit *b;
  
  if(src->screen != this) {
    error("%s: blit called on a layer not belonging to screen",
	  __PRETTY_FUNCTION__);
    return;
  }

  if(src->need_crop)
    src->blitter->crop( src, this );

  b = src->current_blit;

  pscr = (uint32_t*) get_surface() + b->scr_offset;
  play = (uint32_t*) src->buffer   + b->lay_offset;

  // iterates the blit on each horizontal line
  for( c = b->lay_height ; c > 0 ; c-- ) {

    (*b->fun)
      ((void*)play, (void*)pscr,
       b->lay_bytepitch,// * src->geo.bpp>>3,
       &b->parameters);

    // strides down to the next line
    pscr += b->scr_stride + b->lay_pitch;
    play += b->lay_stride + b->lay_pitch;
    
  }

}

uint32_t AaScreen::rgba_to_r(uint32_t c) { return       c & 0xff; }
uint32_t AaScreen::rgba_to_g(uint32_t c) { return (c>> 8) & 0xff; }
uint32_t AaScreen::rgba_to_b(uint32_t c) { return (c>>16) & 0xff; }
uint32_t AaScreen::rgba_to_a(uint32_t c) { return (c>>24);        }

void AaScreen::show() {
  int c;
  // make screen_buffer RGBA to grey
  uint8_t *aa_buf = (uint8_t*)aa_image(ascii_context);
  uint32_t *rgb_buf = (uint32_t*) screen_buffer;

  for(c=0;c<geo.pixelsize;c++)
    aa_buf[c] = .30*rgba_to_r(rgb_buf[c])
      + .59*rgba_to_g(rgb_buf[c])
      + .11*rgba_to_b(rgb_buf[c]);


  aa_render (ascii_context, ascii_rndparms, 0, 0, geo.w,geo.h);
  aa_flush (ascii_context);

}

void *AaScreen::coords(int x, int y) {
//   func("method coords(%i,%i) invoked", x, y);
// if you are trying to get a cropped part of the layer
// use the .pixelsize geometric property for a pre-calculated stride
// that is: number of bytes for one full line
  return
    ( x + geo.pixelsize +
      (uint32_t*)get_surface() );
}

void *AaScreen::get_surface() {
  if(!screen_buffer) {
    error("SOFT screen output is not properly initialised via set_buffer");
    error("this will likely segfault FreeJ");
    return NULL;
  }
  return screen_buffer;
}

#endif
