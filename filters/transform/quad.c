
#include <math.h>
#include <string.h>
#include <freej.h>

static char *name = "QuadTrans";
static char *author = "jaromil";
static char *info = "multiplied positional translation";
static int version = 2;
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

static ScreenGeometry *exgeo;
static int *extable;
static int size = 2;

void quadinit() {
  int x, y, tx, ty;
  
  for(y=0; y<exgeo->h; y++) {

    ty = (y * size) % (exgeo->h);

    if(ty<0) ty = 0;
    if(ty>=exgeo->h) ty = exgeo->h - 1;

    for(x=0; x<exgeo->w; x++) {
      
      tx = (x * size) % (exgeo->w);
      
      if(tx<0) tx = 0;
      if(tx>=exgeo->w) tx = exgeo->w - 1;

      extable[x+y*exgeo->w] = ty*exgeo->w+tx;
    }
  }
}

void init_table(int *table, ScreenGeometry *geo) {
  extable = table;
  exgeo = geo;
  quadinit();
}

int livemap(int x, int y) { return 0; }

int keypress(char key) {
  int res = 1;
  switch(key) {
  case 'w':
    size = (size>9) ? 10 : size+1;
    break;
  case 'q':
    size = (size<3) ? 2 : size-1;
    break;
  default:
    res = 0;
    break;
  }
  if(res) quadinit();
  return res;
}
