#include <freej.h>
#include <freej_plugin.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* setup some data to identify the plugin */
static char *name = "Null"; /* do not assign a *name longer than 8 chars! */
static char *author = "jaromil"; 
static char *info = "Null plugin";
static int version = 1; /* version is just an int (sophisticated isn't it?) */

/* save here screen geometry informations */
static ScreenGeometry *geo;

/* buffer where to copy the screen
   a pointer to it is being given back by process() */
static void *procbuf;

int init(ScreenGeometry *sg) {
  geo = sg;
  procbuf = malloc(geo->size);
  return(1);
}

int clean() {
  free(procbuf);
  return(1);
}

void *process(void *buffo) {
  memcpy(procbuf,buffo,geo->size);
  return(procbuf);
}

int kbd_input(SDL_keysym *keysym) {
  return(0);
}
