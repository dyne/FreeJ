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
 *
 * "$Id$"
 *
 */

#ifndef __GEN_LAYER_H__
#define __GEN_LAYER_H__

#include <SDL/SDL.h>
#include <layer.h>

#define PRIMES 11

class GenLayer: public Layer {
 private:

  /* blossom vars */
  double blossom_count;
  double blossom_m;
  double blossom_n;
  double blossom_i;
  double blossom_j;
  double blossom_k;
  double blossom_l;
  float blossom_r;
  float blossom_a;

  /* primes */
  int prime[PRIMES];
  void blossom();

  float pi2;
  double wd, hd;

  /* surface buffer */
  uint32_t rmask,gmask,bmask,amask;
  uint32_t *pixels;

  /* blob drawing */
  void blob_init(int ray);
  void blob(int x, int y);
  uint32_t *blob_buf;
  int blob_size;

 public:
  GenLayer();
  ~GenLayer();
  
  bool init(Context *screen);
  bool open(char *file);
  void *feed();
  bool keypress(char key);
  void close();

  void blossom_recal(bool r);
};

#endif
