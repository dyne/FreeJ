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

#ifndef __AA_SCREEN_H__
#define __AA_SCREEN_H__

#include <screen.h>

#include <factory.h>

#include <aalib.h>

class AaScreen : public ViewPort {

 public:
  AaScreen();
  ~AaScreen();

  fourcc get_pixel_format() { return RGBA32; };

  void *get_surface();

  void setup_blits(Layer *);

  void blit(Layer *src);

  void *coords(int x, int y);

  void show();

  // allow to use Factory on this class
  FACTORY_ALLOWED

 protected:
  bool _init();
  
 private:
  uint32_t rgba_to_r(uint32_t c);
  uint32_t rgba_to_g(uint32_t c);
  uint32_t rgba_to_b(uint32_t c);
  uint32_t rgba_to_a(uint32_t c);

  void *screen_buffer;

  uint32_t *pscr, *play;  // generic blit buffer pointers

  /* ascii context & html formatting stuff*/
  aa_context *ascii_context;
  struct aa_renderparams *ascii_rndparms;
  struct aa_hardware_params ascii_hwparms;
  struct aa_savedata ascii_save;
};

#endif


