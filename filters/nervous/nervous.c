/*
 * nervousTV - The name says it all...
 * Copyright (C) 2002 TANNENBAUM Edo
 *
 * 2002/2/9 
 *   Original code copied same frame twice, and did not use memcpy().
 *   I modifed those point.
 *   -Kentarou Fukuchi
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

/* setup some data to identify the plugin */
static char *name = "Nervous"; /* do not assign a *name longer than 8 chars! */
static char *author = "Edo Tannenbaum"; 
static char *info = "multiplanar nervous strobe";
static int version = 1; /* version is just an int (sophisticated isn't it?) */
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

/* save here screen geometry informations */
static ScreenGeometry *geo;

#define PLANES 32

static int32_t *buffer = NULL;
static int32_t *planetable[PLANES];
static int mode = 0;
static int plane, stock, timer, stride, readplane;


int init(ScreenGeometry *sg) {
  int c;
  geo = sg;

  buffer = malloc(geo->size*PLANES);
  if(!buffer) {
    fprintf(stderr,"ERROR: nervous plugin can't allocate needed memory\n");
    return 0;
  }
  memset(buffer,0,geo->size*PLANES);
  for(c=0;c<PLANES;c++)
    planetable[c] = &buffer[geo->w*geo->h*c];

  plane = 0;
  stock = 0;
  timer = 0;
  readplane = 0;

  return 1;
}

int clean() {
  if(buffer) free(buffer);
  return 1;
}

/* cheap & fast randomizer (by Fukuchi Kentarou) */
uint32_t randval;
uint32_t fastrand() { return (randval=randval*1103515245+12345); };
void fastsrand(uint32_t seed) { randval = seed; };

void *process(void *buffo) {

  memcpy(planetable[plane],buffo,geo->size);

  if(stock<PLANES) stock++;

  if(mode) {
    if(timer) {
      readplane = readplane + stride;
      while(readplane < 0) readplane += stock;
      while(readplane >= stock) readplane -= stock;
      timer--;
    } else {
      readplane = fastrand() % stock;
      stride = fastrand() % 5 - 2;
      if(stride >= 0) stride++;
      timer = fastrand() % 6 + 2;
    }
  } else
    if(stock > 0)
      readplane = fastrand() % stock;
  
  plane++;
  if(plane==PLANES) plane=0;
  
  return planetable[readplane];
}

int kbd_input(char key) {
  int res = 1;
  switch(key) {
  case 'q':
    mode ^= 1;
    break;
    
  default: 
    res = 0;
    break;
  }

  return res;
}

