/////////////////////////////////////////////////////////////
// Flash Plugin and Player
// Copyright (C) 1998 Olivier Debon
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
static char *rcsid = "$Id$";
#endif

SwfFont::SwfFont(long id) : Character(FontType, id)
{
	glyphs = 0;
	nbGlyphs = 0;
        name = NULL;
	setFontName("Unknown");
	flags = (FontFlags)0;
	lookUpTable = 0;
}

SwfFont::~SwfFont()
{
	if (lookUpTable) {
		delete lookUpTable;
	}
        delete name;
        delete [] glyphs;
}

void
SwfFont::setFontFlags(FontFlags f)
{
	flags = f;
}

char *
SwfFont::getName()
{
	return name;
}

FontFlags
SwfFont::getFlags()
{
	return flags;
}

long
SwfFont::getNbGlyphs()
{
	return nbGlyphs;
}

Shape  *
SwfFont::getGlyph(long index)
{
	if (index >= nbGlyphs) return 0;
	return &glyphs[index];
}

long
SwfFont::getGlyphCode(long index)
{
	if (lookUpTable == 0 || index >= nbGlyphs) return 0;
	return lookUpTable[index];
}

void
SwfFont::setFontName(char *str)
{
    delete name;
    name = new char[strlen(str)+1];
    strcpy(name,str);
}

void
SwfFont::setFontLookUpTable(long *lut)
{
	lookUpTable = lut;
}

void
SwfFont::setFontShapeTable(Shape *shapes, long n)
{
	glyphs = shapes;
	nbGlyphs = n;
}
