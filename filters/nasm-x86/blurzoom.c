
#include <freej.h>
#include <freej_plugin.h>

#include <SDL/SDL.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

#define USE_NASM 1
#define COLORS 32
#define RATIO 0.95

static char *name = "Blurzoom";
static char *author = "Fukuchi Kentarou";
static char *info = "motion-enlightment effect";
static int version = 1;

static ScreenGeometry *geo;

static void *procbuf;

static unsigned char *diff;
static uint32_t *background;
static int y_threshold;

extern void blurzoomcore();
unsigned char *blurzoombuf;
int *blurzoomx;
int *blurzoomy;
int buf_width_blocks;
int buf_width;
int buf_height;
int buf_area;
int buf_margin_right;
int buf_margin_left;

static uint32_t palette[COLORS];
static int mode = 0; /* 0=normal/1=strobe/2=strobe2/3=trigger */
static int snapTime = 0;
static int snapInterval = 3;
static uint32_t *snapframe;

#define VIDEO_HWIDTH (buf_width/2)
#define VIDEO_HHEIGHT (buf_height/2)

static void setTable();
static void makePalette();
static unsigned char *diff_bgsubtract_update_y(uint32_t *src);

int init(ScreenGeometry *sg) {
  geo = sg;

  procbuf = malloc(geo->size);

  buf_width_blocks = (geo->w / 32);
  if(buf_width_blocks > 255) {
    return 0;
  }
  buf_width = buf_width_blocks * 32;
  buf_height = geo->h;
  buf_area = buf_width * buf_height;
  buf_margin_left = (geo->w - buf_width)/2;
  buf_margin_right = geo->w - buf_width - buf_margin_left;

  blurzoombuf = (unsigned char *)malloc(buf_area*2);
  if(!blurzoombuf) return (0);

  blurzoomx = (int *)malloc(buf_width*sizeof(int));
  blurzoomy = (int *)malloc(buf_height*sizeof(int));
  if(blurzoomx == NULL || blurzoomy == NULL) {
    return 0;
  }

  setTable();
  makePalette();

  bzero(blurzoombuf, buf_area*2);
  snapframe = (uint32_t *)malloc((geo->w*geo->h)*sizeof(uint32_t));
  if(!snapframe) return 0;

  background = (uint32_t *)malloc((geo->w*geo->h)*sizeof(uint32_t));
  diff = (unsigned char *)malloc((geo->w*geo->h)*sizeof(unsigned char));
  y_threshold = 40 * 7;
  
  return 1;
}

void *process(void *buffo) {
  int x, y;
  uint32_t a, b;
  uint32_t *src, *dest;
  unsigned char *local_diff, *p;
  
  src = (uint32_t *)buffo;
  
  if(mode != 2 || snapTime <= 0) {
    local_diff = diff_bgsubtract_update_y(src);
    if(mode == 0 || snapTime <= 0) {
      local_diff += buf_margin_left;
      p = blurzoombuf;
      for(y=0; y<buf_height; y++) {
	for(x=0; x<buf_width; x++) {
	  p[x] |= local_diff[x] >> 3;
	}
	local_diff += geo->w;
	p += buf_width;
      }
      if(mode == 1 || mode == 2) {
	memcpy(snapframe, src, geo->size);
      }
    }
  }
  blurzoomcore();
  
  dest = (uint32_t *)procbuf;

  
  if(mode == 1 || mode == 2) {
    src = snapframe;
  }
  p = blurzoombuf;
  for(y=0; y<geo->h; y++) {
    for(x=0; x<buf_margin_left; x++) {
      *dest++ = *src++;
    }
    for(x=0; x<buf_width; x++) {
      a = *src++ & 0xfefeff;
      b = palette[*p++];
      a += b;
      b = a & 0x1010100;
      *dest++ = a | (b - (b >> 8));
    }
    for(x=0; x<buf_margin_right; x++) {
      *dest++ = *src++;
    }
  }
  if(mode == 1 || mode == 2) {
    snapTime--;
    if(snapTime < 0) {
      snapTime = snapInterval;
    }
  }
  
  return procbuf;
}

int clean() {
  free(procbuf);
  free(blurzoombuf);
  free(blurzoomx);
  free(blurzoomy);
  free(snapframe);
  free(background);
  free(diff);
  return 1;
}

