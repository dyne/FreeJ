/*  FreeJ
 *  (c) Copyright 2001 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Globals */

unsigned char *asmsrc1;
unsigned char *asmsrc2;
unsigned char *asmdst;
unsigned int asmnum1;
unsigned int asmnum2;
unsigned int asmnum3;
unsigned int asmnum4;

/* assembly routines */
extern void mmx_memcpy(void);
extern void mmx_blit(void);
extern void mmx_blit_add(void);
extern void mmx_blit_sub(void);
extern void mmx_blit_and(void);
extern void mmx_blit_or(void);
extern void asm_vline16(void);
extern void asm_hline16(void);
extern void asm_vline32(void);
extern void asm_hline32(void);
extern void asm_clearscr(void);

void mmxcopy(void *src1, void *dst, unsigned int size) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = size;

  mmx_memcpy();
}

#include <stdio.h>
void mmxblit(void *src1, void *dst, unsigned int width, unsigned int height, unsigned int pitch, unsigned int scr_pitch) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = height;
  asmnum2 = width;
  asmnum3 = pitch;
  asmnum4 = scr_pitch;
  mmx_blit();
}

void mmxblit_add(void *src1, void *dst, unsigned int height, unsigned int pitch, unsigned int scr_pitch) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = height;
  asmnum2 = pitch;
  asmnum3 = scr_pitch;

  mmx_blit_add();
}

void mmxblit_sub(void *src1, void *dst, unsigned int height, unsigned int pitch, unsigned int scr_pitch) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = height;
  asmnum2 = pitch;
  asmnum3 = scr_pitch;

  mmx_blit_sub();
}

void mmxblit_and(void *src1, void *dst, unsigned int height, unsigned int pitch, unsigned int scr_pitch) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = height;
  asmnum2 = pitch;
  asmnum3 = scr_pitch;

  mmx_blit_and();
}

void mmxblit_or(void *src1, void *dst, unsigned int height, unsigned int pitch, unsigned int scr_pitch) {
  asmsrc1 = src1;
  asmdst = dst;
  asmnum1 = height;
  asmnum2 = pitch;
  asmnum3 = scr_pitch;

  mmx_blit_or();
}
  
/* void *scr is points on desired coordinate offset on screen
   ( use Context::coords(x,y) if blitting directly on context )
   take care THEY DONT CLIP */
void vline(void *scr, unsigned int height, unsigned int pitch, unsigned int bpp) {
  asmdst = scr;
  asmnum1 = height;
  asmnum2 = pitch;

  switch(bpp) {
  case 16:
    asm_vline16();
    break;
  case 32:
    asm_vline32();
    break;
  }

}

void hline(void *scr, unsigned int width, unsigned int bpp) {
  asmdst = scr;
  asmnum1 = width;
  
  switch(bpp) {
  case 16:
    asm_hline16();
    break;
  case 32:
    asm_hline32();
    break;
  }

}

void clearscr(void *scr, unsigned int size) {
  asmdst = scr;
  asmnum1 = size;

  asm_clearscr();
}
