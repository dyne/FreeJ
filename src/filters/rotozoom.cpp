/* Rotozoom effect
 *
 * coded by jaromil (C)2001
 * 

 rotozoom works only on 256x256 and 512x512 layers

 this effect is a old fashion one, often used in demos and intros
 that's not the best implementation around (alltough is pretty
 optimized eh) but it can engage much some programmers out there to do
 something better ;) many people worked around rotozooms and the best
 i ever saw has been coded by Tijs van Bakel and Jorik Blaas
 (Slager Bert intro), i hope somebody dares implementing that one
 as a FreeJ effect one day :)
 
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

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>

#include <lubrify.h>
#include "rotozoom.h"

#ifndef M_PI
#define	M_PI		3.14159265358979323846	/* pi greco */
#endif

Rotozoom::Rotozoom() {
  initialized = false;
  procbuf=NULL;

  supported[0] = false; /* 8bit depth */
  supported[1] = true;  /* 16 bit depth */
  supported[2] = false; /* 24 bit depth */
  supported[3] = true;  /* 32 bit depth */

  strcpy(name,"Rotozoom");
  strcpy(author,"jaromil@dyne.org");
  version = 1;
}

void Rotozoom::_delete() {
  func("Rotozoom::_delete()");
  if(procbuf!=NULL) {
    jfree(procbuf);
  }
}

bool Rotozoom::init() {
  func("Rotozoom filter::init()");

  //  if((*w!=256)|(*h!=256)) {
  //    cerr << " ! rotozoom filters works only on 256x256 16bit layers";
  //    return(false);
  //  }
  procbuf = (unsigned char *) jalloc(procbuf,*size<<3);

  /* init of sin/cos lookup tables */
  init_tables();
  initialized = true;
  func("Rotozoom initialized");
  return(true);
}

void *Rotozoom::process(void *buffo) {
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
  
  /* notice the doubling of size of the layer*/
  //  *w = *w; *h = *h; *pitch = *pitch; *size = *size;
   
  for ( j=0 ; j<(*w) ; j++ ) {
    x = (xprime + xi[theta]);
    xprime += h_sin[theta];
    
    y = (yprime + yi[theta]);
    yprime += h_cos[theta];
    
    for ( i=0 ; i<(*h) ; i++ ) {
      x += h_cos[theta];
      y -= h_sin[theta];
      

      switch(bpp) {
      case 16:
	rotor = ((y&0xFF00) + ((x>>8)&0x00FF))<<1;
	*rotat++ = *(buffer+rotor);
	*rotat++ = *(buffer+rotor+1);
	break;
      case 32:
	rotor = ((y&0xFF00) + ((x>>8)&0x00FF))<<2;
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
  
void Rotozoom::init_tables() {
  int i;
  double hh;
  double radian;

  theta = 180;

  for ( i=0 ; i<360 ; i++ )
    {
      
      radian = 2*i*M_PI/360;
      
      hh = 2+ cos(radian);
      h_cos[i] = (int) ((128<<1) * (hh * cos(radian)));
      h_sin[i] = (int) ((128<<1) * (hh * sin(radian)));
      xi[i] = (int) -((*w)>>1) * h_cos[i];
      yi[i] = (int) ((*w)>>1) * h_sin[i];
      xj[i] = (int) -((*h)>>1) * h_sin[i];
      yj[i] = (int) -((*h)>>1) * h_cos[i];
    }
}
