////////////////////////////////////////////////////////////
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

#define PRINT 0

void deleteButton(FlashMovie *movie, DisplayListEntry *e)
{
    /* save the focus */
    if (movie->mouse_active == 0 && e->renderState == stateOver) {
        movie->lost_over = (Button *)e->character;
        movie->cur_focus = NULL;
    }

    if (e == movie->cur_focus) {
        movie->cur_focus = NULL;
    }
}

void addButton(FlashMovie *movie, DisplayListEntry *e)
{
    if (movie->mouse_active == 0 && 
        movie->cur_focus == NULL &&
        movie->lost_over == (Button *)e->character) {
        /* restore the lost focus */
        e->renderState = stateOver;
        e->oldState = stateOver;
	((Button *)e->character)->updateButtonState(e);
        movie->lost_over = NULL;
        movie->cur_focus = e;
    }
}

DisplayList::DisplayList(FlashMovie *movie)
{
	list = NULL;
        this->movie = movie;
	bbox.reset();
	isSprite = 0;
}

DisplayList::~DisplayList()
{
	clearList();
}

void
DisplayList::clearList()
{
	DisplayListEntry *del, *e;

	for(e = list; e;)
	{
		updateBoundingBox(e);
                if (e->character->isButton()) {
                    deleteButton(movie,e);
                }
		del = e;
		e = e->next;
		delete del;
	}
	list = 0;
}

DisplayListEntry *
DisplayList::getList()
{
	return list;
}

static void bbox(Rect *rect, Matrix *m, long x1, long y1)
{
    long x,y;

    x = m->getX(x1,y1);
    y = m->getY(x1,y1);
    if (x < rect->xmin) rect->xmin = x;
    if (x > rect->xmax) rect->xmax = x;
    if (y < rect->ymin) rect->ymin = y;
    if (y > rect->ymax) rect->ymax = y;
}

// Update bb to include boundary, optional reset of bb
void transformBoundingBox(Rect *bb, Matrix *matrix, Rect *boundary, int reset)
{
    if (reset) {
        bb->reset();
    }
    
    if (boundary->xmin != LONG_MAX) {
        bbox(bb, matrix, boundary->xmin, boundary->ymin);
        bbox(bb, matrix, boundary->xmax, boundary->ymin);
        bbox(bb, matrix, boundary->xmin, boundary->ymax);
        bbox(bb, matrix, boundary->xmax, boundary->ymax);
    }
}

void
DisplayList::placeObject(GraphicDevice *gd,Character *character, long depth, Matrix *matrix, Cxform *cxform, char *name)
{
	DisplayListEntry *n,*e,*prev;

	n = new DisplayListEntry;
	if (n == NULL) return;

	n->depth = depth;
	n->matrix = matrix;
	n->cxform = cxform;
	n->character = character;
	n->instanceName = name;
	n->owner = this;

#if 0
        printf("Dl %lx: placeObject: depth=%d character=%d cxform=%p\n",
               this, n->depth,n->character ? n->character->getTagId() : 0, cxform);
#endif

	if (character == 0 || matrix == 0 || cxform == 0) {
		for (e = list; e; prev = e, e = e->next) {
			if (e->depth == n->depth) {
				if (character == 0) {
					n->character = e->character;
				}
				if (matrix == 0) {
					n->matrix = e->matrix;
				}
				if (cxform == 0) {
					n->cxform = e->cxform;
				}
				break;
			}
		}
	}

	if (n->character == 0) {
		// Not found !!!    Should not happen
            //		printf("PlaceObject cannot find character at depth %ld\n", n->depth);
		delete n;
		return;
	}

	prev = 0;
	for (e = list; e; prev = e, e = e->next)
	{
		if (e->depth == n->depth) {
                        if (e->character->isButton()) {
                            deleteButton(movie, e);
                        }

			// Do update, object has moved or been resized
		        updateBoundingBox(e);

			// Replace object
                        e->depth = n->depth;
                        e->matrix = n->matrix;
                        e->cxform = n->cxform;
                        e->character = n->character;
                        /* if it is a button, we must update its state */
                        if (e->character->isButton()) {
                            movie->buttons_updated = 1;
                            addButton(movie, e);
                        }

		        updateBoundingBox(e);

                        delete n;
                        return;
		}
		if (e->depth > n->depth) break;
	}
        /* new object */

        /* button instantiation */
        if (n->character->isButton()) {
            n->renderState = stateUp;
            n->oldState = stateUp;
	    ((Button *)n->character)->updateButtonState(n);
            addButton(movie,n);
        }

	updateBoundingBox(n);

	if (prev == 0) {
		// Object comes at first place
		n->next = list;
		list = n;
	} else {
		// Insert object
		n->next = prev->next;
		prev->next = n;
	}
}


