#include <iostream.h>
#include <stdio.h>
#include <stdlib.h>
#include <lubrify.h>
#include "strobe.h"

Strobe::Strobe() {
  initialized = false;
  storebuf = NULL;
  frame_count = 0;
  delay = 15; /* default delay */

  supported[0] = true; /* 8bit depth */
  supported[1] = true;  /* 16 bit depth */
  supported[2] = true; /* 24 bit depth */
  supported[3] = true;  /* 32 bit depth */
  
  strcpy(name,"Strobe");
  strcpy(author,"jaromil@dyne.org");
  version = 1;
}

void Strobe::_delete() {
  func("Strobe::_delete()");

  if(initialized)
    jfree(storebuf);
}

bool Strobe::init() {
  func("Strobe::init");
  storebuf = jalloc(storebuf,*size);
  initialized = true;
  return(true);
}

void *Strobe::process(void *buffo) {
  frame_count++;

  if(frame_count>delay) {
    mmxcopy(buffo,storebuf,*size);
    frame_count = 0;
  }

  return(storebuf);
}

void Strobe::set_delay(int num) {
  delay = num;
  frame_count = 0;
}
