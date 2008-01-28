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

/* 07-14-02 Major speed optimizations done by WP (it's really fast now :-)).
   Uses an 8 bit indexed image
   note that the code has not been fully optimized */

/* ported to freej by jaromil */
/* ported to frei0r by joepadmiraal */

#include "frei0r.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

typedef struct rotozoom_instance
{
  unsigned int width;
  unsigned int height;
} rotozoom_instance_t;

static int *roto;
static int *roto2;

void draw_tile(int stepx, int stepy, int zoom,uint32_t *texture,uint32_t *image, int w, int h)
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

  for (j = 0; j < h; j++)
  {
    x = sx; y = sy;   
    for (i = 0; i < w; i++) 
    {
      a = x >> 12 &255;
      b = y >> 12 &255;
      a=a*w>>8; // /255
      b=b*h>>8; // /255
      *image++ = texture[b*w + a];
      x += xd; y += yd;
    }
    sx -= yd; sy += xd;
  }
}

int f0r_init()
{
  return 1;
}

void f0r_deinit()
{ 
  /* no deinitialization required */ 
}

void f0r_get_plugin_info(f0r_plugin_info_t* rotozoomInfo)
{
  rotozoomInfo->name = "rotozoom";
  rotozoomInfo->author = "van Paassen, Ellacott";
  rotozoomInfo->plugin_type = F0R_PLUGIN_TYPE_FILTER;
  rotozoomInfo->color_model = F0R_COLOR_MODEL_RGBA8888;
  rotozoomInfo->frei0r_version = FREI0R_MAJOR_VERSION;
  rotozoomInfo->major_version = 1; 
  rotozoomInfo->minor_version = 0; 
  rotozoomInfo->num_params =  0; 
  rotozoomInfo->explanation = "planar z-axis zoomed rotation";
}

void f0r_get_param_info(f0r_param_info_t* info, int param_index)
{
  /* no params */
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
  rotozoom_instance_t* inst = 
    (rotozoom_instance_t*)malloc(sizeof(rotozoom_instance_t));
  inst->width = width; inst->height = height;

  int i;
  roto = malloc(sizeof(uint32_t *)*width*height);
  roto2 = malloc(sizeof(uint32_t *)*width*height);

  for (i = 0; i < 256; i++) 
  {
    float rad =  (float)i * 1.41176 * 0.0174532;
    float c = sin(rad);
    roto[i] = (c + 0.8) * 4096.0;
    roto2[i] = (2.0 * c) * 4096.0;
  }
  return (f0r_instance_t)inst;
}

void f0r_destruct(f0r_instance_t instance)
{
  free(roto);
  free(roto2);
  free(instance);
}

void f0r_set_param_value(f0r_instance_t instance, 
			 f0r_param_t param, int param_index)
{ 
  /* no params */ 
}

void f0r_get_param_value(f0r_instance_t instance,
			 f0r_param_t param, int param_index)
{ 
  /* no params */
}

void f0r_update(f0r_instance_t instance, double time,
		const uint32_t* inframe, uint32_t* outframe)
{
  static int path=0;
  static int zpath=0;
  assert(instance);
  rotozoom_instance_t* inst = (rotozoom_instance_t*)instance;
  draw_tile(roto[path], roto[(path + 128) & 0xFF], roto2[zpath],inframe,outframe, inst->width, inst->height);
  path = (path - 1) & 255;
  zpath =(zpath + 1) & 255;
}

