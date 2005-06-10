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
 *
 * "$Id$"
 *
 */

#include <screen.h>
#include <scale2x.h>
#include <scale3x.h>

ViewPort::ViewPort() {
	zoom          = 0.4;
	x_translation = 0;
	y_translation = 0;
	x_rotation    = 0;
	y_rotation    = 0;
}

ViewPort::~ViewPort() {
}

void ViewPort::scale2x(uint32_t *osrc, uint32_t *odst) {

      /* apply scale2x to screen */
    int c;
    uint32_t *src, *dst, dw;
    src = osrc;
    dst = odst;
    dw = w*2;

#if defined(__GNUC__) && defined(__i386__)
    scale2x_32_mmx(dst,dst+dw,
		   src,src,src+w,w);
#else
    scale2x_32_def(dst,dst+dw,
		   src,src,src+w,w);
#endif
    dst += dw<<1;
    src += w;
    for(c=0;c<h-2;c++) {
#if defined(__GNUC__) && defined(__i386__)      
      scale2x_32_mmx(dst,dst+dw,
		     src-w,src,src+w,w);
#else
      scale2x_32_def(dst,dst+dw,
		     src-w,src,src+w,w);
#endif
      dst += dw<<1;
      src += w;
    }
#if defined(__GNUC__) && defined(__i386__)
    scale2x_32_mmx(dst,dst+dw,
		   src-w,src,src,w);
    scale2x_mmx_emms();
#else
    scale2x_32_def(dst,dst+dw,
		   src-w,src,src,w);
#endif

}

void ViewPort::scale3x(uint32_t *osrc, uint32_t *odst) {

  /* apply scale3x to screen */
  int c;
  uint32_t *src, *dst, tw;
  src = osrc;
  dst = odst;
  tw = w*3;
  
  scale3x_32_def(dst,dst+tw,dst+tw+tw,
		 src,src,src+w,w);
  dst += tw*3;
  src += w;
  for(c=0;c<h-2;c++) {
    
    scale3x_32_def(dst,dst+tw,dst+tw+tw,
		   src-w,src,src+w,w);
    
    dst += tw*3;
    src += w;
  }
  
  scale3x_32_def(dst,dst+tw,dst+tw+tw,
		 src-w,src,src,w);

}

void ViewPort::set_zoom(float z) {
	zoom = z;
}
float ViewPort::get_zoom() {
	return zoom;
}
void ViewPort::set_rotation(float r) {
	rotation = r;
}
float ViewPort::get_rotation() {
	return rotation;
}
void ViewPort::set_x_rotation(float x) {
	x_rotation = x;
}
float ViewPort::get_x_rotation() {
	return x_rotation;
}
void ViewPort::set_y_rotation(float y) {
	y_rotation = y;
}
float ViewPort::get_y_rotation() {
	return y_rotation;
}
void ViewPort::set_y_translation(float y) {
	y_translation = y;
}
float ViewPort::get_y_translation() {
	return y_translation;
}
void ViewPort::set_x_translation(float x) {
	x_translation = x;
}
float ViewPort::get_x_translation() {
	return x_translation;
}
