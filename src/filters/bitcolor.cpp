/* dumb colorized filter
   by jaromil@dyne.org
*/

#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitcolor.h"

Bitcolor::Bitcolor() {
  initialized = false;
  procbuf = NULL;

  supported[0] = false; /* 8bit depth */
  supported[1] = true;  /* 16 bit depth */
  supported[2] = true; /* 24 bit depth */
  supported[3] = true;  /* 32 bit depth */  

  strcpy(name,"Bitcolor");
  strcpy(author,"jaromil@dyne.org");
  version = 1;
  
}

bool Bitcolor::init() {
  func("Bitcolor::init");  
  
  procbuf = jalloc(procbuf,*size);
  
  ccodio = 3;
  return(true);
}

void *Bitcolor::process(void *buffo) {
  
  unsigned long c, cc;
  Uint8 mask = 0x0;
  
  for(cc=0;cc<ccodio;cc++)
    mask |= (1<<cc);
  
  for(c=0;c<(*size);c++)
    ((Uint8*)procbuf)[c] = 
      ((((Uint8*)buffo)[c]) & mask);

  return(procbuf);
  
}

void Bitcolor::_delete() {
  func("Bitcolor::_delete()");
  
  if(initialized)
    jfree(procbuf);
}

bool Bitcolor::kbd_input(SDL_keysym *keysym) {
  bool res = false;
  switch(keysym->sym) {
  case SDLK_p:
    inc_bitmask();
    res = true;
    break;
  case SDLK_o:
    dec_bitmask();
    res = true;
    break;
  default:
    break;
  }
  return(res);
}
