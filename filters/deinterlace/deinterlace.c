/*
 * EffecTV - Realtime Digital Video Effector
 * Copyright (C) 2001-2003 FUKUCHI Kentaro
 *
 * DeinterlaceTV - deinterlaces the video.
 * Copyright (C) 2001 Casandro (einStein@donau.de
 *
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <freej.h>
#include <freej_plugin.h>

/* DEFINE's by nullset@dookie.net */
#define RED(n)  ((n>>16) & 0x000000FF)
#define GREEN(n) ((n>>8) & 0x000000FF)
#define BLUE(n)  ((n>>0) & 0x000000FF)
#define RGB(r,g,b) ((0<<24) + (r<<16) + (g <<8) + (b))
#define INTENSITY(n)	( ( (RED(n)+GREEN(n)+BLUE(n))/3))

static char *name = "Deinterlace";
static char *author = "Fukuchi Kentaro";
static char *info = "deinterlace video";
static int version = 0;

static int start(void);
static int stop(void);
static int draw(uint32_t *src, uint32_t *dst);
static void *procbuf;
static ScreenGeometry *geo;
static int *ypos;
static int ww[3], hh[3], c;

static int state = 0;

int init(ScreenGeometry *sg) {

  geo = sg;
  procbuf = malloc(geo->size);

  //  hheight = geo->h/2;
  //  hwidth = geo->w/2;

  for(c=1;c<5;c++) {
    hh[c-1] = geo->h / c;
    ww[c-1] = geo->w / c;
  }

  ypos = malloc(geo->h * sizeof(int));
  for(c=0;c<geo->h;c++) ypos[c] = c*geo->w;

  c=0;
  return(1);
}

static int start()
{
	state = 1;
	return 0;
}

int clean() {
  free(procbuf);
  free(ypos);
  return(1);
}

/*int abs(int x)
{
  int y;
  y=x;
  if (y<0) y=0-x;
  return y;
}*/

static int Difference(int a,int b)
{
	return abs(GREEN(a)-GREEN(b));
}

void *process(void *buffo) {

  draw((uint32_t*)buffo, (uint32_t*)procbuf);
  
  return(procbuf);
}

static int MixPixels(int a,int b)
{
	return RGB(((  RED(a)+  RED(b))/2),
		   ((GREEN(a)+GREEN(b))/2),
		   (( BLUE(a)+ BLUE(b))/2));
}

static int draw(uint32_t *src, uint32_t *dst)
{
  int x,y;
  int zeile1a,zeile2a,zeile3a,zeile4a;
  int zeile1b,zeile2b,zeile3b,zeile4b;
  int zeile1c,zeile2c,zeile3c,zeile4c;
  int outp1,outp2,outp3,outp4,outp5,outp6;
  int d1,d2;

	for (y=1;y < geo->h-2; y+=2)
	  for (x=0;x<geo->w-2; x+=3){
	    zeile1a = *(uint32_t *)(src+(y-1)*geo->w+x);
	    zeile2a = *(uint32_t *)(src+(y+0)*geo->w+x);
	    zeile3a = *(uint32_t *)(src+(y+1)*geo->w+x);
	    zeile4a = *(uint32_t *)(src+(y+2)*geo->w+x);
	    zeile1b = *(uint32_t *)(src+(y-1)*geo->w+x+1);
	    zeile2b = *(uint32_t *)(src+(y+0)*geo->w+x+1);
	    zeile3b = *(uint32_t *)(src+(y+1)*geo->w+x+1);
	    zeile4b = *(uint32_t *)(src+(y+2)*geo->w+x+1);
	    zeile1c = *(uint32_t *)(src+(y-1)*geo->w+x+2);
	    zeile2c = *(uint32_t *)(src+(y+0)*geo->w+x+2);
	    zeile3c = *(uint32_t *)(src+(y+1)*geo->w+x+2);
	    zeile4c = *(uint32_t *)(src+(y+2)*geo->w+x+2);
	    
	    outp3 = zeile2b;
	    outp4 = zeile3b;
	    
	    outp1=zeile2a;
	    outp2=zeile3a;
	    outp5=zeile2c;
	    outp6=zeile3c;
	    d1=Difference(MixPixels(zeile1a,zeile1c),MixPixels(zeile3a,zeile3c))+
	       Difference(MixPixels(zeile2a,zeile2c),MixPixels(zeile4a,zeile4c));
	    d2=Difference(MixPixels(zeile1a,zeile1c),MixPixels(zeile4a,zeile4c))+
	       Difference(MixPixels(zeile2a,zeile2c),MixPixels(zeile3a,zeile3c));
	    if ((d1) < (d2))
	      {outp2=MixPixels(zeile2a,zeile4a);
	       outp4=MixPixels(zeile2b,zeile4b);
	       outp6=MixPixels(zeile2c,zeile4c);}
	    *(uint32_t *)(dst+x+(y-1)*geo->w) = outp1;
	    *(uint32_t *)(dst+x+(y+0)*geo->w) = outp2;
	    *(uint32_t *)(dst+x+1+(y-1)*geo->w) = outp3;
	    *(uint32_t *)(dst+x+1+(y+0)*geo->w) = outp4;
	    *(uint32_t *)(dst+x+2+(y-1)*geo->w) = outp5;
	    *(uint32_t *)(dst+x+2+(y+0)*geo->w) = outp6;
	  }				  
	
	return 0;
}
