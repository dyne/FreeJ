/*  FreeJ
 *  Particle generator layer
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 *  blossom original algo is (c) 2003 by ragnar (waves 1.2)
 *  http://home.uninet.ee/~ragnar/waves
 *  further optimizations and changes followed
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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gen_layer.h>
#include <jutils.h>
#include <context.h>
#include <config.h>

/* defines for blob size and roundness */
#define LIM 8 // 25
#define NB_BLOB 16 // 25


GenLayer::GenLayer()
  :Layer() {

  /* initialize prime numbers */
  prime[0] = 2;
  prime[1] = 3;
  prime[2] = 5;
  prime[3] = 7;
  prime[4] = 11;
  prime[5] = 13;
  prime[6] = 17;
  prime[7] = 19;
  prime[8] = 23;
  prime[9] = 29;
  prime[10] = 31;
  
  /* blossom vars */
  blossom_count = 0; 
  blossom_m = 0;
  blossom_n = 0;
  blossom_i = 0;
  blossom_j = 0;
  blossom_k = 0;
  blossom_l = 0;
  blossom_r = 1;
  blossom_a = 0;
  
  /* initialize color masks */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x00000000;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0x00000000;
#endif
  
  surf = NULL;
  pixels = NULL;
  blob_buf = NULL;

  pi2 = 2.0*M_PI;

  set_name("GEO");

}

GenLayer::~GenLayer() {
  //  if(surf) SDL_FreeSurface(surf);
  if(pixels) free(pixels);
  if(blob_buf) free(blob_buf);
}

bool GenLayer::open(char *file) {
  /* we don't need this */
  return true;
}

void GenLayer::close() {
  /* neither this */
  return;
}

bool GenLayer::init(Context *scr) {
  
  /* allocate the buffer on which the layer will write */

  fastsrand( time(NULL) );

  /* internal initalization */
  _init(scr,scr->screen->w,scr->screen->h,32);

  pixels = (uint32_t*)malloc(geo.size);

    //  surf = SDL_CreateRGBSurface
    //    (SDL_HWSURFACE|SDL_HWACCEL,geo.w,geo.h,32,bmask,gmask,rmask,amask);
    //  if(!surf) {
  if(!pixels) {
    error("can't allocate GenLayer memory surface");
    return(false);
  }
  
  blossom_recal(true);

  /* blob initialization */
  blob_init(8);

  return(true);
}

void *GenLayer::feed() {
  /* automatic random recalculation:
     if( !blossom_count ) {    
     recalculate();
     blossom_count = 100+(50.0)*rand()/RAND_MAX;
     } else {
     blossom_count--;
  */
  blossom_a += 0.01;
  if( blossom_a > pi2 )
    blossom_a -= pi2;


  //  SDL_FillRect(surf,NULL,0x0);
  jmemset(pixels,0,geo.size);
  /*  
  if (SDL_MUSTLOCK(surf))
    if (SDL_LockSurface(surf) < 0) {
      error("%s", SDL_GetError());
      return NULL;
    }
  */
  blossom();
  return(pixels);
  /*
  if (SDL_MUSTLOCK(surf)) {
    SDL_UnlockSurface(surf);
  }
  */
}

void GenLayer::blossom_recal(bool r) {
  lock_feed();
  float z = ((PRIMES-2)*fastrand()/RAND_MAX)+1;
  blossom_m = 1.0+(30.0)*fastrand()/RAND_MAX;
  blossom_n = 1.0+(30.0)*fastrand()/RAND_MAX;
  blossom_i = prime[ (int) (z*fastrand()/RAND_MAX) ];
  blossom_j = prime[ (int) (z*fastrand()/RAND_MAX) ];
  blossom_k = prime[ (int) (z*fastrand()/RAND_MAX) ];
  blossom_l = prime[ (int) (z*fastrand()/RAND_MAX) ];
  wd = (double)geo.w;
  hd = (double)geo.h;
  if(r)
    blossom_r = (blossom_r>=1.0)?1.0:blossom_r+0.1;
  else
    blossom_r = (blossom_r<=0.1)?0.1:blossom_r-0.1;
  unlock_feed();
}  

void GenLayer::blossom() {
  
  float	a;
  int x, y;
  double zx, zy;

  /* here place a formula that draws on the screen
     the surface being drawed at this point is always blank */
  for( a=0.0 ; a<pi2; a+=0.005 ) {
    zx = blossom_m*a;
    zy = blossom_n*a;
    x = (int)(wd*(0.47+ (blossom_r*sin(blossom_i*zx+blossom_a)+
			 (1.0-blossom_r)*sin(blossom_k*zy+blossom_a)) /2.2 ));
    
    y = (int)(hd*(0.47+ (blossom_r*cos(blossom_j*zx+blossom_a)+
			 (1.0-blossom_r)*cos(blossom_l*zy+blossom_a)) /2.2 ));
    
    blob(x,y);
    
  } 

}

