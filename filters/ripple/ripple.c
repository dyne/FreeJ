/*
 * RippleTV - Water ripple effect.
 * Copyright (C) 2001 FUKUCHI Kentarou
 *
 */
#include <freej.h>

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

static char *name = "Ripple";
static char *author = "Fukuchi Kentarou";
static char *info = "water ripples";
static int version = 1;
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

static ScreenGeometry *geo;

static void *procbuf;

static int *map;
static int *map1, *map2, *map3;
static int map_h, map_w;
static int sqrtable[256];
static const int point = 16;
static const int impact = 1;//2
static const int decay = 8;
static const int loopnum = 2;
static uint32_t *background;
static unsigned char *diff;
static signed char *vtable;
static int newBackground;
static int threshold;


static void setTable() {
  int i;
  for(i=0; i<128; i++)
    sqrtable[i] = i*i;
  for(i=1; i<=128; i++)
    sqrtable[256-i] = -i*i;
}

static void setbg(uint32_t *src)
{
  int i;
  int R, G, B;
  uint32_t *p;
  short *q;
  
  p = src;
  q = (short *)background;
  for(i=0; i<geo->w*geo->h; i++) {
    R = ((*p)&0xff0000)>>(16-1);
    G = ((*p)&0xff00)>>(8-2);
    B = (*p)&0xff;
    *q = (short)(R + G + B);
    p++;
    q++;
  }
  newBackground = 0;
}

static void motiondetect(uint32_t *src) {
  int i;
  int R, G, B;
  uint32_t *t;
  short *z;
  unsigned char *r;
  
  int v;
  int width;
  int *p, *q;
  int x, y, h;
  unsigned char *pdiff;

  t = src;
  z = (short *)background;
  r = diff;
  for(i=0; i<geo->w*geo->h; i++) {
    R = ((*t)&0xff0000)>>(16-1);
    G = ((*t)&0xff00)>>(8-2);
    B = (*t)&0xff;
    v = (R + G + B) - (int)(*z);
    *z = (short)(R + G + B);
    *r = ((v + threshold)>>24) | ((threshold - v)>>24);
    
    t++;
    z++;
    r++;
  }

  pdiff = diff;

  width = geo->w;
  p = map1+map_w+1;
  q = map2+map_w+1;
  pdiff += width+2;

  for(y=map_h-2; y>0; y--) {
    for(x=map_w-2; x>0; x--) {
      h = (int)*pdiff + (int)*(pdiff+1) + (int)*(pdiff+width) + (int)*(pdiff+width+1);
      if(h>0) {
	*p = h<<(point + impact - 8);
	*q = *p;
      }
      p++;
      q++;
      pdiff += 2;
    }
    pdiff += width+2;
    p+=2;
    q+=2;
  }
}

int init(ScreenGeometry *sg) {
  geo = sg;

  procbuf = malloc(geo->size);

  map_h = geo->h /2 + 1;
  map_w = geo->w /2 + 1;
  map = (int*)malloc(map_h*map_w*3*sizeof(int));
  map3 = map+map_w * map_h *2;

  diff = (unsigned char*) malloc(geo->w*geo->h*sizeof(unsigned char));
  vtable = (signed char*) malloc(map_h*map_w*2*sizeof(signed char));
  background = (uint32_t*)malloc(geo->w*geo->h*sizeof(uint32_t));
  if(!map || !diff || !vtable || !background) return 0;
  setTable();

  memset(map,0,map_h*map_w*3*sizeof(int));
  memset(vtable,0,map_h*map_w*2*sizeof(signed char));
  map1 = map;
  map2 = map+map_w*map_w;
  threshold = 70*7;
  newBackground = 1;
  
  return 1;
}

void *process(void *buffo) {
  int x, y, i;
  int dx, dy;
  int h, v;
  int width, height;
  int *p, *q, *r;
  signed char *vp;

  uint32_t *src = (uint32_t*)buffo;
  uint32_t *dest = (uint32_t*)procbuf;

  if (newBackground)
    setbg(src);
  /* impact from the motion or rain drop */
  //	if(mode) {
  //		raindrop();
  //	} else {
  motiondetect(src);
  //	}
  
  /* simulate surface wave */
  width = map_w;
  height = map_h;
  
  /* This function is called only 30 times per second. To increase a speed
   * of wave, iterates this loop several times. */
  for(i=loopnum; i>0; i--) {
    /* wave simulation */
    p = map1 + width + 1;
    q = map2 + width + 1;
    r = map3 + width + 1;
    for(y=height-2; y>0; y--) {
      for(x=width-2; x>0; x--) {
	h = *(p-width-1) + *(p-width+1) + *(p+width-1) + *(p+width+1)
	  + *(p-width) + *(p-1) + *(p+1) + *(p+width) - (*p)*9;
	h = h >> 3;
	v = *p - *q;
	v += h - (v >> decay);
	*r = v + *p;
	p++;
	q++;
	r++;
      }
      p += 2;
      q += 2;
      r += 2;
    }
    
    /* low pass filter */
    p = map3 + width + 1;
    q = map2 + width + 1;
    for(y=height-2; y>0; y--) {
      for(x=width-2; x>0; x--) {
	h = *(p-width) + *(p-1) + *(p+1) + *(p+width) + (*p)*60;
	*q = h >> 6;
	p++;
	q++;
      }
      p+=2;
      q+=2;
    }
    
    p = map1;
    map1 = map2;
    map2 = p;
  }
  
  vp = vtable;
  p = map1;
  for(y=height-1; y>0; y--) {
    for(x=width-1; x>0; x--) {
      /* difference of the height between two voxel. They are twiced to
       * emphasise the wave. */
      vp[0] = sqrtable[((p[0] - p[1])>>(point-1))&0xff]; 
      vp[1] = sqrtable[((p[0] - p[width])>>(point-1))&0xff]; 
      p++;
      vp+=2;
    }
    p++;
    vp+=2;
  }
  
  height = geo->h;
  width = geo->w;
  vp = vtable;
  
  /* draw refracted image. The vector table is stretched. */
  for(y=0; y<height; y+=2) {
    for(x=0; x<width; x+=2) {
      h = (int)vp[0];
      v = (int)vp[1];
      dx = x + h;
      dy = y + v;
      if(dx<0) dx=0;
      if(dy<0) dy=0;
      if(dx>=width) dx=width-1;
      if(dy>=height) dy=height-1;
      dest[0] = src[dy*width+dx];
      
      i = dx;
      
      dx = x + 1 + (h+(int)vp[2])/2;
      if(dx<0) dx=0;
      if(dx>=width) dx=width-1;
      dest[1] = src[dy*width+dx];
      
      dy = y + 1 + (v+(int)vp[map_w*2+1])/2;
      if(dy<0) dy=0;
      if(dy>=height) dy=height-1;
      dest[width] = src[dy*width+i];
      
      dest[width+1] = src[dy*width+dx];
      dest+=2;
      vp+=2;
    }
    dest += geo->w;
    vp += 2;
  }

  return procbuf;
}

int clean() {
  free(procbuf);
  free(map);
  free(diff);
  free(vtable);
  free(background);
  return 1;
}

int kbd_input(char key) {
  return 0;
}
