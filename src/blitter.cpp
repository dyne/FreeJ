
#include <layer.h>
#include <blitter.h>
#include <context.h>
#include <imagefilter.h>

#include <jutils.h>
#include <config.h>

/* blit functions, prototype is:
   void (*blit_f)(void *src, void *dst, int bytes) */

static inline void memcpy_blit(void *src, void *dst, int bytes) {
  memcpy(dst,src,bytes);
}

static inline void accel_memcpy_blit(void *src, void *dst, int bytes) {
  jmemcpy(dst,src,bytes);
}

static inline void schiffler_add(void *src, void *dst, int bytes) {
  SDL_imageFilterAdd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_sub(void *src, void *dst, int bytes) {
  SDL_imageFilterSub((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_mean(void *src, void *dst, int bytes) {
  SDL_imageFilterMean((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_absdiff(void *src, void *dst, int bytes) {
  SDL_imageFilterAbsDiff((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_mult(void *src, void *dst, int bytes) {
  SDL_imageFilterMult((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_multnor(void *src, void *dst, int bytes) {
  SDL_imageFilterMultNor((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_div(void *src, void *dst, int bytes) {
  SDL_imageFilterDiv((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_multdiv2(void *src, void *dst, int bytes) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_multdiv4(void *src, void *dst, int bytes) {
  SDL_imageFilterMultDivby2((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_and(void *src, void *dst, int bytes) {
  SDL_imageFilterBitAnd((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_or(void *src, void *dst, int bytes) {
  SDL_imageFilterBitOr((unsigned char*)src,(unsigned char*)dst,(unsigned char*)dst,bytes);
}

static inline void schiffler_neg(void *src, void *dst, int bytes) {
  SDL_imageFilterBitNegation((unsigned char*)src,(unsigned char*)dst,bytes);
}

Blit::Blit() :Entry() { }

Blit::~Blit() { }

Blitter::Blitter() {
  x_scale = 0.0;
  y_scale = 0.0;
  rotation = 0.0;
}

Blitter::~Blitter() {

}

bool Blitter::init(Layer *lay) {
  layer = lay;
  func("blitter initialized for layer %s",lay->name);

  /* TODO fill up linklist of blits */
  Blit *b;

  b = new Blit(); sprintf(b->name,"MEMCPY");
  sprintf(b->desc,"vanilla glibc memcpy");
  b->fun = memcpy_blit; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"AMEMCPY");
  sprintf(b->desc,"cpu accelerated memcpy");
  b->fun = accel_memcpy_blit; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"ADD");
  sprintf(b->desc,"bytewise addition");
  b->fun = schiffler_add; blitlist.append(b);
  
  b = new Blit(); sprintf(b->name,"SUB");
  sprintf(b->desc,"bytewise subtraction");
  b->fun = schiffler_sub; blitlist.append(b);
  
  b = new Blit(); sprintf(b->name,"MEAN");
  sprintf(b->desc,"bytewise mean");
  b->fun = schiffler_add; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"ABSDIFF");
  sprintf(b->desc,"absolute difference");
  b->fun = schiffler_absdiff; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"MULT");
  sprintf(b->desc,"multiplication");
  b->fun = schiffler_mult; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"MULTNOR");
  sprintf(b->desc,"normalized multiplication");
  b->fun = schiffler_multnor; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"DIV");
  sprintf(b->desc,"division");
  b->fun = schiffler_div; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"MULTDIV2");
  sprintf(b->desc,"multiplication and division by 2");
  b->fun = schiffler_multdiv2; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"MULTDIV4");
  sprintf(b->desc,"multiplication and division by 4");
  b->fun = schiffler_multdiv4; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"AND");
  sprintf(b->desc,"bitwise and");
  b->fun = schiffler_and; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"OR");
  sprintf(b->desc,"bitwise or");
  b->fun = schiffler_or; blitlist.append(b);

  b = new Blit(); sprintf(b->name,"NEG");
  sprintf(b->desc,"bitwise negation");
  b->fun = schiffler_neg; blitlist.append(b);

  // SET DEFAULT
  set_blit("amemcpy");

  return true;
}

void Blitter::blit() {
  scr = pscr = (uint32_t*)blit_coords;
  off = poff = (uint32_t*)layer->offset + blit_offset;

  for(c=blit_height;c>0;c--) {
    (*blit_function)((void*)off,(void*)scr,blit_pitch);
    off = poff = poff + layer->geo.w;
    scr = pscr = pscr + layer->freej->screen->w;
  }
}



bool Blitter::set_blit(char *name) {
  Blit *b = (Blit*)blitlist.search(name);

  if(!b) {
    error("blit %s not found",name);
    return false;
  }
  
  // found the matching name!
  blit_function = b->fun;
  blitlist.sel(0);
  b->sel(true);
  act("blit %s selected for layer %s",
      b->name,layer->name);
  return true;
}

bool Blitter::set_value(int val) {
  Blit *b = (Blit*)blitlist.selected();
  if(!b) {
    error("no blit selected on layer %s",layer->name);
    return false;
  }
  b->param = val;
  act("blit %s set to %i",b->name,val);
  return true;
}

bool Blitter::set_kernel(short *krn) {
  notice("Blitter::set_kernel : TODO convolution on blits");
  return false;
}

void Blitter::crop(ViewPort *screen) {
  if(!screen)
    screen = layer->freej->screen;

  /* crop for the SDL blit */
  rect.x = -(layer->geo.x);
  rect.y = -(layer->geo.y);
  rect.w = screen->w;
  rect.h = screen->h;

  /* crop for our own blit */
  uint32_t blit_xoff=0;
  uint32_t blit_yoff=0;
  
  blit_x = layer->geo.x;
  blit_y = layer->geo.y;
  blit_width = layer->geo.w;
  blit_height = layer->geo.h;
  blit_xoff = 0;
  blit_yoff = 0;

  //  func("CROP x[%i] y[%i] w[%i] h[%i] on screen w[%i] h[%i]",
  //       layer->geo.x,layer->geo.y,layer->geo.w,layer->geo.h,w,h);

  /* left bound 
     affects x-offset and width */
  if(layer->geo.x<0) {
    blit_xoff = (-layer->geo.x);
    blit_x = 1;

    if(blit_xoff>layer->geo.w) {
      func("layer out of screen to the left");
      layer->hidden = true; /* out of the screen */
      layer->geo.x = -(layer->geo.w+1); /* don't let it go far */      
    } else {
      layer->hidden = false;
      blit_width -= blit_xoff;
    }
  }

  /* right bound
     affects width */
  if((layer->geo.x+layer->geo.w)>screen->w) {
    if(layer->geo.x>screen->w) { /* out of screen */
      func("layer out of screen to the right");
      layer->hidden = true; 
      layer->geo.x = screen->w+1; /* don't let it go far */
    } else {
      layer->hidden = false;
      blit_width -= (layer->geo.w - (screen->w - layer->geo.x));
    }
  }

  /* upper bound
     affects y-offset and height */
  if(layer->geo.y<0) {
    blit_yoff = (-layer->geo.y);
    blit_y = 1;

    if(blit_yoff>layer->geo.h) {
      func("layer out of screen up");
      layer->hidden = true; /* out of the screen */
      layer->geo.y = -(layer->geo.h+1); /* don't let it go far */      
    } else {
      layer->hidden = false;
      blit_height -= blit_yoff;
    }
  }

  /* lower bound
     affects height */
  if((layer->geo.y+layer->geo.h) >screen->h) {
    if(layer->geo.y >screen->h) { /* out of screen */
      func("layer out of screen down");
      layer->hidden = true; 
      layer->geo.y = screen->h+1; /* don't let it go far */
    } else {
      layer->hidden = false;
      blit_height -= (layer->geo.h - (screen->h - layer->geo.y));
    }
  }
  
  blit_coords = (uint32_t*)screen->coords(blit_x,blit_y);

  blit_offset = (blit_xoff<<2) + (blit_yoff*layer->geo.pitch);

  blit_pitch = blit_width * (layer->geo.bpp>>3);

  func("CROP x[%i] y[%i] w[%i] h[%i] xoff[%i] yoff[%i]",
       blit_x, blit_y, blit_width, blit_height, blit_xoff, blit_yoff);
}


