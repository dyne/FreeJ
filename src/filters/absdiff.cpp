#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>

#include <lubrify.h>
#include "absdiff.h"

Absdiff::Absdiff() {
  initialized = false;
  lastimage = NULL;
  procbuf = NULL;
  threshold_value = 0;

  supported[0] = true; /* 8bit depth */
  supported[1] = true;  /* 16 bit depth */
  supported[2] = false; /* 24 bit depth */
  supported[3] = false;  /* 32 bit depth */
  
  strcpy(name,"Absdiff");
  strcpy(author,"jaromil@dyne.org");
  version = 1;
}

void Absdiff::_delete() {
  func("Absdiff::_delete()");

  if(initialized) {
    jfree(lastimage);
    jfree(procbuf);
  }

}

bool Absdiff::init() {

  lastimage = jalloc(lastimage,*size);
  procbuf = jalloc(procbuf,*size);
  initialized = true;
  return(initialized);

}

void *Absdiff::process(void *buffo) {

  switch(bpp) {
  case 8:
    mmxdiff8(buffo,lastimage,procbuf,*size);
    break;
  case 16:
    mmxdiff16(buffo,lastimage,procbuf,*size);
    break;
  }
  
  return(procbuf);
}
