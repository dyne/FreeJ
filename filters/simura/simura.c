/*
 * Copyright (C) 2001 FUKUCHI Kentarou
 * ported to FreeJ by jaromil
 */

#include <stdlib.h>

#include <freej.h>

static char *name = "Simura";
static char *author = "Fukuchi Kentarou";
static char *info = "interactive multi mirror and colorizer";
static int version = 1;
char *getname() { return name; };
char *getauthor() { return author; };
char *getinfo() { return info; };
int getversion() { return version; };

static uint32_t colortable[26] = {
	0x000080, 0x0000e0, 0x0000ff,
	0x008000, 0x00e000, 0x00ff00,
	0x008080, 0x00e0e0, 0x00ffff,
	0x800000, 0xe00000, 0xff0000,
	0x800080, 0xe000e0, 0xff00ff,
	0x808000, 0xe0e000, 0xffff00,
	0x808080, 0xe0e0e0, 0xffffff,
	0x76ca0a, 0x3cafaa, 0x60a848, 
	0x504858, 0x89ba43
};

static int hheight, hwidth, stat, color, mirror;

static void *procbuf;
static ScreenGeometry *geo;

static void mirror_no(uint32_t *src, uint32_t *dest);
static void mirror_u(uint32_t *src, uint32_t *dest);
static void mirror_d(uint32_t *src, uint32_t *dest);
static void mirror_l(uint32_t *src, uint32_t *dest);
static void mirror_r(uint32_t *src, uint32_t *dest);
static void mirror_ul(uint32_t *src, uint32_t *dest);
static void mirror_ur(uint32_t *src, uint32_t *dest);
static void mirror_dl(uint32_t *src, uint32_t *dest);
static void mirror_dr(uint32_t *src, uint32_t *dest);

int init(ScreenGeometry *sg) {
  geo = sg;
  procbuf = malloc(geo->size);

  hheight = geo->h/2;
  hwidth = geo->w/2;
  color = mirror = 0;
  stat = 1;
  return(1);
}

int clean() {
  if(procbuf) free(procbuf);
  return(1);
}

void *process(void *buffo) {
  switch(mirror) {
  case 1:
    mirror_l((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 2:
    mirror_r((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 3:
    mirror_d((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 4:
    mirror_dl((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 5:
    mirror_dr((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 6:
    mirror_u((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 7:
    mirror_ul((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 8:
    mirror_ur((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  case 0:
  default:
    mirror_no((uint32_t*)buffo, (uint32_t*)procbuf);
    break;
  }
  return(procbuf);
}

int kbd_input(char key) {
  int res = 1;
  /*  if(keysym->mod & KMOD_CAPS)
      colors are removed temporarly
    switch(key) {
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
      color = colortable[key - 'a'];
      break;
    case 'z':
      color = 0;
      break;
  */
  switch(key) {
  case 'z':
    mirror = 0;
    break;
  case 'x': mirror = 1; break;
  case 'c': mirror = 2; break;
  case 'a': mirror = 3; break;
  case 's': mirror = 4; break;
  case 'd': mirror = 5; break;
  case 'q': mirror = 6; break;
  case 'w': mirror = 7; break;
  case 'e': mirror = 8; break;
  default:
    res = 0;
    break;
  }
  
  return(res);
}

static void mirror_no(uint32_t *src, uint32_t *dest) {
	unsigned int i;

	for(i=0; i<geo->size>>2; i++) {
		dest[i] = src[i] ^ color;
	}
}

static void mirror_u(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=0; y<hheight; y++) {
		for(x=0; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_d(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=hheight; y<(geo->h); y++) {
		for(x=0; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_l(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=0; y<(geo->h); y++) {
		for(x=0; x<hwidth; x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_r(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=0; y<(geo->h); y++) {
		for(x=hwidth; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_ul(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=0; y<hheight; y++) {
		for(x=0; x<hwidth; x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_ur(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=0; y<hheight; y++) {
		for(x=hwidth; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_dl(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=hheight; y<(geo->h); y++) {
		for(x=0; x<hwidth; x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_dr(uint32_t *src, uint32_t *dest) {
	int x, y;

	for(y=hheight; y<(geo->h); y++) {
		for(x=hwidth; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}
