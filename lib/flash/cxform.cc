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
//  Author : Olivier Debon  <odebon@club-internet.fr>
//  

#include "swf.h"

#ifdef RCSID
static char *rcsid = "$Id";
#endif

long
Cxform::getRed(long v) {
	long val;

	val = (long)(ra*v+rb);
	if (val > 255) val = 255;
	else if (val < 0) val = 0;
	return val;
}

long
Cxform::getGreen(long v) {
	long val;

	val = (long)(ga*v+gb);
	if (val > 255) val = 255;
	else if (val < 0) val = 0;
	return val;
}

long
Cxform::getBlue(long v) {
	long val;

	val = (long)(ba*v+bb);
	if (val > 255) val = 255;
	else if (val < 0) val = 0;
	return val;
}

long
Cxform::getAlpha(long v) {
	long val;

	val = (long)(aa*v+ab);
	if (val > 255) val = 255;
	else if (val < 0) val = 0;
	return val;
}

Color
Cxform::getColor(Color color) {
	Color newColor;

	newColor.red = getRed(color.red);
	newColor.green = getGreen(color.green);
	newColor.blue = getBlue(color.blue);
	newColor.alpha = getAlpha(color.alpha);

	return newColor;
}
