/* Cartoon filter
 * main algorithm: (c) Copyright 2003 Dries Pruimboom <dries@irssystems.nl>
 * further optimizations and freej port by Denis Rojo <jaromil@dyne.org>
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

#include <freej.h>
#include <freej_plugin.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>

#define RED(n)  ((n>>16) & 0x000000FF)
#define GREEN(n) ((n>>8) & 0x000000FF)
#define BLUE(n)  ((n>>0) & 0x000000FF)
#define RGB(r,g,b) ((0<<24) + (r<<16) + (g <<8) + (b))

/* setup some data to identify the plugin */
static char *name = "Cartoon"; /* do not assign a *name longer than 8 chars! */
static char *author = "Dries Pruimboom"; 
static char *info = "Cartoon";
static int version = 1; /* version is just an int (sophisticated isn't it?) */

/* save here screen geometry informations */
static ScreenGeometry *geo;

/* buffer where to copy the screen
   a pointer to it is being given back by process() */
static void *procbuf;

unsigned int *d;
int32_t *prePixBuffer = NULL;
int32_t *conBuffer = NULL;
int *yprecal;
int OPT_TripLevel=1200;
int OPT_DiffSpace = 1;
int32_t black;

void FlattenColor(int32_t *c);
void ColorBoost(int32_t *c);
void FlatAndBoost(int32_t *c);
void ColorWhite(int32_t *c);
long GetMaxContrast(int32_t *src,int x,int y);
void MakeCartoon(int32_t *src,int32_t *dst);
void ColorCopy(int32_t *c) { }
void (*ColorAction)(int32_t *)=ColorWhite;
void (*PrePixelModify)(int32_t *)=ColorBoost;

#define PIXELAT(x1,y1,s) ((s)+(x1)+ yprecal[y1])// (y1)*(geo->w)))
#define GMERROR(cc1,cc2) ((((RED(cc1)-RED(cc2))*(RED(cc1)-RED(cc2))) + \
   			((GREEN(cc1)-GREEN(cc2)) *(GREEN(cc1)-GREEN(cc2))) + \
			((BLUE(cc1)-BLUE(cc2))*(BLUE(cc1)-BLUE(cc2)))))

long gmerror(int32_t a, int32_t b) {
  int8_t *pa; int8_t *pb;
  int32_t gr,gg,gb;
  pa = (int8_t*)&a; pb = (int8_t*)&b;
  gr = *pa - *pb; gr *= gr;          /* red */
  gg = *(pa+1) - *(pb+1); gg *= gg;  /* green */
  gb = *(pa+2) - *(pb+2); gb *= gb;  /* blue */
  return(gr+gg+gb);
}

int init(ScreenGeometry *sg) {
  int c;
  geo = sg;
  procbuf = malloc(geo->size);
  prePixBuffer = malloc(geo->size);
  conBuffer = malloc(geo->size);
  d = (unsigned int*)procbuf;
  yprecal = (int*)malloc(geo->h*2*sizeof(int));
  for(c=0;c<geo->h*2;c++)
    yprecal[c] = geo->w*c;
  black = 0x00000000;
  return(1);
}

int clean() {
  free(procbuf);
  free(prePixBuffer);
  free(conBuffer);
  free(yprecal);
  return(1);
}

void *process(void *buffo) {
  MakeCartoon(buffo,procbuf);
  return procbuf;
}


int kbd_input(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_w: OPT_TripLevel +=100; break;
  case SDLK_q: OPT_TripLevel -=100; break;
  case SDLK_s: OPT_DiffSpace++; break;
  case SDLK_a: OPT_DiffSpace--; break;
  case SDLK_e: ColorAction = ColorWhite; break;
  case SDLK_r: ColorAction = FlattenColor; break;
  case SDLK_t: ColorAction = ColorCopy; break;
  case SDLK_y: ColorAction = ColorBoost; break;
  case SDLK_u: ColorAction = FlatAndBoost; break;
  case SDLK_i: PrePixelModify = ColorCopy; break;
  case SDLK_o: PrePixelModify = ColorBoost; break;	
  default: res = 0; break;
  }
  return res;
}


void FlattenColor(int32_t *c) {
  // (*c) = RGB(40*(RED(*c)/40),40*(GREEN(*c)/40),40*(BLUE(*c)/40)); */
  int8_t *p;
  p = (int8_t*)c;
  (*p) = ((*p)>>4)<<4;
  *(p+1) = (*(p+1)>>4)<<4;
  *(p+2) = (*(p+2)>>4)<<4;
}

