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
static char *rcsid = "$Id$";
#endif

Sprite::Sprite(FlashMovie *movie, long id, long frameCount) : Character(SpriteType, id)
{
	program = new Program(movie, frameCount);
	if (program == NULL) return;
	if (program->totalFrames == 0) {
		delete program;
		program = NULL;
		return;
	}
	program->dl->isSprite = 1;
}

Sprite::~Sprite()
{
	delete program;
}

void
Sprite::reset()
{
	program->rewindMovie();
}

int
Sprite::isSprite(void)
{
    return 1;
}

Program *
Sprite::getProgram()
{
	return program;
}

int
Sprite::execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform)
{
	return program->dl->render(gd,matrix,cxform);
}

ActionRecord *
Sprite::eventHandler(GraphicDevice *gd, FlashEvent *event)
{
#if 0
	DisplayList *dl;
	ActionRecord *actions;

	dl = program->getDisplayList();
	actions = dl->processEvent(gd, event);
	if (actions) {
		program->doAction(actions,0);
	}
	return actions;
#endif
	return NULL;
}

void
Sprite::getBoundingBox(Rect *bb, DisplayListEntry *e)
{
	program->dl->getBoundary(bb);
}
