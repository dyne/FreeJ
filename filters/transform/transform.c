/*
 * originally written by clifford smith <nullset@dookie.net>
 * further modifications by denis roio <jaromil@dyne.org>
 * 
 * TransForm.c: Performs positional translations on images
 * requires initialization of the table from other files so that
 * many plugins can be spawned out of it at link time
 *
 * for happy tweaking see in quad.c rand.c square.c ...
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL/SDL.h>

#include <freej.h>

static void *procbuf;
static int *Table;
static int *ypos;

/* function defined into the linked additional file
   builds the translation map */
extern void init_table(int *table, ScreenGeometry *sg);

/* function called when the Table value is -2
   computes a traslation realtime */
extern int livemap(int x, int y);

/* function called when a key is pressed
   computes changes for interactive commands */
extern int keypress(SDL_keysym *keysym);

static ScreenGeometry *geo;

int init(ScreenGeometry *sg) {
  int c;
  geo = sg;

  procbuf = malloc(geo->size);
  if(!procbuf) return 0;
  
  Table = malloc(geo->w * geo->h * sizeof(int));
  if(!Table) return 0;
  init_table(Table, geo);

  ypos = malloc(geo->h * sizeof(int));
  for(c=0;c<geo->h;c++) ypos[c] = c*geo->w;
  
  return 1;
}

int clean() {
  free(procbuf);
  free(Table);
  free(ypos);
  return 1;
}

void *process(void *buffo) {
  int x,y;
  int dest, value=0;
  Uint32 *src = (Uint32*)buffo, *dst = (Uint32*)procbuf;
  
  for(y=0;y<geo->h;y++)
    for(x=0;x<geo->w;x++) {
      dest = Table[x+ypos[y]];
      if(dest >= 0) value = *(Uint32*)(src+dest);
      else if(dest == -1) value = 0;
      else if(dest == -2) dest = livemap(x,y);
      *(Uint32*)(dst+x+ypos[y]) = value;
    }
  return(procbuf);
}

int kbd_input(SDL_keysym *keysym) {
  return keypress(keysym); 
}
