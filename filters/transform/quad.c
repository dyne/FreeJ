#include <math.h>
#include <string.h>
#include <freej.h>
#include <freej_plugin.h>

static char *name = "QuadTrans";
static char *author = "jaromil";
static char *info = "quaduplicated positional translation";
static int version = 1;

void init_table(int *table, ScreenGeometry *geo) {
  int x, y, tx, ty;
  
  for(y=0; y<geo->h; y++) {

    /* ==== CORE translation table formula
       Y AXIS - you can play with this ;) */
    
    ty = (y * 2) % (geo->h);

    /* =================================== */

    if(ty<0) ty = 0;
    if(ty>=geo->h) ty = geo->h - 1;

    for(x=0; x<geo->w; x++) {
      
      /* ==== CORE translation table formula
	 X AXIS - you can play with this ;) */
      
      tx = (x * 2) % (geo->w);
      
      /* ================================== */

      if(tx<0) tx = 0;
      if(tx>=geo->w) tx = geo->w - 1;

      table[x+y*geo->w] = ty*geo->w+tx;
    }
  }
}

int livemap(int x, int y) { return 0; }