Character *
DisplayList::removeObject(GraphicDevice *gd,Character *character, long depth)
{
    DisplayListEntry *e,*prev;
    
    // List should not be empty
    if (list == 0) return 0;
    
#if 0
    printf("removeObject: depth=%d character=%d\n",
           depth,character ? character->getTagId() : 0);
#endif
    
    prev = 0;
    for (e = list; e; prev = e, e = e->next) {
        if (e->depth == depth) {
            if (prev) {
                prev->next = e->next;
            } else {
                list = e->next;
            }
            if (character == 0) {
                character = e->character;
            }
            if (e->character->isButton()) {
                deleteButton(movie, e);
            }
            if (e->character->isSprite()) {
	    	((Sprite*)e->character)->reset();
	    }
                
	    updateBoundingBox(e);

            delete e;
            return character;
        }
    }
    return 0;	// Should not happen
}

void
DisplayList::updateBoundingBox(DisplayListEntry *e)
{
	Rect	 rect;

	//rect.reset();
	e->character->getBoundingBox(&rect,e);
	transformBoundingBox(&this->bbox, e->matrix, &rect, 0);
}

int
DisplayList::updateSprites()
{
    Sprite *sprite;
    DisplayListEntry *e;
    int refresh = 0;

    for (e = this->list; e != NULL; e = e->next) {
        if (e->character->isButton() && e->buttonCharacter) {
		if (e->buttonCharacter->isSprite()) {
			Matrix mat;

			sprite = (Sprite *)e->buttonCharacter;
			refresh |= sprite->program->dl->updateSprites();
			refresh |= sprite->program->nestedMovie(this->movie->gd,this->movie->sm, e->matrix, e->cxform);
			mat = (*e->matrix) * e->buttonMatrix;
			transformBoundingBox(&this->bbox, &mat,
					&(sprite->program->dl->bbox),
					0);
		}
	}
        if (e->character->isSprite()) {
		sprite = (Sprite *)e->character;
		refresh |= sprite->program->dl->updateSprites();
		refresh |= sprite->program->nestedMovie(this->movie->gd,this->movie->sm, e->matrix, e->cxform);
		transformBoundingBox(&this->bbox, e->matrix,
				&(sprite->program->dl->bbox),
				0);
        }
    }
    return refresh;
}

/* Function can return either 0,1 or 2
   0:  Nothing match, continue
   1:  Something matches, but continue searching
   2:  Something matches, but stop searching
*/

static int exploreButtons1(Program *prg, void *opaque, 
                           ExploreButtonFunc func)
{
    DisplayListEntry *e;
    int ret, ret2 = 0;

    for(e=prg->dl->list; e != NULL; e = e->next) {
	if (e->character == NULL) continue;
        if (e->character->isButton()) {
            ret = func(opaque,prg,e);
	    if (ret == 2) return ret;	// Func asks to return at once !!!
            if (ret) ret2 = 1;
        }
        if (e->character->isSprite()) {
            ret = exploreButtons1(((Sprite *)e->character)->program,
                                  opaque,func);
	    if (ret == 2) return ret;	// Func asks to return at once !!!
            if (ret) ret2 = 1;
        }
    }
    return ret2;
}

int exploreButtons(FlashMovie *movie, void *opaque, ExploreButtonFunc func)
{
    CInputScript *script;
    int ret;

    script = movie->main;
    while (script != NULL) {
	if (script->program) {
		ret = exploreButtons1(script->program, opaque, func);
		if (ret) return ret;
	}
        script = script->next;
    }
    return 0;
}

typedef struct {
    long x,y;
    int hit;
    DisplayListEntry *bhit;
} HitTable;

static void button_hit_func(void *id, long y, long start, long end)
{
    HitTable *h = (HitTable *) id;
    if ( y == h->y && (h->x >= start && h->x < end) )
        h->hit = 1;
}

typedef struct {
    FlashMovie *movie;
    DisplayListEntry *bhit;
} ButtonHit;

static int button_hit(void *opaque, Program *prg, DisplayListEntry *e)
{
    ButtonHit *h = (ButtonHit *) opaque;
    HitTable hit_table;
    FlashMovie *movie = h->movie;
    Rect bb,boundary;
    Matrix mat;
    ButtonState save;

    hit_table.x = movie->mouse_x;
    hit_table.y = movie->mouse_y / FRAC;
    hit_table.hit = 0;
    
    // Compute the bounding box in screen coordinates
    save = e->renderState;
    e->renderState = stateHitTest;
    e->character->getBoundingBox(&boundary,e);
    e->renderState = save;
    mat = (*movie->gd->adjust) * e->renderMatrix;
    transformBoundingBox(&bb, &mat, &boundary, 1);
    // Check if mouse is within bb
    if (movie->mouse_x < bb.xmin) return 0;
    if (movie->mouse_x > bb.xmax) return 0;
    if (movie->mouse_y < bb.ymin) return 0;
    if (movie->mouse_y > bb.ymax) return 0;

    e->character->getRegion(movie->gd, &e->renderMatrix, 
                            &hit_table, button_hit_func);
                
    if (hit_table.hit) {
        h->bhit = e;
        return 1;
    } else {
        return 0;
    }
}

