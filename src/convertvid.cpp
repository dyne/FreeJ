/*
 * mlt_frame.c -- interface for all frame classes
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <inttypes.h>

#include <convertvid.h>

/***** convenience functions *****/

int mlt_convert_yuv422_to_rgb24a( uint8_t *yuv, uint8_t *rgba, unsigned int total )
{
	int ret = 0;
	int yy, uu, vv;
      	int r,g,b;
	total /= 2;
	while (total--) 
	{
		yy = yuv[0];
		uu = yuv[1];
		vv = yuv[3];
		YUV2RGB(yy, uu, vv, r, g, b);
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
		rgba[3] = 255;
		yy = yuv[2];
		YUV2RGB(yy, uu, vv, r, g, b);
		rgba[4] = r;
		rgba[5] = g;
		rgba[6] = b;
		rgba[7] = 255;
		yuv += 4;
		rgba += 8;
	}
	return ret;
}

int mlt_convert_rgb24a_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha )
{
	int ret = 0;
	register int y0, y1, u0, u1, v0, v1;
	register int r, g, b;
	register uint8_t *d = yuv;
	register int i, j;

	if ( alpha )
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			r = *s++;
			g = *s++;
			b = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	else
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			r = *s++;
			g = *s++;
			b = *s++;
			s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}

	return ret;
}

int mlt_convert_rgb24_to_yuv422( uint8_t *rgb, int width, int height, int stride, uint8_t *yuv )
{
	int ret = 0;
	register int y0, y1, u0, u1, v0, v1;
	register int r, g, b;
	register uint8_t *d = yuv;
	register int i, j;

	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgb + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	return ret;
}

int mlt_convert_bgr24a_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha )
{
	int ret = 0;
	register int y0, y1, u0, u1, v0, v1;
	register int r, g, b;
	register uint8_t *d = yuv;
	register int i, j;

	if ( alpha )
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			b = *s++;
			g = *s++;
			r = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			*alpha++ = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	else
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			b = *s++;
			g = *s++;
			r = *s++;
			s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	return ret;
}

int mlt_convert_bgr24_to_yuv422( uint8_t *rgb, int width, int height, int stride, uint8_t *yuv )
{
	int ret = 0;
	register int y0, y1, u0, u1, v0, v1;
	register int r, g, b;
	register uint8_t *d = yuv;
	register int i, j;

	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgb + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			b = *s++;
			g = *s++;
			r = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			b = *s++;
			g = *s++;
			r = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	return ret;
}

int mlt_convert_argb_to_yuv422( uint8_t *rgba, int width, int height, int stride, uint8_t *yuv, uint8_t *alpha )
{
	int ret = 0;
	register int y0, y1, u0, u1, v0, v1;
	register int r, g, b;
	register uint8_t *d = yuv;
	register int i, j;

	if ( alpha )
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			*alpha++ = *s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*alpha++ = *s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			*alpha++ = *s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	else
	for ( i = 0; i < height; i++ )
	{
		register uint8_t *s = rgba + ( stride * i );
		for ( j = 0; j < ( width / 2 ); j++ )
		{
			s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y1, u1 , v1);
			*d++ = y0;
			*d++ = (u0+u1) >> 1;
			*d++ = y1;
			*d++ = (v0+v1) >> 1;
		}
		if ( width % 2 )
		{
			s++;
			r = *s++;
			g = *s++;
			b = *s++;
			RGB2YUV (r, g, b, y0, u0 , v0);
			*d++ = y0;
			*d++ = u0;
		}
	}
	return ret;
}

int mlt_convert_yuv420p_to_yuv422( uint8_t *yuv420p, int width, int height, int stride, uint8_t *yuv )
{
	int ret = 0;
	register int i, j;

	int half = width >> 1;

	uint8_t *Y = yuv420p;
	uint8_t *U = Y + width * height;
	uint8_t *V = U + width * height / 4;

	register uint8_t *d = yuv;

	for ( i = 0; i < height; i++ )
	{
		register uint8_t *u = U + ( i / 2 ) * ( half );
		register uint8_t *v = V + ( i / 2 ) * ( half );

		for ( j = 0; j < half; j++ )
		{
			*d ++ = *Y ++;
			*d ++ = *u ++;
			*d ++ = *Y ++;
			*d ++ = *v ++;
		}
	}
	return ret;
}
