#include <freej.h>
#include <freej_plugin.h>



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>

/* setup some data to identify the plugin */
static char *name = "Blur"; /* do not assign a *name longer than 8 chars! */
static char *author = "jaromil"; 
static char *info = "Soft pixel blending";
static int version = 1; /* version is just an int (sophisticated isn't it?) */

/* save here screen geometry informations */
static ScreenGeometry *geo;

/* buffer where to copy the screen
   a pointer to it is being given back by process() */
static void *procbuf;

#define blend(a,b) ((((a) >> 1) & 0x7F7F7F7F) + (((b) >> 1) & 0x7F7F7F7F))

static unsigned int i;
static unsigned int *s, *d;

int init(ScreenGeometry *sg) {
  geo = sg;
  procbuf = malloc(geo->size);
  d = (unsigned int*)procbuf;
  return(1);
}

int clean() {
  free(procbuf);
  return(1);
}

void *process(void *buffo) {
  s = (unsigned int*)buffo;
  for (i = geo->w+1; i < geo->w*(geo->h-1)-1; i++)
    d[i] = blend( blend(s[i-1], s[i+1]),
		  blend(s[i+geo->w-1], s[i+geo->w+1]) );
  return procbuf;
}


int kbd_input(SDL_keysym *keysym) {
  return(0);
}