static int button_reset(void *opaque, Program *prg, DisplayListEntry *e)
{
    if (e->renderState != stateUp) {
	    e->owner->updateBoundingBox(e);
	    e->oldState = e->renderState;
	    e->renderState = stateUp;
	    ((Button *)e->character)->updateButtonState(e);
	    e->owner->updateBoundingBox(e);
    }
    return 0;
}

/* update the button states according to the current mouse state & return the list of actions */
void
DisplayList::updateButtons(FlashMovie *movie)
{
    DisplayListEntry *bhit;
    ButtonHit h;

    if (movie->mouse_active) {

        h.bhit = NULL;
        h.movie = movie;

        exploreButtons(movie, &h, button_hit);

        bhit = h.bhit;

        /* set every button to not hit */
        exploreButtons(movie, NULL, button_reset);

        if (bhit) {
	    ButtonState state;

            if (movie->button_pressed) {
                state = stateDown;
            } else {
                state = stateOver;
            }
	    if (state != bhit->renderState) {
		    bhit->owner->updateBoundingBox(bhit);
		    bhit->renderState = state;
		    ((Button *)bhit->character)->updateButtonState(bhit);
		    bhit->owner->updateBoundingBox(bhit);
		    movie->cur_focus = bhit;
		    if (movie->cursorOnOff)
			    movie->cursorOnOff(1,movie->cursorOnOffClientData);
	    }
        } else {
	    if (movie->cursorOnOff)
		    movie->cursorOnOff(0,movie->cursorOnOffClientData);
	}
    }
}

typedef struct {
    ActionRecord *action;	// Action to do
    Program	 *prg;		// Context program
} ButtonAction;

static int button_action(void *opaque, Program *prg, DisplayListEntry *e)
{
    ButtonAction *h = (ButtonAction *)opaque;
    static ActionRecord actionRefresh;
    static ActionRecord soundFx;
    Button *b;
    ActionRecord **paction;
    int n;

    actionRefresh.action = ActionRefresh;
    actionRefresh.next = 0;
    
    soundFx.action = ActionPlaySound;
    soundFx.next = &actionRefresh;

    b = (Button *)e->character;

    if (e->oldState != e->renderState) {
        
        paction = &actionRefresh.next;
        
        if (b->conditionList) {
            *paction = b->getActionFromTransition(e->renderState, e->oldState);
        } else if (e->renderState == stateDown) {
            /* if the button is pressed and 
               no condition list is defined*/
            *paction = b->actionRecords;
        }
        
        switch(e->renderState) {
        case stateUp:
            n = 0;
            break;
        case stateOver:
            n = 1;
            break;
        default:
            /* case stateDown: */
            n = 2;
            break;
        }
        
        if (b->sound[n]) {
            soundFx.sound = b->sound[n];
            h->action = &soundFx;
        } else {
            h->action = &actionRefresh;
        }
        
        e->oldState = e->renderState;

        h->prg = prg;
        return 2;
    }
    h->action = 0;	// Nothing to do about this
    return 0;
}

int computeActions(FlashMovie *movie, Program **prg, ActionRecord **ar)
{
    ButtonAction h;

    h.action = NULL;
    exploreButtons(movie, &h, button_action);
    if (h.action) {
    	*prg = h.prg;
	*ar = h.action;
	return 1;
    }
    return 0;
}

#define FOCUS_ZOOM       1.5
/* in pixels */
#define FOCUS_SIZE_MIN   50
#define FOCUS_TRANSLATE  15

