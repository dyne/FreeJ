/* FreeJ
   Rotozoom effect
   (C)2001 by jaromil

 rotozoom works only on 256x256 and 512x512 layers (BEING FIXED)

 this effect is a old fashion one, often used in demos and intros.
 that's not the very best implementation around, but it can engage
 some programmers out there to do something better ;)
 many people worked around rotozooms and the best i ever saw has
 been coded by Tijs van Bakel and Jorik Blaas (Slager Bert intro),
 i hope somebody dares implementing that one as a FreeJ effect one day ;)
 
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <freej.h>
#include <freej_plugin.h>

#ifndef M_PI
#define	M_PI		3.14159265358979323846	/* pi greco */
#endif

static char *name = "Rotozoom";
static char *author = "jaromil";
static char *info = "old school rotozoom effect";
static int version = 1;

static void *procbuf;
static ScreenGeometry *geo;

static int theta;
static int h_cos [360];
static int h_sin [360];
static int xi [360];
static int yi [360];
static int xj [360];
static int yj [360];

static void init_tables();

int init(ScreenGeometry *sg) {
  geo = sg;
  procbuf = malloc(geo->size);

  init_tables();
  return(1);
}

int clean() {
  if(procbuf) free(procbuf);
  return(1);
}

void *process(void *buffo) {
  int i,j;
  
  short x,y;
  int rotor;
  int xprime = xj[theta];
  int yprime = yj[theta];
 
  /* for very strange effect here try:
     Uint16 *rotat = (Uint16 *)procbuf;
     Uint16 *buffer = (Uint16 *)buffo;
  */

  Uint8 *rotat = (Uint8 *)procbuf;
  Uint8 *buffer = (Uint8 *)buffo;
  
  for ( j=0 ; j<(geo->h) ; j++ ) {
    x = (xprime + xi[theta]);
    xprime += h_sin[theta];
    
    y = (yprime + yi[theta]);
    yprime += h_cos[theta];
    
    for ( i=0 ; i<(geo->w) ; i++ ) {
      x += h_cos[theta];
      y -= h_sin[theta];
      
      
      switch(geo->bpp) {
      case 16:
	/*	rotor = ((y&0xFF00) + ((x>>8)&0x00FF))<<1; */
	rotor = ((y&0xFF00) + ((x>>8)&0x00FF))<<1;
	*rotat++ = *(buffer+rotor);
	*rotat++ = *(buffer+rotor+1);
	break;
      case 32:
	/*	
	  rotor = ((y&0xFF00) + ((x>>8)&0x00FF))<<2;
	  *rotat++ = *(buffer+rotor);
	  *rotat++ = *(buffer+rotor+1);
	  *rotat++ = *(buffer+rotor+2);
	  *rotat++ = *(buffer+rotor+3);
	  */
	rotor = ((y&0xFF00) + ((x>>8)&0x00FF))*5;
	*rotat++ = *(buffer+rotor);
	*rotat++ = *(buffer+rotor+1);
	*rotat++ = *(buffer+rotor+2);
	*rotat++ = *(buffer+rotor+3);
	break;
      }
    }
  }
  theta = (theta+1)%360;
  return(procbuf);
}

static void init_tables() {
  int i;
  double hh;
  double radian;
  
  theta = 180;
  
  for ( i=0 ; i<360 ; i++ )
    {
      
      radian = 2*i*M_PI/360;
      
      hh = 2+ cos(radian);
      h_cos[i] = (int) ((geo->w) * (hh * cos(radian)));
      h_sin[i] = (int) ((geo->h) * (hh * sin(radian))); 
      /*      h_cos[i] = 256 * (hh * cos(radian));
	      h_sin[i] = 256 * (hh * sin(radian)); */
      
      xi[i] = (int) -(geo->w>>1) * h_cos[i];
      yi[i] = (int) (geo->w>>1) * h_sin[i];
      xj[i] = (int) -(geo->h>>1) * h_sin[i];
      yj[i] = (int) -(geo->h>>1) * h_cos[i];
    }
}
