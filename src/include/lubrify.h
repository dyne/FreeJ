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

#ifndef __LUBRIFY_H__
#define __LUBRIFY_H__

#include <stdint.h>

extern "C" {
  void mmxcopy(void *src1, void *dst, unsigned int size);
  void mmxblit(void *src1, void *dst, unsigned int height, unsigned int pitch, unsigned int scr_pitch);
  void mmxdiff8(void *src1, void *src2, void *dst, unsigned int size);
  void mmxdiff16(void *src1, void *src2, void *dst, unsigned int size);
  void vline(void *scr, unsigned int height, unsigned int pitch, unsigned int bpp);
  void hline(void *scr, unsigned int width, unsigned int bpp);
  void clearscr(void *scr, unsigned int size);
}

#endif
