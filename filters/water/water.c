/* Water filter (originally implemented in FreeJ)
 * (c) Copyright 2000-2003 Denis Rojo aka jaromil <jaromil@dyne.org>
 * 
 * from an original idea of water algorithm by Federico 'Pix' Feroldi
 * code contains optimizations by Jason Hood and Scott Scriven
 * C++, animated background, 32bit colorspace and opts by jaromil

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

#include <freej.h>
#include <freej_plugin.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <time.h>

/* setup some data to identify the plugin */
static char *name = "Water";
static char *author = "jaromil"; 
static char *info = "Water surface";
static int version = 1;

/* save here screen geometry informations */
static ScreenGeometry *geo;



#define CLIP_EDGES \
  if(x - radius < 1) left -= (x-radius-1); \
  if(y - radius < 1) top  -= (y-radius-1); \
  if(x + radius > geo->w-1) right -= (x+radius-geo->w+1); \
  if(y + radius > geo->h-1) bottom-= (y+radius-geo->h+1);

/* water physics */
#define WATER 1
#define JELLY 2
#define SLUDGE 3
#define SUPER_SLUDGE 4

/* 2 pages of Height field */
uint32_t *Height[2];
/* 3 copies of the background */
uint32_t *BkGdImagePre;
uint32_t *BkGdImage;
uint32_t *BkGdImagePost;

uint32_t *buffer;

void *surface;

/* water effect variables */
int Hpage;
int xang, yang;
int swirlangle;
int x, y, ox, oy;
int done;
int mode;

/* precalculated to optimize a bit */
int water_surfacesize;
int calc_optimization;

/* density: water density (step 1)
   pheight: splash height (step 40)
   radius: waterdrop radius (step 1) */
int density, pheight, radius;
int offset;  

int rain;
int raincount;
int surfer;
int swirl;
int physics;
int horizline;
int horizcount;
int blend;

void water_clear();
void water_distort();
void water_setphysics(int physics);
void water_update();
void water_drop(int x, int y);
void water_bigsplash(int x, int y);
void water_surfer();
void water_swirl();
void water_3swirls();
void water_horizline();

void DrawWater(int page);
void CalcWater(int npage, int density);
void CalcWaterBigFilter(int npage, int density);

void SmoothWater(int npage);

void HeightBlob(int x, int y, int radius, int height, int page);
void HeightBox (int x, int y, int radius, int height, int page);

void WarpBlob(int x, int y, int radius, int height, int page);
void SineBlob(int x, int y, int radius, int height, int page);


/* precalculated sinusoidal tables */
#include <math.h>
#define FSINMAX 2047
#define SINFIX 16
#define FSINBITS 16
#ifndef PI
#define PI 3.14159265358979323846
#endif
int FSinTab[2048], FCosTab[2048];
int FSin(int angle) { return FSinTab[angle&FSINMAX]; }
int FCos(int angle) { return FCosTab[angle&FSINMAX]; }
void FCreateSines() {
  int i; double angle;  
  for(i=0; i<2048; i++) {
    angle = (float)i * (PI/1024.0);
    FSinTab[i] = (int)(sin(angle) * (float)0x10000);
    FCosTab[i] = (int)(cos(angle) * (float)0x10000);
  }
}

/* cheap & fast randomizer (by Fukuchi Kentarou) */
uint32_t randval;
uint32_t fastrand() { return (randval=randval*1103515245+12345); };
void fastsrand(uint32_t seed) { randval = seed; };

/* integer optimized square root
   i learned this on books // by jaromil */
static int isqrt(unsigned int x) {
  unsigned int m, y, b; m = 0x40000000;
  y = 0; while(m != 0) { b = y | m; y = y>>1;
  if(x>=b) { x=x-b; y=y|m; }
  m=m>>2; } return y;
}


int init(ScreenGeometry *sg) {
  geo = sg;

  Hpage = 0;
  ox = 80;
  oy = 80;
  done = 0;
  mode = 0x4000;
  
  /* default physics */
  density = 4;
  pheight = 600;
  radius = 30;

  rain = 0;
  raincount = 0;
  surfer = 0;
  swirl = 0;
  physics = 0;
  blend = 0;
  horizline = 0;
  horizcount = 0;

  fastsrand(time(NULL));

  FCreateSines();

  water_surfacesize = geo->size;
  calc_optimization = (geo->h-1)*geo->w;
  
  xang = fastrand()%2048;
  yang = fastrand()%2048;
  swirlangle = fastrand()%2048;

  /* buffer allocation tango */
  Height[0] = (uint32_t*)malloc(geo->size);
  Height[1] = (uint32_t*)malloc(geo->size);
  memset(Height[0], 0, geo->size);
  memset(Height[1], 0, geo->size);
  buffer = malloc(geo->size);
  BkGdImagePre = malloc(geo->size);
  BkGdImage = malloc(geo->size);
  BkGdImagePost = malloc(geo->size);

  return(1);
}

