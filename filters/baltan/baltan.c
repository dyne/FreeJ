/*
 *
 * BaltanTV - like StreakTV, but following for a long time
 * Copyright (C) 2001 FUKUCHI Kentarou
 * ported to FreeJ by jaromil
 *
 */

#include <stdlib.h>
#include <string.h>

#include <freej.h>

#define PLANES 32

#define STRIDE 8
#define STRIDE2 16 /* (STRIDE*2) */
#define STRIDE3 24 /* (STRIDE*3) */

static char *name = "Baltan";
static char *author = "Fukuchi Kentarou";
static char *info = "delayed alphachannel blit";
static int version = 1;
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

static uint32_t *planebuf;
static uint32_t *planetable[PLANES];
static void *procbuf;
static int plane;
static int pixels;

static ScreenGeometry *geo;

int init(ScreenGeometry *sg) {
    int i;
    
    geo = sg;
    pixels = geo->w*geo->h;
    
    planebuf = malloc(geo->size*PLANES);
    bzero(planebuf, geo->size*PLANES);

    for(i=0;i<PLANES;i++)
      planetable[i] = &planebuf[pixels*i];

    procbuf = malloc(geo->size);

    plane = 0;
    
    return 1;
}

int clean() {
  free(procbuf);
  return 1;
}

void *process(void *buffo) {
  int i, cf;

  uint32_t *buf = (uint32_t*)buffo;
  uint32_t *dst = (uint32_t*)procbuf;

  
  for(i=0; i<pixels; i++)
    planetable[plane][i] = (buf[i] & 0xfcfcfc)>>2;
  

  cf = plane & (STRIDE-1);
  
  for(i=0; i<pixels; i++) {
    dst[i] = planetable[cf][i]
      + planetable[cf+STRIDE][i]
      + planetable[cf+STRIDE2][i]
      + planetable[cf+STRIDE3][i];
    planetable[plane][i] = (dst[i]&0xfcfcfc)>>2;
  }

  plane++;
  plane = plane & (PLANES-1);
  
  return procbuf;
}

int kbd_input(char key) { return 0; }
