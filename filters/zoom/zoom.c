#include <stdlib.h>
#include <SDL/SDL.h>

#include <freej.h>
#include <freej_plugin.h>

static char *name = "Zoom";
static char *author = "Lluis Gomez";
static char *info = "zomm into the screen 3 times looped";
static int version = 0;
static int c = 0;
static int cont = 0;
static int cont2 = 0;
static int i = 1;
static float ii = 1;


static int hheight, hwidth, stat, zoom;

static void *procbuf;
static ScreenGeometry *geo;
static int *ypos;

static void zoom_1(Uint32 *src, Uint32 *dest);

int init(ScreenGeometry *sg) {
  geo = sg;
  procbuf = malloc(geo->size);

  hheight = geo->h/2;
  hwidth = geo->w/2;
  zoom = 0;
  stat = 1;
  
  ypos = malloc(geo->h * sizeof(int));
  for(c=0;c<geo->h;c++) ypos[c] = c*geo->w;

  return(1);
}

int clean() {
  free(procbuf);
  free(ypos);
  return(1);
}

void *process(void *buffo) {
  switch(zoom) {
  case 0:
  default:
    zoom_1((Uint32*)buffo, (Uint32*)procbuf);
    break;
  }
  return(procbuf);
}



static void zoom_1(Uint32 *src, Uint32 *dest) {
	int x, y, tx, ty, hh, ww;
	
	i = (int)ii;
	hh = geo->h / i;
	ww = geo->w / i;

	for(y=0; y<hh; y++) {
		for(x=0; x<ww; x++) {
			tx = (geo->w/2)-(ww / 2)+x;
			ty = (geo->h/2)-(hh / 2)+y;

			for (cont=0; cont<i; cont++) 
			   for (cont2=0; cont2<i; cont2++) {
				dest[i*x-cont+ypos[i*y-cont2]] = src[tx+ypos[ty]] ;
			   }
		}
	}
	ii = ii + 0.1;
	if (ii>4) ii=1;
}

