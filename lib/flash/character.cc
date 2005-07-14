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

///// Character member definitions

Character::Character(ObjectType objectType, long tagid)
{
	type = objectType;
	tagId = tagid;
	name = NULL;
}

Character::~Character()
{
	delete name;
}

int
Character::execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform)
{
	printf("Cannot be executed\n");
	return 0;
}

ActionRecord *
Character::eventHandler(GraphicDevice *gd, FlashEvent *ev)
{
	fprintf(stderr,"Unable to handle event !!!\n");
	return 0;
}

int
Character::isButton()
{
	return 0;
}

int
Character::isSprite(void)
{
    return 0;
}

char *
Character::getName()
{
	return name;
}

void
Character::getBoundingBox(Rect *bb, DisplayListEntry *e)
{
    //fprintf(stderr,"Unable to handle getBoundingBox !!!\n");
    bb->xmin = LONG_MAX;
    bb->ymin = LONG_MAX;
    bb->ymax = LONG_MIN;
    bb->ymax = LONG_MIN;
    return;
}

void
Character::getRegion(GraphicDevice *gd, Matrix *matrix, 
                               void *id, ScanLineFunc scan_line_func)
{
	fprintf(stderr,"Unable to handle getRegion !!!\n");
	return;
}

long
Character::getTagId()
{
	return tagId;
}

void
Character::reset()
{
}

ObjectType
Character::getType()
{
	return type;
}

char *
Character::getTypeString()
{
	switch (type) {
		case BitmapType:
			return "Bitmap";
		case FontType:
			return "Font";
		case ButtonType:
			return "Button";
		case SpriteType:
			return "Sprite";
		case ShapeType:
			return "Shape";
		case SoundType:
			return "Sound";
		case TextType:
			return "Text";
		default:
			return "Unknown";
	}
}

void
Character::setName(char* string)
{
	name = strdup(string);
}

///// Dict methods definitions

Dict::Dict()
{
	head = 0;
}

Dict::~Dict()
{
	struct sCharCell *cell,*del;
	
	for(cell = head; cell;)
	{
		del = cell;
		cell = cell->next;
                delete del->elt;
		delete del;
	}
}

void
Dict::addCharacter(Character *character)
{
	struct sCharCell *cell;

	cell = new sCharCell;
	if (cell == NULL) {
		delete character;
		return;
	}
	cell->elt = character;
	cell->next = head;

	head = cell;
}

Character *
Dict::getCharacter(long id)
{
	struct sCharCell *cell;
	
	for(cell = head; cell; cell = cell->next)
	{
		if (id == cell->elt->getTagId()) return cell->elt;
	}
	return 0;
}

void
Dict::dictRewind()
{
	currentCell = head;
}

Character *
Dict::dictNextCharacter()
{
	if (currentCell) {
		struct sCharCell *cell;

		cell = currentCell;
		currentCell = currentCell->next;
		return cell->elt;
	} else {
		return 0;
	}
}

void
Dict::nameCharacter(long id, char *string)
{
        struct sCharCell *cell;

        for(cell = head; cell; cell = cell->next)
        {
                if (cell->elt->getTagId() == id) {
                        cell->elt->setName(string);
                        break;
                }
        }
}

#ifdef DUMP
void
Dict::dictSetUnsaved()
{
	struct sCharCell *cell;
	
	for(cell = head; cell; cell = cell->next)
	{
		cell->elt->saved = 0;
	}
}
#endif
