/*
 *
 * VertigoTV - Alpha blending with zoomed and rotated images.
 * Copyright (C) 2001 FUKUCHI Kentarou
 * some small modifications done by jaromil
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>

#include <freej.h>
#include <freej_plugin.h>

static char *name = "Vertigo";
static char *author = "Fukuchi Kentarou";
static char *info = "alpha blending with zoomed and rotated images";
static int version = 1;
static int bpp = 4;

static Uint32 *buffer;
static Uint32 *current_buffer, *alt_buffer;
static int dx, dy;
static int sx, sy;
static int x,y,xc,yc;
static int pixels;
static double phase;
static double phase_increment;
static double zoomrate;
static double tfactor;
static int mode = 3;

static ScreenGeometry *geo;

static void setParams()
{
  double vx, vy;
  double dizz;
  
  dizz = sin(phase) * 10 + sin(phase*1.9+5) * 5;
  /*  
  if(geo->w > geo->h) {
  */
  if(dizz >= 0) {
    if(dizz > x) dizz = x;
    vx = (x*(x-dizz) + yc) / tfactor;
  } else {
    if(dizz < -x) dizz = -x;
    vx = (x*(x+dizz) + yc) / tfactor;
  }
  vy = (dizz*y) / tfactor;
  /*
  } else {
    if(dizz >= 0) {
      if(dizz > y) dizz = y;
      vx = (xc + y*(y-dizz)) / tfactor;
    } else {
      if(dizz < -y) dizz = -y;
      vx = (xc + y*(y+dizz)) / tfactor;
    }
    vy = (dizz*x) / tfactor;
  }
  */
  dx = vx * 65536;
  dy = vy * 65536;
  sx = (-vx * x + vy * y + x + cos(phase*5) * 2) * 65536;
  sy = (-vx * y - vy * x + y + sin(phase*6) * 2) * 65536;
  
  phase += phase_increment;
  if(phase > 5700000) phase = 0;
}

int init(ScreenGeometry *sg) {
  geo = sg;
  pixels = geo->w*geo->h;

  buffer = malloc(geo->size*2);
  if(buffer == NULL) return 0;
  memset(buffer,0,geo->size * 2);

  current_buffer = buffer;
  alt_buffer = buffer + pixels;

  phase = 0.0;
  phase_increment = 0.02;
  zoomrate = 1.01;

  x = geo->w>>1;
  y = geo->h>>1;
  xc = x*x;
  yc = y*y;
  tfactor = (xc+yc) * zoomrate;
  return 1;
}

int clean() {
  free(buffer);
  return 1;
}

void *process(void *buffo) {
  Uint32 *src, *dest, *p;
  Uint32 v;
  int x, y;
  int ox, oy;
  int i;

  src = buffo;

  setParams();
  p = alt_buffer;
  for(y=geo->h; y>0; y--) {
    ox = sx;
    oy = sy;
    for(x=geo->w; x>0; x--) {
      i = (oy>>16)*geo->w + (ox>>16);
      if(i<0) i = 0;
      if(i>=pixels) i = pixels;
      v = current_buffer[i] & 0xfcfcff;
      v = (v * mode) + ((*src++) & 0xfcfcff);
      *p++ = (v>>2);
      ox += dx;
      oy += dy;
    }
    sx -= dy;
    sy += dx;
  }
  
  dest = alt_buffer;

  p = current_buffer;
  current_buffer = alt_buffer;
  alt_buffer = p;
  
  return dest;
}

int kbd_input(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_BACKSPACE:
    phase = 0.0;
    phase_increment = 0.02;
    zoomrate = 1.01;
    break;
  case SDLK_w:
    phase_increment += 0.01;
    break;
  case SDLK_q:
    if(phase_increment>0.01)
      phase_increment -= 0.01;
    break;
  case SDLK_s:
    if(zoomrate<1.1) {
      zoomrate += 0.01;
      tfactor = (xc+yc) * zoomrate;
    }
    break;
  case SDLK_a:
    if(zoomrate>1.01) {
      zoomrate -= 0.01;
      tfactor = (xc+yc) * zoomrate;
    }
    break;
  case SDLK_x:
    if(mode<5) mode++;
    break;
  case SDLK_z:
    if(mode>3) mode--;
    break;
  default:
    res = 0;
    break;
  }    
  return(res);
}
