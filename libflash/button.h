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
#ifndef _BUTTON_H_
#define _BUTTON_H_

struct ButtonRecord {
	ButtonState		 state;
	Character		*character;
	long			 layer;
	Matrix			 buttonMatrix;
	Cxform			*cxform;

	struct ButtonRecord	*next;
};

struct Condition {
	long			 transition;
	ActionRecord		*actions;

	Condition		*next;
};

class Button : public Character {
public:
	long			 defLevel;

	ButtonRecord		*buttonRecords;
	ActionRecord		*actionRecords;
	Condition		*conditionList;

	long			 isMenu;

	Sound			*sound[4];

	Button(long id, int level = 1);
	~Button();
	void		 addActionRecord( ActionRecord *ar );
        void		 addButtonRecord( ButtonRecord *br );
	void		 addCondition( long transition );
	int		 execute(GraphicDevice *gd, Matrix *matrix, 
                                 Cxform *cxform, ButtonState renderState);
	ActionRecord	*getActionFromTransition(ButtonState currentState, 
                                                 ButtonState old);
	void		 getRegion(GraphicDevice *gd, Matrix *matrix, 
                                   void *id, ScanLineFunc scan_line_func);
	ButtonRecord	*getButtonRecords();
	void		 setButtonSound(Sound *, int);
	void		 setButtonMenu(int);

	ActionRecord	*getActionRecords();
	Condition	*getConditionList();
	Sound	       **getSounds();

	void		 getBoundingBox(Rect *bb, DisplayListEntry *);

	void		 updateButtonState(DisplayListEntry *);
	Character	*getRenderCharacter(ButtonState state);

	// Builtin
	int	 isButton() {
		return 1;
	};

#ifdef DUMP
	void		 dump(BitStream *);
	void		 dumpButtonRecords(BitStream *, int putCxform = 0);
	void		 dumpButtonConditions(BitStream *);
#endif
};

#endif /* _BUTTON_H_ */