void GenLayer::blob_init(int ray) {
  int i,j;
  int val;
  uint8_t col;

  blob_size = ray*2;

  /* @TODO
     there should be a gradient sphere here
     Niels helps me with this: calculating a circle
     while(theta <= 360) {
     double radians = (theta / 180.0) * PI;
     double dx = ( origin[0] + cos( radians ) * radius );
     double dy = ( origin[1] + sin( radians ) * radius );
     
     (there are always some basics you learn at school and then forget)
  */
  uint32_t dx,dy;
  double rad, th;
  int c;
  srand(time(NULL));
  if(blob_buf) free(blob_buf);
  
  blob_buf = (uint32_t*) calloc(ray*2*ray*2,sizeof(uint32_t));
  //  memset(blob_buf,0,ray*2*ray*2*sizeof(uint32_t));

  for(th=1;th<=360;th++) {
    rad = (th / 180.0) * M_PI;
    for(c=ray;c>0;c--) {
      dx = ( (ray) + cos( rad ) * c );
      dy = ( (ray) + sin( rad ) * c );
      //      col = (int)(10.0*rand()/(RAND_MAX+1.0))/c;
      //      col += 0x99/c * 0.8;
      col = 0x99/c * 0.8;
      blob_buf[ (dx+((ray*2)*dy)) ] =
	col|(col<<8)|(col<<16)|(col<<24);
    }
  }
  
#if 0
  /* init sphere */
  if(blob_buf) free(blob_buf);
  blob_buf = (uint32_t*) malloc(8*blob_size*blob_size);
  
  for(j=0; j<blob_size; j++)
    for(i=0; i<blob_size; i++) {
      val = 
	(i-blob_size/2)*(i-blob_size/2) +
	(j-blob_size/2)*(j-blob_size/2);
      if(val < LIM*LIM) {
	val = val/(LIM*LIM);
	val = 1-val;
	val *= val;
      } else val = 0;
      col = (0xff/NB_BLOB)*val; // ((65535/NB_BLOB)*val);
      blob_buf[i+j*blob_size] = 
	SDL_MapRGB(surf->format,col,col,col);
    }
#endif
}
  

void GenLayer::blob(int x, int y) {
  //  if(y>geo.h-blob_size) return;
  //  if(x>geo.w-blob_size) return;

  int i, j;
  int stride = (geo.w-blob_size)>>1;

  uint64_t *tmp_scr = (uint64_t*)pixels + ((x + y*geo.w)>>1);
  uint64_t *tmp_blob = (uint64_t*)blob_buf;

#ifdef HAVE_MMX
  /* using mmx packed unsaturated addition on bytes
     for cleaner and shiny result */
  for(j=blob_size; j>0; j--) {
    for(i=blob_size>>4; i>0; i--) {
      
      asm volatile("movq (%1),%%mm0;"
		   "movq 8(%1),%%mm1;"
		   "movq 16(%1),%%mm2;"
		   "movq 24(%1),%%mm3;"
		   "movq 32(%1),%%mm4;"
		   "movq 40(%1),%%mm5;"
		   "movq 48(%1),%%mm6;"
		   "movq 56(%1),%%mm7;"

		   "paddusb (%0),%%mm0;" // packed add unsaturated on bytes
		   "paddusb 8(%0),%%mm1;" // addizione clippata
		   "paddusb 16(%0),%%mm2;"
		   "paddusb 24(%0),%%mm3;"
		   "paddusb 32(%0),%%mm4;"
		   "paddusb 40(%0),%%mm5;"
		   "paddusb 48(%0),%%mm6;"
		   "paddusb 56(%0),%%mm7;"

		   "movq %%mm0,(%0);"	  
		   "movq %%mm1,8(%0);"	  
		   "movq %%mm2,16(%0);"	  
		   "movq %%mm3,24(%0);"	  
		   "movq %%mm4,32(%0);"	  
		   "movq %%mm5,40(%0);"	  
		   "movq %%mm6,48(%0);"	 
		   "movq %%mm7,56(%0);"
		   //	  "paddsw %0, %%mm0;"// halo violetto?
		   :
		   :"r"(tmp_scr),"r"(tmp_blob)
		   :"memory");
      tmp_scr+=8;
      tmp_blob+=8;
    }
    tmp_scr += stride;
  }
  asm("emms;");


#else // ! HAVE_MMX
  for(j=blob_size; j>0; j--) {
    for(i=blob_size>>1; i>0; i--) {
      *(tmp_scr++) += *(tmp_blob++);
    }
    tmp_scr += stride;
  }
#endif

}

bool GenLayer::keypress(SDL_keysym *keysym) {
  switch(keysym->sym) {

  case SDLK_RIGHT:
    blossom_recal(true);
    break;

  case SDLK_LEFT:
    blossom_recal(false);
    break;

  default: return(false);
  
  }

  return(true);
}
    
