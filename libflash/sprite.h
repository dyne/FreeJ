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
#ifndef _SPRITE_H_
#define _SPRITE_H_

class Sprite : public Character {
public:
	Program		*program;

	Sprite(FlashMovie *movie, long id, long frameCount);
	~Sprite();
	Program		*getProgram();
	int		 execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform);
	int		 hasEventHandler();
	void		 reset();
	ActionRecord	*eventHandler(GraphicDevice *, FlashEvent *);
        int              isSprite(void);
	void		 getBoundingBox(Rect *bb, DisplayListEntry *de);
};

#endif /* _SPRITE_H_ */
