#include <string.h>
#include <freej.h>
#include <freej_plugin.h>

static char *name = "SquareTrans";
static char *author = "Clifford Smith";
static char *info = "squared positional translation";
static int bpp = 4;
static int version = 1;

void init_table(int *table, ScreenGeometry *geo) {
  const int size = 16;
  int x, y, tx, ty;
  
  for(y=0; y<geo->h; y++) {

    ty = y % size - size / 2;

    if((y/size)%2) ty = y - ty;
    else ty = y + ty;

    if(ty<0) ty = 0;
    if(ty>=geo->h) ty = geo->h - 1;

    for(x=0; x<geo->w; x++) {
      tx = x % size - size / 2;
      if((x/size)%2) tx = x - tx;
      else tx = x + tx;
      if(tx<0) tx = 0;
      if(tx>=geo->w) tx = geo->w - 1;
      table[x+y*geo->w] = ty*geo->w+tx;
    }
  }
}

int livemap(int x, int y) { return 0; }
