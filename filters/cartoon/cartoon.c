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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define RED(n)  ((n>>16) & 0x000000FF)
#define GREEN(n) ((n>>8) & 0x000000FF)
#define BLUE(n)  (n & 0x000000FF)
#define RGB(r,g,b) ((r<<16) + (g <<8) + (b))

/* setup some data to identify the plugin */
static char *name = "Cartoon"; /* do not assign a *name longer than 8 chars! */
static char *author = "Dries Pruimboom"; 
static char *info = "Cartoon";
static int version = 1; /* version is just an int (sophisticated isn't it?) */
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

/* save here screen geometry informations */
static ScreenGeometry *geo;

/* buffer where to copy the screen
   a pointer to it is being given back by process() */
static void *procbuf;

unsigned int *d;
int32_t *prePixBuffer = NULL;
int32_t *conBuffer = NULL;
int *yprecal;
uint16_t powprecal[256];
int OPT_TripLevel=1000;
int OPT_DiffSpace = 1;
int32_t black;

void FlattenColor(int32_t *c);
void ColorWhite(int32_t *c);
long GetMaxContrast(int32_t *src,int x,int y);
void MakeCartoon(int32_t *src,int32_t *dst);
void ColorCopy(int32_t *c) { }
void (*ColorAction)(int32_t *)=FlattenColor;
void (*PrePixelModify)(int32_t *)=ColorCopy;

#define PIXELAT(x1,y1,s) ((s)+(x1)+ yprecal[y1])// (y1)*(geo->w)))
#define GMERROR(cc1,cc2) ((((RED(cc1)-RED(cc2))*(RED(cc1)-RED(cc2))) + \
   			((GREEN(cc1)-GREEN(cc2)) *(GREEN(cc1)-GREEN(cc2))) + \
			((BLUE(cc1)-BLUE(cc2))*(BLUE(cc1)-BLUE(cc2)))))

/* the following should be faster on older CPUs
   but runs slower than the GMERROR define on SSE units*/
inline uint16_t gmerror(int32_t a, int32_t b) {
  register int dr, dg, db;
  if((dr = RED(a) - RED(b)) < 0) dr = -dr;
  if((dg = GREEN(a) - GREEN(b)) < 0) dg = -dg;
  if((db = BLUE(a) - BLUE(b)) < 0) db = -db;
  return(powprecal[dr]+powprecal[dg]+powprecal[db]);
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
  for(c=0;c<256;c++) 
    powprecal[c] = c*c;
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


int kbd_input(char key) {
  int res = 1;
  switch(key) {
  case 'w': OPT_TripLevel +=100; break;
  case 'q': OPT_TripLevel -=100; break;
  case 's': if(OPT_DiffSpace<0xff) OPT_DiffSpace++; break;
  case 'a': if(OPT_DiffSpace>1) OPT_DiffSpace--; break;
  case 'e': ColorAction = ColorWhite; PrePixelModify = ColorCopy; break;
  case 'r': ColorAction = ColorCopy; break;
  case 't': ColorAction = FlattenColor; break;
  default: res = 0; break;
  }
  return res;
}


void FlattenColor(int32_t *c) {
  // (*c) = RGB(40*(RED(*c)/40),40*(GREEN(*c)/40),40*(BLUE(*c)/40)); */
  uint8_t *p;
  p = (uint8_t*)c;
  (*p) = ((*p)>>5)<<5; p++;
  (*p) = ((*p)>>5)<<5; p++;
  (*p) = ((*p)>>5)<<5;
}

#define BOOST(n) { \
if((*p = *p<<4)<0)>>n; \
*(p+1) = (*(p+1)<<4)>>n; \
*(p+2) = (*(p+2)<<4)>>n; }

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
  //  long t;
  /*  
  for (x=0;x<geo->w;x++) {
    for (y=0;y<geo->h;y++) {
      *(prePixBuffer+x+yprecal[y]) =
	*(src+x+yprecal[y]);
      PrePixelModify(prePixBuffer+x+yprecal[y]);
    }
  }
  */

  for (x=OPT_DiffSpace;x<geo->w-(1+OPT_DiffSpace);x++) {
    for (y=OPT_DiffSpace;y<geo->h-(1+OPT_DiffSpace);y++) {
      int t;
      if ((t = GetMaxContrast(src,x,y))>OPT_TripLevel) {
	//  Make a border pixel 
	*(dst+x+yprecal[y]) = 0x0;
      } else {
	//   Copy original color 
	*(dst+x+yprecal[y]) = *(src+x+yprecal[y]);
	ColorAction(dst+x+yprecal[y]);
      }
    }
  }

}
