#include <inttypes.h>
#include <judenburg.h>

void *asmsrc;
void *asmdst;
unsigned int asmnum1;
unsigned int asmnum2;
Uint8 threshold[8]; /* r g b r g b */

/* for external assembler routines must require C linkage
   if included in c++ files */
extern "C" {
  void asm_judenburg(void);
}

Judenburg::Judenburg() {
  initialized = false;
  procbuf = NULL;
  
  supported[0] = false;
  supported[1] = true;
  supported[2] = false;
  supported[3] = true;

  strcpy(name,"Judenburg");
  strcpy(author,"jaromil@dyne.org");
  version = 1;
}

bool Judenburg::init() {
  func("Judenburg::init");

  procbuf = jalloc(procbuf,*size);
  r = g = b = a = 0x69;
  return(procbuf!=NULL);
}

void *Judenburg::process(void *buffo) {
  
  asmsrc = buffo;
  asmdst = procbuf;
  asmnum1 = *size;

  threshold[0] = r;
  threshold[1] = g;
  threshold[2] = b;
  threshold[3] = a;
  threshold[4] = r;
  threshold[5] = g;
  threshold[6] = b;
  threshold[7] = a;

  asm_judenburg();
  return procbuf;
}

void Judenburg::_delete() {
  func("Judenburg::_delete()");
  
  if(initialized)
    jfree(procbuf);
}

bool Judenburg::kbd_input(SDL_keysym *keysym) {
  bool res = true;
  int step = (keysym->mod==KMOD_LSHIFT) ? 10 : 1;

  switch(keysym->sym) {
  case SDLK_r:
    r += step;
    break;
  case SDLK_f:
    r -= step;
    break;
  case SDLK_t:
    g += step;
    break;
  case SDLK_g:
    g -= step;
    break;    
  case SDLK_y:
    b += step;
    break;
  case SDLK_h:
    b -= step;
    break;
  case SDLK_u:
    a += step;
    break;
  case SDLK_j:
    a -= step;
    break;
  default:
    res = false;
  }
  return res;
}
