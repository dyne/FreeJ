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
#ifndef _SWFFONT_H_
#define _SWFFONT_H_

class SwfFont : public Character {
	Shape		*glyphs;	// Array
	long		 nbGlyphs;
	char		*name;
	FontFlags	 flags;
	long		*lookUpTable;	// Array

	// Font2
	long		 ascent;
	long		 descent;
	long		 leading;

public:
	SwfFont(long id);
	~SwfFont();

	void		 setFontShapeTable(Shape *shapes, long n);
	void		 setFontName(char *str);
	void		 setFontLookUpTable(long *lut);
	void		 setFontFlags(FontFlags f);
	long		 getGlyphCode(long index);
        long		 getNbGlyphs();
	Shape		*getGlyph(long index);

	char 		*getName();
	FontFlags	 getFlags();
	
#ifdef DUMP
	void		 dump(BitStream *bs);
	void		 dumpFontInfo(BitStream *bs);
#endif
};

#endif /* _SWFFONT_H_ */
