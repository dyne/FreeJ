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

#include "matrix.h"

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

Matrix::Matrix()
{
	a = 1.0;
	d = 1.0;
	b = c = 0.0;
	tx = ty = 0;
}

Matrix Matrix::operator*(Matrix m)
{
	Matrix mat;

	mat.a = this->a * m.a + this->b * m.c;
	mat.b = this->a * m.b + this->b * m.d;
	mat.c = this->c * m.a + this->d * m.c;
	mat.d = this->c * m.b + this->d * m.d;

	mat.tx = this->getX(m.tx,m.ty);
	mat.ty = this->getY(m.tx,m.ty);

	return mat;
}

Matrix Matrix::invert()
{
	Matrix mat;
	float det;

	det = a*d-b*c;

	mat.a  = d/det;
	mat.b  = -b/det;
	mat.c  = -c/det;
	mat.d  = a/det;

	mat.tx = - (long)(mat.a * tx + mat.b * ty);
	mat.ty = - (long)(mat.c * tx + mat.d * ty);

	return mat;
}
