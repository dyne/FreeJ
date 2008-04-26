/*
 * mlt_frame.h -- interface for all frame classes
 * Copyright (C) 2003-2004 Ushodaya Enterprises Limited
 * Author: Charles Yates <charles.yates@pandora.be>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _MLT_FRAME_H_
#define _MLT_FRAME_H_

/* convenience functions */
extern int mlt_convert_yuv422_to_rgb24a( uint8_t *yuv, uint8_t *rgba, unsigned int total );
extern int mlt_convert_rgb24a_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha );
extern int mlt_convert_rgb24_to_yuv422( uint8_t *rgb, int width, int height, int stride, uint8_t *yuv );
extern int mlt_convert_bgr24a_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha );
extern int mlt_convert_argb_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha );
extern int mlt_convert_bgr24_to_yuv422( uint8_t *rgb, int width, int height, int stride, uint8_t *yuv );
extern int mlt_convert_yuv420p_to_yuv422( uint8_t *yuv420p, int width, int height, int stride, uint8_t *yuv );

/* this macro scales rgb into the yuv gamut, y is scaled by 219/255 and uv by 224/255 */
#define RGB2YUV(r, g, b, y, u, v)\
  y = ((263*r + 516*g + 100*b) >> 10) + 16;\
  u = ((-152*r - 298*g + 450*b) >> 10) + 128;\
  v = ((450*r - 377*g - 73*b) >> 10) + 128;

/* this macro assumes the user has already scaled their rgb down into the broadcast limits */
#define RGB2YUV_UNSCALED(r, g, b, y, u, v)\
  y = (299*r + 587*g + 114*b) >> 10;\
  u = ((-169*r - 331*g + 500*b) >> 10) + 128;\
  v = ((500*r - 419*g - 81*b) >> 10) + 128;\
  y = y < 16 ? 16 : y;\
  u = u < 16 ? 16 : u;\
  v = v < 16 ? 16 : v;\
  y = y > 235 ? 235 : y;\
  u = u > 240 ? 240 : u;\
  v = v > 240 ? 240 : v

#define YUV2RGB( y, u, v, r, g, b ) \
  r = ((1192 * ( y - 16 ) + 1634 * ( v - 128 ) ) >> 10 ); \
  g = ((1192 * ( y - 16 ) - 832 * ( v - 128 ) - 400 * ( u - 128 ) ) >> 10 ); \
  b = ((1192 * ( y - 16 ) + 2066 * ( u - 128 ) ) >> 10 ); \
  r = r < 0 ? 0 : r > 255 ? 255 : r; \
  g = g < 0 ? 0 : g > 255 ? 255 : g; \
  b = b < 0 ? 0 : b > 255 ? 255 : b;

#endif
