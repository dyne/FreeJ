#include <stdlib.h>
#include <SDL/SDL.h>

#include <freej.h>
#include <freej_plugin.h>

static char *name = "Zoom";
static char *author = "Lluis Gomez";
static char *info = "zomm into the screen 3 times looped";
static int version = 0;
static int cont = 0;
static int cont2 = 0;
static int ww[3], hh[3], c;

static void *procbuf;
static ScreenGeometry *geo;
static int *ypos;

static void zoom_1(Uint32 *src, Uint32 *dest);

int init(ScreenGeometry *sg) {

  geo = sg;
  procbuf = malloc(geo->size);

  //  hheight = geo->h/2;
  //  hwidth = geo->w/2;

  for(c=1;c<5;c++) {
    hh[c-1] = geo->h / c;
    ww[c-1] = geo->w / c;
  }

  ypos = malloc(geo->h * sizeof(int));
  for(c=0;c<geo->h;c++) ypos[c] = c*geo->w;

  c=0;

  return(1);
}

int clean() {
  free(procbuf);
  free(ypos);
  return(1);
}

void *process(void *buffo) {
  c++; if(c>3) c=0;
  if(!c) return buffo;

  zoom_1((Uint32*)buffo, (Uint32*)procbuf);
  
  return(procbuf);
}



static void zoom_1(Uint32 *src, Uint32 *dest) {
  int x, y, tx, ty;
  
  for(y=0; y<hh[c]; y++) {
    for(x=0; x<ww[c]; x++) {
      tx = (geo->w/2)-(ww[c]/2)+x;
      ty = (geo->h/2)-(hh[c]/2)+y;
      
      for (cont=0; cont<c; cont++) 
	for (cont2=0; cont2<c; cont2++) {
	  dest[c*x-cont+ypos[c*y-cont2]] = src[tx+ypos[ty]] ;
	}
    }
  }


}

