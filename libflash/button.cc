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

#define PRINT 0

// Contructor

Button::Button(long id, int level) : Character(ButtonType, id)
{
	defLevel = level;
	actionRecords = 0;
	buttonRecords = 0;
	conditionList = 0;
	reset();
	isMenu = 0;
	sound[0] = sound[1] = sound[2] = sound[3] = 0;
}

// Destructor

Button::~Button()
{
	if (actionRecords) {
		ActionRecord *ar,*del;
		for(ar = actionRecords; ar;) {
			del = ar;
			ar = ar->next;
			delete del;
		}
	}
	if (buttonRecords) {
		ButtonRecord *br,*del;
		for(br = buttonRecords; br;) {
			del = br;
			br = br->next;
			if (del->cxform)
				delete del->cxform;
			delete del;
		}
	}
	if (conditionList) {
		Condition *cond,*del;
		for(cond = conditionList; cond;) {
			ActionRecord *ar,*d;

			for(ar = cond->actions; ar;) {
				d = ar;
				ar = ar->next;
				delete d;
			}

			del = cond;
			cond = cond->next;
			delete del;
		}
	}
}

ButtonRecord *
Button::getButtonRecords()
{
	return buttonRecords;
}

ActionRecord *
Button::getActionRecords()
{
	return actionRecords;
}

Sound **
Button::getSounds()
{
	return sound;
}

Condition *
Button::getConditionList()
{
	return conditionList;
}

void
Button::setButtonSound(Sound *s, int state)
{
	if (state >=0 && state < 4) {
		sound[state] = s;
	}
}

void
Button::setButtonMenu(int menu)
{
	isMenu = menu;
}

void
Button::addButtonRecord( ButtonRecord *br )
{
#if 0
    /* SURTOUT PAS !!! */
    ButtonRecord **l;
    
    /* sort by layer */
    l=&buttonRecords;
    while (*l != NULL && (*l)->layer < br->layer) l = &(*l)->next;
    br->next = *l;
    *l = br;
#else
	br->next = 0;

	if (buttonRecords == 0) {
		buttonRecords = br;
	} else {
		ButtonRecord *current;

		for(current = buttonRecords; current->next; current = current->next);

		current->next = br;
	}
#endif
}

void
Button::addCondition( long transition )
{
	Condition *condition;

	condition = new Condition;
	if (condition == NULL) return;

	condition->transition = transition; 
	condition->next = conditionList;

	// Move current actionRecords to this condition
	condition->actions = actionRecords;
	actionRecords = 0;

	conditionList = condition;
}

void
Button::addActionRecord( ActionRecord *ar )
{
	ar->next = 0;

	if (actionRecords == 0) {
		actionRecords = ar;
	} else {
		ActionRecord *current;

		for(current = actionRecords; current->next; current = current->next);

		current->next = ar;
	}
}

void
Button::getRegion(GraphicDevice *gd, Matrix *matrix, void *id, ScanLineFunc scan_line_func)
{
	ButtonRecord *br;

	for (br = buttonRecords; br; br = br->next)
	{
		if ((br->state & stateHitTest) && br->character /* Temporaire */) {
			Matrix mat;

			mat = (*matrix) * br->buttonMatrix;
			br->character->getRegion(gd, &mat, id, scan_line_func);
		}
	}
}

int
Button::execute(GraphicDevice *gd, Matrix *matrix, Cxform *cxform, ButtonState renderState)
{
	ButtonRecord *br;
	int sprite = 0;
	Cxform *cxf = 0;

#if PRINT==2
	printf("Rendering Button %d  for State(s) ", getTagId());
#endif
	for (br = buttonRecords; br; br = br->next)
	{
            if ((br->state & renderState) && br->character != NULL) {
			Matrix mat;
			
#if PRINT==2
		printf("%d ", br->state);
#endif
			mat = (*matrix) * br->buttonMatrix;

                        if (cxform) {
				cxf = cxform;
			} else if (br->cxform) {
				cxf = br->cxform;
			} 

			if (br->character->execute(gd, &mat, cxf)) {
				sprite = 1;
			}
		}
	}
#if PRINT==2
	printf("\n");
#endif
	return sprite;
}

ActionRecord *
Button::getActionFromTransition(ButtonState cur, ButtonState old)
{
	Condition *cond;
	long mask;

	if (old == cur) return NULL;

        /* transitions */
        mask = 0;
        if (old == stateUp && cur == stateOver)
            mask |= 0x001;
        else if (old == stateOver && cur == stateUp)
            mask |= 0x002;
        else if (old == stateOver && cur == stateDown)
            mask |= 0x004;
        else if (old == stateDown && cur == stateOver)
            mask |= 0x008;

        if (!isMenu) {
            /* push button transitions (XXX: not quite correct) */
            if (old == stateDown && cur == stateUp)
                mask = 0x010;
            else if (old == stateUp && cur == stateDown)
                mask = 0x020;
            /* XXX: what is transition 0x040 ?? */
        } else {
            /* menu button transitions (XXX: not quite correct) */
            if (old == stateUp && cur == stateDown)
                mask = 0x080;
            else if (old == stateDown && cur == stateUp)
                mask = 0x100;
        }

	for (cond = conditionList; cond; cond = cond->next) {
		if (cond->transition & mask) {
			return cond->actions;
		}
	}
	return 0;
}

void
Button::getBoundingBox(Rect *bbox, DisplayListEntry *e)
{
	ButtonRecord *br;

	bbox->reset();
	for (br = buttonRecords; br; br = br->next)
	{
                if (br->state & e->renderState) {
			if (br->character) {
				Rect bb;
				
				bb.reset();
				br->character->getBoundingBox(&bb,e);
				transformBoundingBox(bbox, &br->buttonMatrix, &bb, 0);
			}
		}
	}
}

/* Get current render character, actually it should be a list of characters
   so a DisplayList after all */
Character *
Button::getRenderCharacter(ButtonState state)
{
	ButtonRecord *br;

	for (br = buttonRecords; br; br = br->next)
	{
                if (br->state & state) {
			return br->character;
		}
	}
	return 0;
}

void
Button::updateButtonState(DisplayListEntry *e)
{
	ButtonRecord *br;

	e->buttonCharacter = 0;
	for (br = buttonRecords; br; br = br->next)
	{
                if (br->state & e->renderState) {
			e->buttonCharacter = br->character;
			e->buttonMatrix = br->buttonMatrix;
			return;
		}
	}
}
