#include <stdlib.h>
#include <SDL/SDL.h>

#include <freej.h>
#include <freej_plugin.h>

static char *name = "Simura";
static char *author = "Fukuchi Kentarou, jaromil";
static char *info = "interactive multi mirror and colorizer";
static int version = 1;

static Uint32 colortable[26] = {
	0x000080, 0x0000e0, 0x0000ff,
	0x008000, 0x00e000, 0x00ff00,
	0x008080, 0x00e0e0, 0x00ffff,
	0x800000, 0xe00000, 0xff0000,
	0x800080, 0xe000e0, 0xff00ff,
	0x808000, 0xe0e000, 0xffff00,
	0x808080, 0xe0e0e0, 0xffffff,
	0x76ca0a, 0x3cafaa, 0x60a848, 0x504858, 0x89ba43
};

static int hheight, hwidth, stat, color, mirror;

static void *procbuf;
static ScreenGeometry *geo;

static void mirror_no(Uint32 *src, Uint32 *dest);
static void mirror_u(Uint32 *src, Uint32 *dest);
static void mirror_d(Uint32 *src, Uint32 *dest);
static void mirror_l(Uint32 *src, Uint32 *dest);
static void mirror_r(Uint32 *src, Uint32 *dest);
static void mirror_ul(Uint32 *src, Uint32 *dest);
static void mirror_ur(Uint32 *src, Uint32 *dest);
static void mirror_dl(Uint32 *src, Uint32 *dest);
static void mirror_dr(Uint32 *src, Uint32 *dest);

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
    mirror_l((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 2:
    mirror_r((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 3:
    mirror_d((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 4:
    mirror_dl((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 5:
    mirror_dr((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 6:
    mirror_u((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 7:
    mirror_ul((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 8:
    mirror_ur((Uint32*)buffo, (Uint32*)procbuf);
    break;
  case 0:
  default:
    mirror_no((Uint32*)buffo, (Uint32*)procbuf);
    break;
  }
  return(procbuf);
}

int kbd_input(SDL_keysym *keysym) {
  int res = 1;
  if(keysym->mod==KMOD_CAPS)
    switch(keysym->sym) {
    case SDLK_a:
    case SDLK_b:
    case SDLK_c:
    case SDLK_d:
    case SDLK_e:
    case SDLK_f:
    case SDLK_g:
    case SDLK_h:
    case SDLK_i:
    case SDLK_j:
    case SDLK_k:
    case SDLK_l:
    case SDLK_m:
    case SDLK_n:
    case SDLK_o:
    case SDLK_p:
    case SDLK_q:
    case SDLK_r:
    case SDLK_s:
    case SDLK_t:
    case SDLK_u:
    case SDLK_v:
    case SDLK_w:
    case SDLK_x:
    case SDLK_y:
    case SDLK_z:
      color = colortable[keysym->sym - SDLK_a];
      break;
    case SDLK_BACKSPACE:
      color = 0;
      break;
    default:
      res = 0;
      break;
    }
  else
    switch(keysym->sym) {
    case SDLK_KP0:
      mirror = 0;
      break;
    case SDLK_KP1:
    case SDLK_KP2:
    case SDLK_KP3:
    case SDLK_KP4:
    case SDLK_KP5:
    case SDLK_KP6:
    case SDLK_KP7:
    case SDLK_KP8:
    case SDLK_KP9:
      mirror = keysym->sym - SDLK_KP1;
      break;
    default:
      res = 0;
    }
  
  return(res);
}

static void mirror_no(Uint32 *src, Uint32 *dest) {
	unsigned int i;

	for(i=0; i<geo->size>>2; i++) {
		dest[i] = src[i] ^ color;
	}
}

static void mirror_u(Uint32 *src, Uint32 *dest) {
	int x, y;

	for(y=0; y<hheight; y++) {
		for(x=0; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_d(Uint32 *src, Uint32 *dest) {
	int x, y;

	for(y=hheight; y<(geo->h); y++) {
		for(x=0; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[((geo->h)-y-1)*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_l(Uint32 *src, Uint32 *dest) {
	int x, y;

	for(y=0; y<(geo->h); y++) {
		for(x=0; x<hwidth; x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_r(Uint32 *src, Uint32 *dest) {
	int x, y;

	for(y=0; y<(geo->h); y++) {
		for(x=hwidth; x<(geo->w); x++) {
			dest[y*(geo->w)+x] = src[y*(geo->w)+x] ^ color;
			dest[y*(geo->w)+((geo->w)-x-1)] = src[y*(geo->w)+x] ^ color;
		}
	}
}

static void mirror_ul(Uint32 *src, Uint32 *dest) {
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

static void mirror_ur(Uint32 *src, Uint32 *dest) {
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

static void mirror_dl(Uint32 *src, Uint32 *dest) {
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

static void mirror_dr(Uint32 *src, Uint32 *dest) {
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