// #define BOOST(n) RGB((RED(*c)<<7)>>n,(GREEN(*c)<<7)>>n,(BLUE(*c)<<7)>>n)
#define BOOST(n) { \
*p = ((*p)<<7)>>n; \
*(p+1) = (*(p+1)<<7)>>n; \
*(p+2) = (*(p+2)<<7)>>n; }

void ColorBoost(int32_t *c) {
   int max=0;
   int8_t *p;
   p = (int8_t*)c;
   /* Find biggest of r,g and b */
   if((*p)>max) max = *p;
   if(*(p+1)>max) max = *(p+1);
   if(*(p+2)>max) max = *(p+2);
   if(max==0) return;
   if (max<2) BOOST(1)
   else if (max<4) BOOST(2)
   else if (max<8) BOOST(3)
   else if (max<16) BOOST(4)
   else if (max<32) BOOST(5)
   else if (max<64) BOOST(6)
   else if (max<128) BOOST(7)
   else if (max<256) BOOST(8)

		       /* above optimization by jaromil, was:
			  Find biggest of r,g and b 
			  if (RED(*c) > max) max = RED(*c);
			  if (GREEN(*c) > max) max = GREEN(*c);
			  if (BLUE(*c) > max) max = BLUE(*c); */
		       
		       /* make biggest component 100% (255)  
			  if (max==0) return;
			  if (max<2) (*c) = BOOST(1);
			  else if (max<4) (*c) = BOOST(2);
			  else if (max<8) (*c) = BOOST(3);
			  else if (max<16) (*c) = BOOST(4);
			  else if (max<32) (*c) = BOOST(5);
			  else if (max<64) (*c) = BOOST(6);
			  else if (max<128) (*c) = BOOST(7);
			  else if (max<256) (*c) = BOOST(8); */
		       /*     (*c) = RGB((RED(*c)*255)/max,
			      (GREEN(*c)*255)/max,
			      (BLUE(*c)*255)/max);    
			      (*c) = RGB((RED(*c)<<8)/max,
			      (GREEN(*c)<<8)/max,
			      (BLUE(*c)<<8)/max); */

}		       


void FlatAndBoost(int32_t *c) {
  FlattenColor(c);
  ColorBoost(c);
}

void ColorWhite(int32_t *c) {
  (*c) = 0xffffffff; //RGB(255,255,255);
}

long GetMaxContrast(int32_t *src,int x,int y) {
  int32_t c1,c2;
  long error,max=0;
  
  /* Assumes PrePixelModify has been run */
  c1 = *PIXELAT(x-OPT_DiffSpace,y,src);
  c2 = *PIXELAT(x+OPT_DiffSpace,y,src);	
  error = GMERROR(c1,c2);
  if (error>max) max = error;
  
  c1 = *PIXELAT(x,y-OPT_DiffSpace,src);
  c2 = *PIXELAT(x,y+OPT_DiffSpace,src);
  error = GMERROR(c1,c2);
  if (error>max) max = error;
  
  c1 = *PIXELAT(x-OPT_DiffSpace,y-OPT_DiffSpace,src);
  c2 = *PIXELAT(x+OPT_DiffSpace,y+OPT_DiffSpace,src);
  error = GMERROR(c1,c2);
  if (error>max) max = error;
  
  c1 = *PIXELAT(x+OPT_DiffSpace,y-OPT_DiffSpace,src);
  c2 = *PIXELAT(x-OPT_DiffSpace,y+OPT_DiffSpace,src);
  error = GMERROR(c1,c2);
  if (error>max) max = error;
  
  return(max);
}


/*
 *  Cartoonify picture, do a form of edge detect 
 */
void MakeCartoon(int32_t *src,int32_t *dst) {
  int x,y;
  long t;
  
  for (x=0;x<geo->w;x++) {
    for (y=0;y<geo->h;y++) {
      *(prePixBuffer+x+yprecal[y]) =
	*(src+x+yprecal[y]);
      PrePixelModify(prePixBuffer+x+yprecal[y]);
    }
  }

  for (x=OPT_DiffSpace;x<geo->w-(1+OPT_DiffSpace);x++) {
    for (y=OPT_DiffSpace;y<geo->h-(1+OPT_DiffSpace);y++) {
      long t;
      if ((t = GetMaxContrast(prePixBuffer,x,y))>OPT_TripLevel) {
	//  Make a border pixel 
	*(dst+x+yprecal[y]) = 0x0;
      } else {
	//   Copy original color 
	*(dst+x+yprecal[y]) = *(prePixBuffer+x+yprecal[y]);
	ColorAction(dst+x+yprecal[y]);
      }
    }
  }

}
