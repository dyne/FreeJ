/*
 * EdgeTV - detects edge and display it in good old computer way. 
 * Copyright (C) 2001 FUKUCHI Kentarou
 *
 * The idea of EdgeTV is taken from Adrian Likins's effector script for GIMP,
 * `Predator effect.'
 *
 * The algorithm of the original script pixelizes the image at first, then
 * it adopts the edge detection filter to the image. It also adopts MaxRGB
 * filter to the image. This is not used in EdgeTV.
 * This code is highly optimized and employs many fake algorithms. For example,
 * it devides a value with 16 instead of using sqrt() in line 142-144. It is
 * too hard for me to write detailed comment in this code in English.
 */

#include <freej.h>
#include <freej_plugin.h>

#include <SDL/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

static char *name = "Edge";
static char *author = "Fukuchi Kentarou";
static char *info = "edge detection";
static int version = 1;

static ScreenGeometry *geo;

static void *procbuf;

static int32_t *map;
static int map_width;
static int map_height;

static int w2,w3,w4,mw2;

static int video_width_margin;

int init(ScreenGeometry *sg) {
  geo = sg;

  procbuf = malloc(geo->size);

  map_width = geo->w / 4;
  map_height = geo->h / 4;
  video_width_margin = geo->w - map_width * 4;

  /* some precalculation to save cycles to the inner loop
     (optimization by jaromil) */
  w2 = geo->w*2;
  w3 = geo->w*3; 
  w4 = geo->w*4;
  mw2 = map_width*2;

  map = (int32_t *)malloc(map_width*map_height*sizeof(int32_t)*2);
  if(!map) return 0;
  
  return 1;
}

int clean() {
  if(procbuf) {
    free(procbuf); procbuf = NULL; }
  if(map) {
    free(map); map = NULL; }
  return 1;
}

void *process(void *buffo) {
  int x, y;
  int r, g, b;
  int32_t p, q;
  int32_t v0, v1, v2, v3;
  int32_t *src, *dest;
  src = (int32_t*)buffo;
  dest = (int32_t*)procbuf;

  src += w4+4;
  dest += w4+4;
  for(y=1; y<map_height-1; y++) {
    for(x=1; x<map_width-1; x++) {
      p = *src;
      q = *(src - 4);
      
      /* difference between the current pixel and right neighbor. */
      r = ((p&0xff0000) - (q&0xff0000))>>16;
      g = ((p&0xff00) - (q&0xff00))>>8;
      b = (p&0xff) - (q&0xff);
      r *= r;
      g *= g;
      b *= b;
      r = r>>5; /* To lack the lower bit for saturated addition,  */
      g = g>>5; /* devide the value with 32, instead of 16. It is */
      b = b>>4; /* same as `v2 &= 0xfefeff' */
      if(r>127) r = 127;
      if(g>127) g = 127;
      if(b>255) b = 255;
      v2 = (r<<17)|(g<<9)|b;
      
      /* difference between the current pixel and upper neighbor. */
      q = *(src - w4);
      r = ((p&0xff0000) - (q&0xff0000))>>16;
      g = ((p&0xff00) - (q&0xff00))>>8;
      b = (p&0xff) - (q&0xff);
      r *= r;
      g *= g;
      b *= b;
      r = r>>5;
      g = g>>5;
      b = b>>4;
      if(r>127) r = 127;
      if(g>127) g = 127;
      if(b>255) b = 255;
      v3 = (r<<17)|(g<<9)|b;
      
      v0 = map[(y-1)*mw2+(x<<1)];
      v1 = map[y*mw2+((x-1)<<1)+1];
      map[y*mw2+(x<<1)] = v2;
      map[y*mw2+(x<<1)+1] = v3;
      r = v0 + v1;
      g = r & 0x01010100;
      dest[0] = r | (g - (g>>8));
      r = v0 + v3;
      g = r & 0x01010100;
      dest[1] = r | (g - (g>>8));
      dest[2] = v3;
      dest[3] = v3;
      r = v2 + v1;
      g = r & 0x01010100;
      dest[geo->w] = r | (g - (g>>8));
      r = v2 + v3;
      g = r & 0x01010100;
      dest[geo->w+1] = r | (g - (g>>8));
      dest[geo->w+2] = v3;
      dest[geo->w+3] = v3;
      dest[w2] = v2;
      dest[w2+1] = v2;
      dest[w3] = v2;
      dest[w3+1] = v2;
      
      src += 4;
      dest += 4;
    }
    src += w3+8+video_width_margin;
    dest += w3+8+video_width_margin;
  }
  return procbuf;
}

int kbd_input(SDL_keysym *keysym) {
  return 0;
}
