#include <stdio.h>
#include <stdlib.h>

#include <freej.h>
#include <freej_plugin.h>

static char *name = "Absdiff";
static char *author = "jaromil";
static char *info = "Absolute difference between frames";
static int version = 1;
static int bpp = 6;

static void *procbuf;
static void *lastimage;

static int threshold_value;

static ScreenGeometry *geo;

static void absdiff16(void *src1, void *src2, void *dst);
static void absdiff32(void *src1, void *src2, void *dst);

int init(ScreenGeometry *sg) {
  geo = sg;
  lastimage = malloc(geo->size);
  if(!lastimage) return 0;
  procbuf = malloc(geo->size);
  if(!procbuf) return 0;

  threshold_value = 0;
  return(1);
}

int clean() {
  free(lastimage);
  free(procbuf);
  return(1);
}

void *process(void *buffo) {

  switch(geo->bpp) {
  case 16: absdiff16(buffo,lastimage,procbuf); break;
  case 32: absdiff32(buffo,lastimage,procbuf); break;
  }
  
  return(procbuf);
}

extern void mmx_absdiff16(void);
extern void mmx_absdiff32(void); 
unsigned char *asmsrc1;
unsigned char *asmsrc2;
unsigned char *asmdst;
unsigned int asmnum1;
static void absdiff16(void *src1, void *src2, void *dst) {
  asmsrc1 = src1;
  asmsrc2 = src2;
  asmdst = dst;
  asmnum1 = geo->size;
  mmx_absdiff16();
}

static void absdiff32(void *src1, void *src2, void *dst) {
  asmsrc1 = src1;
  asmsrc2 = src2;
  asmdst = dst;
  asmnum1 = geo->size;
  mmx_absdiff32();
}

int kbd_input(SDL_keysym *keysym) { return 0; }
