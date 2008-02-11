/* 
 *
 * VertigoTV - Alpha blending with zoomed and rotated images.
 * Copyright (C) 2001 FUKUCHI Kentarou
 * porting and some small modifications done by jaromil
 *
 */
/* ported to frei0r by joepadmiraal */

#include "frei0r.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <string.h>


typedef struct vertigo_instance
{
  unsigned int width;
  unsigned int height;
  int x,y,xc,yc;
  int mode;
  double phase_increment;
  double zoomrate;
  double tfactor;
  uint32_t *current_buffer, *alt_buffer;
} vertigo_instance_t;

static uint32_t *buffer;
static int dx, dy;
static int sx, sy;

static int pixels;
static double phase;


int f0r_init()
{
  return 1;
}

void f0r_deinit()
{ 
  /* no deinitialization required */ 
}

void f0r_get_plugin_info(f0r_plugin_info_t* vertigoInfo)
{
  vertigoInfo->name = "Vertigo";
  vertigoInfo->author = "Fukuchi Kentarou";
  vertigoInfo->plugin_type = F0R_PLUGIN_TYPE_FILTER;
  vertigoInfo->color_model = F0R_COLOR_MODEL_RGBA8888;
  vertigoInfo->frei0r_version = FREI0R_MAJOR_VERSION;
  vertigoInfo->major_version = 1;
  vertigoInfo->minor_version = 0;
  vertigoInfo->num_params =  3;
  vertigoInfo->explanation = "alpha blending with zoomed and rotated images";
}

void f0r_get_param_info(f0r_param_info_t* info, int param_index)
{
  switch(param_index)
  {
  case 0:
    info->name = "Mode";
    info->type = F0R_PARAM_DOUBLE;
    info->explanation = "The effect mode";
    break;
  case 1:
    info->name = "PhaseIncrement";
    info->type = F0R_PARAM_DOUBLE;
    info->explanation = "Phase increment";
    break;
  case 2:
    info->name = "Zoomrate";
    info->type = F0R_PARAM_DOUBLE;
    info->explanation = "Zoomrate";
    break;
  }
}

f0r_instance_t f0r_construct(unsigned int width, unsigned int height)
{
  vertigo_instance_t* inst = 
    (vertigo_instance_t*)malloc(sizeof(vertigo_instance_t));
  inst->width = width; inst->height = height;

  pixels = width*height;

  buffer = malloc(sizeof(uint32_t *)*pixels*2);
  if(buffer == NULL) return 0;
  memset(buffer,0,pixels * 2);

  inst->current_buffer = buffer;
  inst->alt_buffer = buffer + pixels;

  phase = 0.0;
  inst->mode = 3;
  inst->phase_increment = 0.02;
  inst->zoomrate = 1.01;

  inst->x = width>>1;
  inst->y = height>>1;
  inst->xc = inst->x*inst->x;
  inst->yc = inst->y*inst->y;
  inst->tfactor = (inst->xc+inst->yc) * inst->zoomrate;

  return (f0r_instance_t)inst;
}

void f0r_destruct(f0r_instance_t instance)
{
  if(buffer) 
  {
    free(buffer); 
    buffer = NULL; 
  }
  free(instance);
}

void f0r_set_param_value(f0r_instance_t instance, 
			 f0r_param_t param, int param_index)
{ 
  assert(instance);
  vertigo_instance_t* inst = (vertigo_instance_t*)instance;

  switch(param_index)
  {
  case 0:
    /* mode */
    inst->mode = *((double*)param);
    break;
  case 1:
    /* phase_increment */
    inst->phase_increment = *((double*)param);
    break;
  case 2:
    /* zoomrate */
    inst->zoomrate = *((double*)param);
    inst->tfactor = (inst->xc+inst->yc) * inst->zoomrate;
    break;
  }
}

void f0r_get_param_value(f0r_instance_t instance,
			 f0r_param_t param, int param_index)
{ 
  assert(instance);
  vertigo_instance_t* inst = (vertigo_instance_t*)instance;
  
  switch(param_index)
  {
  case 0:
    /* mode */
    *((double*)param) = (double) (inst->mode);
    break;
  case 1:
    /* phase_increment */
    *((double*)param) = (double) (inst->phase_increment);
    break;
  case 2:
    /* zoomrate */
    *((double*)param) = (double) (inst->zoomrate);
    break;
  }
}

void f0r_update(f0r_instance_t instance, double time,
		const uint32_t* inframe, uint32_t* outframe)
{
  vertigo_instance_t* inst = (vertigo_instance_t*)instance;
  unsigned int w = inst->width;
  unsigned int h = inst->height;
  int x = inst->x;
  int y = inst->y;
  int xc = inst->xc;
  int yc = inst->yc;
  double tfactor = inst->tfactor;

  uint32_t* dst = outframe;
  const uint32_t* src = inframe;
  uint32_t *p;
  uint32_t v;
  int ox, oy;
  int i;

  double vx, vy;
  double dizz;
  
  dizz = sin(phase) * 10 + sin(phase*1.9+5) * 5;

  if(w > h) 
  {
    if(dizz >= 0) 
    {
      if(dizz > x) dizz = x;
      vx = (x*(x-dizz) + yc) / tfactor;
    } else 
    {
      if(dizz < -x) dizz = -x;
      vx = (x*(x+dizz) + yc) / tfactor;
    }
    vy = (dizz*y) / tfactor;

  } else 
  {
    if(dizz >= 0) 
    {
      if(dizz > y) dizz = y;
      vx = (xc + y*(y-dizz)) / tfactor;
    } else 
    {
      if(dizz < -y) dizz = -y;
      vx = (xc + y*(y+dizz)) / tfactor;
    }
    vy = (dizz*x) / tfactor;
  }

  dx = vx * 65536;
  dy = vy * 65536;
  sx = (-vx * x + vy * y + x + cos(phase*5) * 2) * 65536;
  sy = (-vx * y - vy * x + y + sin(phase*6) * 2) * 65536;
  
  phase += inst->phase_increment;
  if(phase > 5700000) phase = 0;


  p = inst->alt_buffer;

  for(y=h; y>0; y--) 
  {
    ox = sx;
    oy = sy;
    for(x=w; x>0; x--) 
    {
      i = (oy>>16)*w + (ox>>16);
      if(i<0) i = 0;
      if(i>=pixels) i = pixels;
      v = inst->current_buffer[i] & 0xfcfcff;
      v = (v * inst->mode) + ((*src++) & 0xfcfcff);
      *dst++ = (v>>2);
      *p++ = (v>>2);
      ox += dx;
      oy += dy;
    }
    sx -= dy;
    sy += dx;
  }

  p = inst->current_buffer;
  inst->current_buffer = inst->alt_buffer;
  inst->alt_buffer = p;

}
