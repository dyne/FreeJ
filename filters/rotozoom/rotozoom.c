/*
 * EffecTV - Realtime Digital Video Effector
 * Copyright (C) 2001 FUKUCHI Kentarou
 *
 * Rotozoom - Ported to effectv by Buddy Smith.
 * Original license below
 *
 * then ported to FreeJ by jaromil
 *
 */

/* Copyright (C) 2002 W.P. van Paassen - peter@paassen.tmfweb.nl, Byron Ellacott - bje@apnic.net

   This program is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* 07-14-02 Major speed optimizations done by WP (it's really fast now :-)). Uses an 8 bit indexed image*/
/* note that the code has not been fully optimized */

#include <freej.h>
#include <freej_plugin.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <SDL/SDL.h>

static char *name = "Rotozoom";
static char *author = "Paassen, Ellacott";
static char *info = "planar z-axis rotation";
static int version = 1;

static int *roto;
static int *roto2;

static ScreenGeometry *geo;

static void *procbuf;

void draw_tile(int stepx, int stepy, int zoom,int32_t *texture,int32_t *image)
{
  int x, y, i, j, xd, yd, a, b, sx, sy;
    
  sx = sy = 0;
  xd = (stepx * zoom) >> 12;
  yd = (stepy * zoom) >> 12;
    
  /* Stepping across and down the screen, each screen row has a
   * starting coordinate in the texture: (sx, sy).  As each screen
   * row is traversed, the current texture coordinate (x, y) is
   * modified by (xd, yd), which are (sin(rot), cos(rot)) multiplied
   * by the current zoom factor.  For each vertical step, (xd, yd)
   * is rotated 90 degrees, to become (-yd, xd).
   *
   * More fun can be had by playing around with x, y, xd, and yd as
   * you move about the image.
   */

  for (j = 0; j < geo->h; j++) {
    x = sx; y = sy;   
    for (i = 0; i < geo->w; i++) {
      a = x >> 12 &255;
      b = y >> 12 &255;
      a=a*geo->w/255;
      b=b*geo->h/255;
      *image++ = texture[b*geo->w + a];
      x += xd; y += yd;
    }
    sx -= yd; sy += xd;
  }
}

int init(ScreenGeometry *sg) {
  int i;
  geo = sg;
  
  roto = malloc(sizeof(int32_t *)*geo->w*geo->h);
  roto2 = malloc(sizeof(int32_t *)*geo->w*geo->h);

  procbuf = malloc(geo->size);
  
  for (i = 0; i < 256; i++) {
    float rad =  (float)i * 1.41176 * 0.0174532;
    float c = sin(rad);
    roto[i] = (c + 0.8) * 4096.0;
    roto2[i] = (2.0 * c) * 4096.0;
  }
  return 1;
}

int clean() {
  free(roto);
  free(roto2);
  return 1;
}

void *process(void *buffo) {
  static int path=0;
  static int zpath=0;
  int32_t *src = (int32_t*)buffo;
  int32_t *dst = (int32_t*)procbuf;

  draw_tile(roto[path], roto[(path + 128) & 0xFF], roto2[zpath],src,dst);
  path = (path - 1) & 255;
  zpath =(zpath + 1) & 255;

  return procbuf;
}

int kbd_input(SDL_keysym *keysym) {
  return 0;
}
