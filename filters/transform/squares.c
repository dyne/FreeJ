#include <string.h>
#include <SDL/SDL.h>
#include <freej.h>
#include <freej_plugin.h>

static char *name = "SquareTrans";
static char *author = "Clifford Smith";
static char *info = "squared positional translation";
static int version = 2;

static int size = 16;
static int *extable;
static ScreenGeometry *exgeo;

void sqinit() {
  int x, y, tx, ty;
  
  for(y=0; y<exgeo->h; y++) {

    ty = y % size - size / 2;

    if((y/size)%2) ty = y - ty;
    else ty = y + ty;

    if(ty<0) ty = 0;
    if(ty>=exgeo->h) ty = exgeo->h - 1;

    for(x=0; x<exgeo->w; x++) {
      tx = x % size - size / 2;
      if((x/size)%2) tx = x - tx;
      else tx = x + tx;
      if(tx<0) tx = 0;
      if(tx>=exgeo->w) tx = exgeo->w - 1;
      extable[x+y*exgeo->w] = ty*exgeo->w+tx;
    }
  }
}

void init_table(int *table, ScreenGeometry *geo) {
  extable = table;
  exgeo = geo;
  sqinit();
}

int livemap(int x, int y) { return 0; }

int keypress(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_w:
    size = (size>15) ? 16 : size+1;
    break;
  case SDLK_q:
    size = (size<3) ? 2 : size-1;
    break;
  default:
    res = 0;
    break;
  }
  if(res) sqinit();
  return res;
}