int
DisplayList::render(GraphicDevice *gd, Matrix *render_matrix, Cxform *cxform)
{
	DisplayListEntry *e,*cur_focus;
	int sprite = 0;
	long n = 0;
        Cxform cxf,*cxf1;
	Rect bb,boundary;

        cur_focus = NULL;

	/*
	if (isSprite == 0) {
		if (this->bbox.xmin == LONG_MAX) return 0;
		gd->updateClippingRegion(&this->bbox, render_matrix);
		gd->clearCanvas();
	}
	*/

	for (e = list; e; e = e->next)
	{
#if PRINT
		printf("Character %3d @ %3d\n", e->character ? e->character->getTagId() : 0, e->depth);
#endif
		if (e->character) {
			Matrix mat;

			if (render_matrix) {
				mat = *render_matrix;
			}

			if (e->matrix) {
				mat = mat * (*e->matrix);
			}

                        /* fast clipping */
			// If object boundaries are outside current clip region give up with rendering
                        e->character->getBoundingBox(&boundary,e);
                        if (boundary.xmin != LONG_MAX) {
                            Matrix tmat;

                            tmat = (*gd->adjust) * mat;
                            transformBoundingBox(&bb, &tmat, &boundary, 1);

                            bb.xmin = bb.xmin >> FRAC_BITS;
                            bb.ymin = bb.ymin >> FRAC_BITS;
                            bb.xmax = (bb.xmax + FRAC - 1) >> FRAC_BITS;
                            bb.ymax = (bb.ymax + FRAC - 1) >> FRAC_BITS;

                            if (bb.xmin >= gd->clip_rect.xmax ||
                                bb.xmax <= gd->clip_rect.xmin ||
                                bb.ymin >= gd->clip_rect.ymax ||
                                bb.ymax <= gd->clip_rect.ymin) {
                                continue;
                            }
                        }

                        if (cxform == NULL) {
                            cxf1 = e->cxform;
                        }
			else if (e->cxform == NULL) {
                            cxf1 = cxform;
                        }
			else {
                            cxf1 = &cxf;
                            cxf.ra = cxform->ra * e->cxform->ra;
                            cxf.ga = cxform->ga * e->cxform->ga;
                            cxf.ba = cxform->ba * e->cxform->ba;
                            cxf.aa = cxform->aa * e->cxform->aa;
                            
                            cxf.rb = (long)(cxform->ra * e->cxform->rb + cxform->rb);
                            cxf.gb = (long)(cxform->ga * e->cxform->gb + cxform->gb);
                            cxf.bb = (long)(cxform->ba * e->cxform->bb + cxform->bb);
                            cxf.ab = (long)(cxform->aa * e->cxform->ab + cxform->ab);
                        }

                        if (e->character->isButton()) {
                            Button *b = (Button *) e->character;

                            e->renderMatrix = mat;

                            if (e->renderState != stateUp && movie->mouse_active == 0) {
                                cur_focus = e;
				((Button *)e->character)->updateButtonState(e);
                            }

                            if (b->execute(gd, &mat, cxf1, e->renderState)) {
				sprite = 1;
                            }
                        } else {
                            if (e->character->execute(gd, &mat, cxf1)) {
				sprite = 1;
                            }
                        }

			n++;
		}
	}

#if 0
    {
	/* display the bounding box (debug) */
	Matrix tmat;
	long x1,x2,y1,y2;
	Color white;

	white.red = 255;
	white.green = white.blue = 0;
	gd->setForegroundColor(white);

	if (render_matrix) {
		tmat = (*gd->adjust) * (*render_matrix);
	} else {
		tmat = *gd->adjust;
	}
	x1 = bbox.xmin;
	y1 = bbox.ymin;
	x2 = bbox.xmax;
	y2 = bbox.ymax;
	gd->drawLine(tmat.getX(x1,y1),tmat.getY(x1,y1),tmat.getX(x2,y1),tmat.getY(x2,y1),10*FRAC);
	gd->drawLine(tmat.getX(x2,y1),tmat.getY(x2,y1),tmat.getX(x2,y2),tmat.getY(x2,y2),10*FRAC);
	gd->drawLine(tmat.getX(x2,y2),tmat.getY(x2,y2),tmat.getX(x1,y2),tmat.getY(x1,y2),10*FRAC);
	gd->drawLine(tmat.getX(x1,y2),tmat.getY(x1,y2),tmat.getX(x1,y1),tmat.getY(x1,y1),10*FRAC);
	bbox.print();
    }
#endif
        
	// Reset clipping zone
        bbox.reset();

	return sprite;
}

void
DisplayList::getBoundary(Rect *bb)
{
	DisplayListEntry *e;
	Rect boundary;

	bb->reset();
	for (e = list; e; e = e->next)
	{
		if (e->character) {
			e->character->getBoundingBox(&boundary,e);
			transformBoundingBox(bb, e->matrix, &boundary, 0);
		}
	}
}

extern "C" {

void dump_buttons(FlashHandle flashHandle)
{
#if 0
    Rect rect;
    DisplayListEntry *e;
    FlashMovie *movie;

    movie = (FlashMovie *)flashHandle;

    for (e = movie->first_button; e; e = e->next_button) {
        computeBBox(movie,&rect,e);
        printf("button: id=%d pos=%d %d %d %d\n",
               e->character->getTagId(),
               rect.xmin, rect.ymin, rect.xmax, rect.ymax);
    }
#endif
}

}