int clean() {
  free(Height[0]);
  free(Height[1]);
  free(BkGdImagePre);
  free(BkGdImage);
  free(BkGdImagePost);
  free(buffer);
  return(1);
}

void *process(void *buffo) {
  memcpy(BkGdImage,     buffo, geo->size);

  water_update();
  return buffer;
}


int kbd_input(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_e: /* bigsplash in center */
    water_bigsplash(geo->w>>1,geo->y>>1);
    break;
  case SDLK_r: /* random splash */
    water_bigsplash(fastrand()%geo->w,fastrand()%geo->h);
    break;
  case SDLK_t: /* rain */
    rain = (rain)?0:1;
    break;
  case SDLK_d: /* distort surface */
    if(!rain) water_distort();
    break;
  case SDLK_f: /* smooth surface */
    SmoothWater(Hpage);
    break;
  case SDLK_y: /* swirl */
    swirl = (swirl)?0:1;
    break;
  case SDLK_u: /* surfer */
    surfer = (surfer)?0:1;
    break;
  case SDLK_g: /* randomize swirl angles */
    swirlangle = fastrand()%2048;
    xang = fastrand()%2048;
    yang = fastrand()%2048;
    break;
  case SDLK_h: /* horizontal line */
    horizline = 1;
    break;
    
  case SDLK_q:
    if(physics>1) physics--;
    water_setphysics(physics);
    break;
  case SDLK_w:
    if(physics<4) physics++;
    water_setphysics(physics);

  default:
    res = 0;
    break;
  }
  return(res);
}

void water_clear() {
  memset(Height[0], 0, water_surfacesize);
  memset(Height[1], 0, water_surfacesize);
}

void water_distort() {
  memset(Height[Hpage], 0, water_surfacesize);
}

void water_setphysics(int physics) {
  switch(physics) {
  case WATER:
    mode |= 0x4000;
    density=4;
    pheight=600;
    break;
  case JELLY:
    mode &= 0xBFFF;
    density=3;
    pheight=400;
    break;
  case SLUDGE:
    mode &= 0xBFFF;
    density=6;
    pheight=400;
    break;
  case SUPER_SLUDGE:
    mode &=0xBFFF;
    density=8;
    pheight=400;
    break;
  }
}

void water_update() {

  if(rain) {
    raincount++;
    if(raincount>3) {
      water_drop( (fastrand()%geo->w-40)+20 , (fastrand()%geo->h-40)+20 );
      raincount=0;
    }
  }

  if(swirl) water_swirl();
  if(surfer) water_surfer();
  if(horizline) water_horizline();
  DrawWater(Hpage);

  CalcWater(Hpage^1, density);
  Hpage ^=1 ;
}

void water_drop(int x, int y) {
  if(mode & 0x4000)
    HeightBlob(x,y, radius>>2, pheight, Hpage);
  else
    WarpBlob(x, y, radius, pheight, Hpage);
}

void water_bigsplash(int x, int y) {
  if(mode & 0x4000)
    HeightBlob(x, y, (radius>>1), pheight, Hpage);
  else
    SineBlob(x, y, radius, -pheight*6, Hpage);
}

void water_surfer() {
  x = (geo->w>>1)
    + ((
	(
	 (FSin( (xang* 65) >>8) >>8) *
	 (FSin( (xang*349) >>8) >>8)
	 ) * ((geo->w-8)>>1)
	) >> 16);
  y = (geo->h>>1)
    + ((
	(
	 (FSin( (yang*377) >>8) >>8) *
	 (FSin( (yang* 84) >>8) >>8)
	 ) * ((geo->h-8)>>1)
	) >> 16);
  xang += 13;
  yang += 12;
  
  if(mode & 0x4000)
    {
      offset = (oy+y)/2*geo->w + ((ox+x)>>1); // QUAAA
      Height[Hpage][offset] = pheight;
      Height[Hpage][offset + 1] =
	Height[Hpage][offset - 1] =
	Height[Hpage][offset + geo->w] =
	Height[Hpage][offset - geo->w] = pheight >> 1;
      
      offset = y*geo->w + x;
      Height[Hpage][offset] = pheight<<1;
      Height[Hpage][offset + 1] =
	Height[Hpage][offset - 1] =
	Height[Hpage][offset + geo->w] =
	Height[Hpage][offset - geo->w] = pheight;
    }
  else
    {
      SineBlob(((ox+x)>>1), ((oy+y)>>1), 3, -1200, Hpage);
      SineBlob(x, y, 4, -2000, Hpage);
    }
  
  ox = x;
  oy = y; 
}