int kbd_input(SDL_keysym *keysym) {
  int res = 1;
  switch(keysym->sym) {
  case SDLK_KP1:
  case SDLK_KP2:
  case SDLK_KP3:
  case SDLK_KP4:
    mode = keysym->sym - SDLK_KP1;
    if(mode == 3) snapTime = 1;
    else snapTime = 0;
    break;

  case SDLK_a:
    if(mode == 3) snapTime = 1;
    break;
    
  case SDLK_s:
    if(mode == 3) snapTime = 0;
    break;

  case SDLK_w:
    snapInterval++;
    break;
      
  case SDLK_q:
    snapInterval--;
    if(snapInterval < 1) {
      snapInterval = 1;
    }
    break;
    
  default: 
    res = 0;
    break;
  }

  return res;
}


/* this table assumes that video_width is times of 32 */
static void setTable() {
  unsigned int bits;
  int x, y, tx, ty, xx;
  int ptr, prevptr;
  
  prevptr = (int)(0.5+RATIO*(-VIDEO_HWIDTH)+VIDEO_HWIDTH);
	for(xx=0; xx<(buf_width_blocks); xx++){
		bits = 0;
		for(x=0; x<32; x++){
			ptr= (int)(0.5+RATIO*(xx*32+x-VIDEO_HWIDTH)+VIDEO_HWIDTH);
#ifdef USE_NASM
			bits = bits<<1;
			if(ptr != prevptr)
				bits |= 1;
#else
			bits = bits>>1;
			if(ptr != prevptr)
				bits |= 0x80000000;
#endif /* USE_NASM */
			prevptr = ptr;
		}
		blurzoomx[xx] = bits;
	}

	ty = (int)(0.5+RATIO*(-VIDEO_HHEIGHT)+VIDEO_HHEIGHT);
	tx = (int)(0.5+RATIO*(-VIDEO_HWIDTH)+VIDEO_HWIDTH);
	xx=(int)(0.5+RATIO*(buf_width-1-VIDEO_HWIDTH)+VIDEO_HWIDTH);
	blurzoomy[0] = ty * buf_width + tx;
	prevptr = ty * buf_width + xx;
	for(y=1; y<buf_height; y++){
		ty = (int)(0.5+RATIO*(y-VIDEO_HHEIGHT)+VIDEO_HHEIGHT);
		blurzoomy[y] = ty * buf_width + tx - prevptr;
		prevptr = ty * buf_width + xx;
	}
}		

static void makePalette()
{
	int i;

#define DELTA (255/(COLORS/2-1))

	for(i=0; i<COLORS/2; i++) {
		palette[i] = i*DELTA;
	}
	for(i=0; i<COLORS/2; i++) {
		palette[i+COLORS/2] = 255 | (i*DELTA)<<16 | (i*DELTA)<<8;
	}
	for(i=0; i<COLORS; i++) {
		palette[i] = palette[i] & 0xfefeff;
	}
}

/* The origin of subtraction function is;
 * diff(src, dest) = (abs(src - dest) > threshold) ? 0xff : 0;
 *
 * This functions is transformed to;
 * (threshold > (src - dest) > -threshold) ? 0 : 0xff;
 *
 * (v + threshold)>>24 is 0xff when v is less than -threshold.
 * (v - threshold)>>24 is 0xff when v is less than threshold.
 * So, ((v + threshold)>>24) | ((threshold - v)>>24) will become 0xff when
 * abs(src - dest) > threshold.
 */

/* Background image is refreshed every frame */
static unsigned char *diff_bgsubtract_update_y(uint32_t *src) {
  int i;
  int R, G, B;
  uint32_t *p;
  short *q;
  unsigned char *r;
  int v;
  
  p = src;
  q = (short *)background;
  r = diff;
  for(i=0; i<(geo->w*geo->h); i++) {
    R = ((*p)&0xff0000)>>(16-1);
    G = ((*p)&0xff00)>>(8-2);
    B = (*p)&0xff;
    v = (R + G + B) - (int)(*q);
    *q = (short)(R + G + B);
    *r = ((v + y_threshold)>>24) | ((y_threshold - v)>>24);
    
    p++;
    q++;
    r++;
  }
  return diff;
}
