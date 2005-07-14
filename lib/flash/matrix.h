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
#ifndef _MATRIX_H_
#define _MATRIX_H_

struct Matrix {
	float a,b,c,d;
	long tx,ty;
public:
	Matrix operator*(Matrix);
	Matrix invert();
	Matrix();

#ifdef DUMP
	void dump(BitStream *bs);
#endif

	inline
	long Matrix::getX(long x, long y)
	{
		return (long) (x*a+y*b+tx);
	};

	inline
	long Matrix::getY(long x, long y)
	{
		return (long) (x*c+y*d+ty);
	};

};

#endif /* _MATRIX_H_ */
