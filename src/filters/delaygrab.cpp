/*
  FreeJ effect Delaygrab

  delaygrab - blockwise, controllable image delay
  Copyright (C) 1999/2000  A. Schiffler
  further done modifications by <jaromil@dyne.org>
  
  original sourcecode is from libbgrab 2.1f
  ported to FreeJ and successively modified by jaromil
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  
  To contact the author try these:
  |  Andreas Schiffler                    aschiffler@home.com  |
  |  Senior Systems Engineer    -    Deskplayer Inc., Buffalo  |
  |  4707 Eastwood Cres., Niagara Falls, Ont  L2E 1B4, Canada  |  
  |  +1-905-371-3652 (private)  -  +1-905-371-8834 (work/fax)  |
  
*/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <lubrify.h>
#include "delaygrab.h"

#define QUEUEDEPTH 71 /* was 76 */

static bool _lock = false;

void lock() {
  while(_lock)
    SDL_Delay(10);
  _lock = true;
}

void unlock() {
  _lock = false;
}

Delaygrab::Delaygrab() {
  initialized = false;

  delaymap=NULL;
  imagequeue=NULL;
  procbuf=NULL;

  supported[0] = true; /* 8bit depth */
  supported[1] = true;  /* 16 bit depth */
  supported[2] = true; /* 24 bit depth */
  supported[3] = true;  /* 32 bit depth */

  strcpy (name,"Delaygrab");
  strcpy (author,"Andreas Schiffler, Jaromil");
  version = 1;

}

void Delaygrab::_delete() {
  func("Delaygrab::_delete()");
  if(initialized) {
    jfree(delaymap);
    jfree(imagequeue);
    jfree(procbuf);
  }
}

bool Delaygrab::init() {
  /* call first the class initializer */
  func("Delaygrab::init()");

  /* starting mode */
  current_mode = 4;
  /* starting blocksize */
  set_blocksize(2);

  imagequeue = (Uint8 *) jalloc(imagequeue,QUEUEDEPTH*(*size));
  procbuf = jalloc(procbuf,*size);
  
  curqueue=imagequeue;
  curqueuenum=0;
  initialized  = true;
  func("Delaygrab initialized");
  return(true);
}

void Delaygrab::createDelaymap(int mode) {
  
  func("Delaygrab::createDelaymap(%u)",mode);

  lock();
  
  curdelaymap=(Uint32 *)delaymap;
  srand(time(NULL));

  for (y=0; y<delaymapheight; y++) {
    for (x=0; x<delaymapwidth; x++) {
      switch (mode) {
      case 1:
	double d;
	/* Random delay with square distribution */
	d = (double)rand()/(double)RAND_MAX;
	*curdelaymap = (int)(d*d*16.0);
	break;
      case 2:
	/* Vertical stripes of increasing delay outward from center */
	if (x<(delaymapwidth/2)) {
	  v=(delaymapwidth/2)-x;
	} else if (x>(delaymapwidth/2)) {
	  v=x-(delaymapwidth/2);
	} else {
	  v=0;
	}
	*curdelaymap=v/2;
	break;
      case 3:
	/* Horizontal stripes of increasing delay outward from center */
	if(y<(delaymapheight/2)) {
	  v = (delaymapheight/2)-y;
	} else if(y>(delaymapheight/2)) {
	  v = y-(delaymapheight/2);
	} else {
	  v=0;
	}
	*curdelaymap=v/2;
	break;
      case 4:
	/* Rings of increasing delay outward from center */
	v = (int)sqrt((double)((x-(delaymapwidth/2))*
			       (x-(delaymapwidth/2))+
			       (y-(delaymapheight/2))*
			       (y-(delaymapheight/2))));
	*curdelaymap=v/2;
	break;
      } // switch
      /* Clip values */
      if (*curdelaymap<0) {
	*curdelaymap=0;
      } else if (*curdelaymap>(QUEUEDEPTH-1)) {
	*curdelaymap=(QUEUEDEPTH-1);
      }
      curdelaymap++;
    }
  }
  current_mode = mode;

  unlock();

}

void Delaygrab::set_blocksize(int bs) {
  lock();

  blocksize = bs;
  block_per_pitch = blocksize*(*pitch);
  block_per_bytespp = blocksize*(bpp>>3);
  block_per_res = blocksize<<(bpp>>4);
  
  delaymapwidth = (*w)/blocksize;
  delaymapheight = (*h)/blocksize;
  delaymapsize = delaymapheight*delaymapwidth;

  if(delaymap!=NULL) { jfree(delaymap); delaymap = NULL; }
  delaymap = jalloc(delaymap,delaymapsize*4);

  unlock();

  createDelaymap(current_mode);

}

bool Delaygrab::kbd_input(SDL_keysym *keysym) {
  bool res = true;
  switch(keysym->sym) {
  case SDLK_w:
    inc_mode();
    break;
  case SDLK_q:
    dec_mode();
    break;
  case SDLK_s:
    inc_blocksize();
    break;
  case SDLK_a:
    dec_blocksize();
    break;
  default:
    res = false;
    break;
  }
  return res;
}

void *Delaygrab::process(void *buffo) {

  lock();

  /* Update queue pointer */
  if (curqueuenum==0) {
    curqueuenum=QUEUEDEPTH-1;
    curqueue = imagequeue;
    curqueue += (*size*(QUEUEDEPTH-1));
  } else {
    curqueuenum--;
    curqueue -= *size;
  }

   /* Copy image to queue */
  mmxcopy(buffo,curqueue,*size);

     /* Copy image blockwise to screenbuffer */
  curdelaymap= (Uint32 *)delaymap;
  for (y=0; y<delaymapheight; y++) {
    for (x=0; x<delaymapwidth; x++) {

      curposnum=((curqueuenum + (*curdelaymap)) % QUEUEDEPTH);
      
      xyoff= (x*block_per_bytespp) + (y*block_per_pitch);
      /* source */
      curpos= imagequeue;
      curpos += (*size*curposnum);
      curpos += xyoff;
      /* target */
      curimage = (Uint8 *)procbuf;
      curimage += xyoff;
      /* copy block */
      for (i=0; i<blocksize; i++) {
	memcpy(curimage,curpos,block_per_res);
	curpos += *pitch;
	curimage += *pitch;
      }
      curdelaymap++;
    }
  }

  unlock();
  return(procbuf);
}
