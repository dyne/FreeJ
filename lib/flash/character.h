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
#ifndef _CHARACTER_H_
#define _CHARACTER_H_

enum ObjectType {
	ShapeType,
	TextType,
	FontType,
	SoundType,
	BitmapType,
	SpriteType,
	ButtonType
};

class DisplayListEntry;

// Character definition

class Character {
	long			 tagId;
	ObjectType		 type;
	char			*name;

public:
	Character(ObjectType type, long tagId);
        virtual ~Character();

	virtual int		 execute(GraphicDevice *, Matrix *, Cxform *);	// Display, play or whatever
	virtual int		 isButton(void);	// True if Character is a button
        virtual int              isSprite(void);
	virtual ActionRecord	*eventHandler(GraphicDevice *, FlashEvent *);
        virtual void		 getRegion(GraphicDevice *gd, Matrix *matrix, void *id, ScanLineFunc scan_line_func);
	virtual void		 reset();	// Reset internal state of object
	virtual void		 getBoundingBox(Rect *bb, DisplayListEntry *de);
#ifdef DUMP
	virtual void		 dump(BitStream *main);

	int			 saved;
#endif

	long			 getTagId();	// Return tagId
	ObjectType		 getType();
	char			*getTypeString();
	char			*getName();
	void			 setName(char *);
};

struct sCharCell {
	Character *elt;
	struct sCharCell *next;
};

class Dict {
	struct sCharCell *head;
	struct sCharCell *currentCell;	// Iteration variable for dictNextCharacter

public:
	Dict();
	~Dict();

	void		 addCharacter(Character *character);
	void		 nameCharacter(long id, char *string);
	Character	*getCharacter(long id);
	void		 dictRewind();
	Character	*dictNextCharacter();

#ifdef DUMP
	void		 dictSetUnsaved();
#endif
};

#endif /* _CHARACTER_H_ */
