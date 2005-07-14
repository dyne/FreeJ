/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998,1999 Olivier Debon
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
///////////////////////////////////////////////////////////////
#ifndef _CXFORM_H_
#define _CXFORM_H_

struct Color {
    unsigned char red,green,blue,alpha;
    long		 pixel;
};

struct Cxform
{
	float aa; long ab;	// a is multiply factor, b is addition factor
	float ra; long rb;
	float ga; long gb;
	float ba; long bb;

	long	getRed(long v);
	long	getGreen(long v);
	long	getBlue(long v);
	long	getAlpha(long v);
	Color	getColor(Color color);

#ifdef DUMP
	void dump(BitStream *bs, int alpha = 0);
#endif
};

#endif /* _CXFORM_H_ */