void water_swirl() {
  x = (geo->w>>1)
    + ((
	(FCos(swirlangle)) * (25)
	) >> 16);
  y = (geo->h>>1)
    + ((
	(FSin(swirlangle)) * (25)
	) >> 16);
  swirlangle += 50;
  if(mode & 0x4000)
    HeightBlob(x,y, radius>>2, pheight, Hpage);
  else
    WarpBlob(x, y, radius, pheight, Hpage);
}

void water_horizline() {
  if(horizline > geo->w-60) {
    horizline = 0;
    horizcount = 0;
    return;
  }
  horizcount++;
  if(horizcount>8) {
    horizcount=0;
    horizline+=10;
    water_drop(horizline,geo->h>>1);
  }
}

void water_3swirls() {
#define ANGLE 15
  x = (95)
    + ((
	(FCos(swirlangle)) * (ANGLE)
	) >> 16);
  y = (45)
    + ((
	(FSin(swirlangle)) * (ANGLE)
	) >> 16);

  if(mode & 0x4000) HeightBlob(x,y, radius>>2, pheight, Hpage);
  else WarpBlob(x, y, radius, pheight, Hpage);
  
  x = (95)
    + ((
	(FCos(swirlangle)) * (ANGLE)
	) >> 16);
  y = (255)
    + ((
	(FSin(swirlangle)) * (ANGLE)
	) >> 16);
  
  if(mode & 0x4000) HeightBlob(x,y, radius>>2, pheight, Hpage);
  else WarpBlob(x, y, radius, pheight, Hpage);
  
  x = (345)
    + ((
	(FCos(swirlangle)) * (ANGLE)
	) >> 16);
  y = (150)
    + ((
	(FSin(swirlangle)) * (ANGLE)
	) >> 16);
 
 if(mode & 0x4000) HeightBlob(x,y, radius>>2, pheight, Hpage);
  else WarpBlob(x, y, radius, pheight, Hpage);


  swirlangle += 50;
}

/* internal physics routines */
void DrawWater(int page) {
  int dx, dy;
  int x, y;
  int c;
  int offset=geo->w + 1;
  int *ptr = &Height[page][0];
  
  for (y = calc_optimization; offset < y; offset += 2) {
    for (x = offset+geo->w-2; offset < x; offset++) {
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+geo->w];
      c = BkGdImage[offset + geo->w*(dy>>3) + (dx>>3)];
      
      buffer[offset] = c;

      offset++;
      dx = ptr[offset] - ptr[offset+1];
      dy = ptr[offset] - ptr[offset+geo->w];
      c = BkGdImage[offset + geo->w*(dy>>3) + (dx>>3)];

      buffer[offset] = c;      
    }
  }
}

void CalcWater(int npage, int density) {
  int newh;
  int count = geo->w + 1;
  int *newptr = &Height[npage][0];
  int *oldptr = &Height[npage^1][0];
  int x, y;

  for (y = calc_optimization; count < y; count += 2) {
    for (x = count+geo->w-2; count < x; count++) {
      /* eight pixels */
      newh = ((oldptr[count + geo->w]
	       + oldptr[count - geo->w]
	       + oldptr[count + 1]
	       + oldptr[count - 1]
	       + oldptr[count - geo->w - 1]
	       + oldptr[count - geo->w + 1]
	       + oldptr[count + geo->w - 1]
	       + oldptr[count + geo->w + 1]
	       ) >> 2 )
	- newptr[count];
      newptr[count] =  newh - (newh >> density);
    }
  }
}

void SmoothWater(int npage) {
  int newh;
  int count = geo->w + 1;
  int *newptr = &Height[npage][0];
  int *oldptr = &Height[npage^1][0];
  int x, y;

  for(y=1; y<geo->h-1; y++) {
    for(x=1; x<geo->w-1; x++) {
      /* eight pixel */
      newh          = ((oldptr[count + geo->w]
			+ oldptr[count - geo->w]
			+ oldptr[count + 1]
			+ oldptr[count - 1]
			+ oldptr[count - geo->w - 1]
			+ oldptr[count - geo->w + 1]
			+ oldptr[count + geo->w - 1]
			+ oldptr[count + geo->w + 1]
			) >> 3 )
	+ newptr[count];
      
      
      newptr[count] =  newh>>1;
      count++;
    }
    count += 2;
  }
}

