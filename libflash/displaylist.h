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
#ifndef _DISPLAYLIST_H_
#define _DISPLAYLIST_H_

class Character;
class Program;

struct DisplayList;

// Display List management
struct DisplayListEntry {
    Character		*character;
    long		 depth;
    Matrix		*matrix;
    Cxform		*cxform;
    char		*instanceName;
    
    /* button state */
    ButtonState		 renderState;
    ButtonState		 oldState;
    Character		*buttonCharacter;
    Matrix		 buttonMatrix;
    Matrix renderMatrix; /* last render matrix */

    DisplayListEntry	*next;

    DisplayList		*owner;	// Parent
};

struct DisplayList {
    DisplayListEntry	*list;
    FlashMovie          *movie;
    Rect		 bbox;		// Delta clipping region
    int			 isSprite;
public:
    DisplayList(FlashMovie *movie);
    ~DisplayList();
    DisplayListEntry	*getList();
    void		 clearList();
    void		 placeObject(GraphicDevice *gd,Character *character, long depth, Matrix *matrix = 0, Cxform *cxform = 0, char *name = 0);
    Character		*removeObject(GraphicDevice *gd, Character *character, long depth);

    int			 render(GraphicDevice *gd, Matrix *m = 0, Cxform *cxform = 0);
    void		 updateBoundingBox(DisplayListEntry *);
    void		 updateButtons (FlashMovie *);
    void 		 getBoundary(Rect *bb);	// Returns boundary of current displayed objects
    int			 updateSprites();	// Update sprites in the display list
};

typedef void (*DisplayListFunc)(DisplayListEntry *e, void *opaque);

void		 updateButtons(FlashMovie *m);
int		 computeActions(FlashMovie *m, Program **prog, ActionRecord **ar);
void		 renderFocus(FlashMovie *movie);

typedef int (*ExploreButtonFunc)(void *opaque, Program *prg, DisplayListEntry *e);
int		 exploreButtons(FlashMovie *movie, void *opaque, ExploreButtonFunc func);
void		 updateBoundingBox(DisplayListEntry *e);
void		 transformBoundingBox(Rect *bb, Matrix *matrix, Rect *boundary, int reset);
void		 updateButtonState(DisplayListEntry *e, ButtonState state);

#endif /* _DISPLAYLIST_H_ */
