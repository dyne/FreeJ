#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <freej.h>
#include <freej_plugin.h>

static char *name = "Absdiff";
static char *author = "jaromil";
static char *info = "Absolute difference between frames";
static int version = 1;

static void *procbuf;
static void *lastimage;

static int threshold_value;

static ScreenGeometry *geo;

extern void mmx_absdiff32(void); 
unsigned char *absdiff_asmsrc1;
unsigned char *absdiff_asmsrc2;
unsigned char *absdiff_asmdst;
unsigned int absdiff_asmnum1;


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
  absdiff_asmsrc1 = buffo;
  absdiff_asmsrc2 = lastimage;
  absdiff_asmdst = procbuf;
  absdiff_asmnum1 = geo->size;
  mmx_absdiff32();
  
  return(procbuf);
}


int kbd_input(SDL_keysym *keysym) { return 0; }