void CalcWaterBigFilter(int npage, int density) {
  int newh;
  int count = (geo->w<<1) + 2;
  int *newptr = &Height[npage][0];
  int *oldptr = &Height[npage^1][0];
  int x, y;
  
  for(y=2; y<geo->h-2; y++) {
    for(x=2; x<geo->w-2; x++) {
      /* 25 pixels */
      newh = (
	      (
	       (
		(oldptr[count + geo->w]
		 + oldptr[count - geo->w]
		 + oldptr[count + 1]
		 + oldptr[count - 1]
		 )<<1)
	       + ((oldptr[count - geo->w - 1]
		   + oldptr[count - geo->w + 1]
		   + oldptr[count + geo->w - 1]
		   + oldptr[count + geo->w + 1]))
	       + ( (
		    oldptr[count - (geo->w<<1)]
		    + oldptr[count + (geo->w<<1)]
		    + oldptr[count - 2]
		    + oldptr[count + 2]
		    ) >> 1 )
	       + ( (
		    oldptr[count - (geo->w<<1) - 1]
		    + oldptr[count - (geo->w<<1) + 1]
		    + oldptr[count + (geo->w<<1) - 1]
		    + oldptr[count + (geo->w<<1) + 1]
		    + oldptr[count - 2 - geo->w]
		    + oldptr[count - 2 + geo->w]
		    + oldptr[count + 2 - geo->w]
		    + oldptr[count + 2 + geo->w]
		    ) >> 2 )
	       )
	      >> 3)
	- (newptr[count]);
      newptr[count] =  newh - (newh >> density);
      count++;
    }
    count += 4;
  }
}

void HeightBlob(int x, int y, int radius, int height, int page) {
  int rquad;
  int cx, cy, cyq;
  int left, top, right, bottom;

  rquad = radius * radius;

  /* Make a randomly-placed blob... */
  if(x<0) x = 1+radius+ fastrand()%(geo->w-2*radius-1);
  if(y<0) y = 1+radius+ fastrand()%(geo->h-2*radius-1);

  left=-radius; right = radius;
  top=-radius; bottom = radius;

  CLIP_EDGES

  for(cy = top; cy < bottom; cy++) {
    cyq = cy*cy;
    for(cx = left; cx < right; cx++) {
      if(cx*cx + cyq < rquad)
        Height[page][geo->w*(cy+y) + (cx+x)] += height;
    }
  }
}


void HeightBox (int x, int y, int radius, int height, int page) {
  int cx, cy;
  int left, top, right, bottom;

  if(x<0) x = 1+radius+ fastrand()%(geo->w-2*radius-1);
  if(y<0) y = 1+radius+ fastrand()%(geo->h-2*radius-1);
  
  left=-radius; right = radius;
  top=-radius; bottom = radius;
  
  CLIP_EDGES
  
  for(cy = top; cy < bottom; cy++) {
    for(cx = left; cx < right; cx++) {
      Height[page][geo->w*(cy+y) + (cx+x)] = height;
    }
  } 
}

void WarpBlob(int x, int y, int radius, int height, int page) {
  int cx, cy;
  int left,top,right,bottom;
  int square;
  int radsquare = radius * radius;
  
  radsquare = (radius*radius);
  
  height = height>>5;
  
  left=-radius; right = radius;
  top=-radius; bottom = radius;

  CLIP_EDGES
  
  for(cy = top; cy < bottom; cy++) {
    for(cx = left; cx < right; cx++) {
      square = cy*cy + cx*cx;
      if(square < radsquare) {
	Height[page][geo->w*(cy+y) + cx+x]
          += (int)((radius-isqrt(square))*(float)(height));
      }
    }
  }
}

void SineBlob(int x, int y, int radius, int height, int page) {
  int cx, cy;
  int left,top,right,bottom;
  int square, dist;
  int radsquare = radius * radius;
  float length = (1024.0/(float)radius)*(1024.0/(float)radius);
  
  if(x<0) x = 1+radius+ fastrand()%(geo->w-2*radius-1);
  if(y<0) y = 1+radius+ fastrand()%(geo->h-2*radius-1);

  radsquare = (radius*radius);
  left=-radius; right = radius;
  top=-radius; bottom = radius;

  CLIP_EDGES

  for(cy = top; cy < bottom; cy++) {
    for(cx = left; cx < right; cx++) {
      square = cy*cy + cx*cx;
      if(square < radsquare) {
        dist = (int)(isqrt(square*length));
        Height[page][geo->w*(cy+y) + cx+x]
          += (int)((FCos(dist)+0xffff)*(height)) >> 19;
      }
    }
  }
}

