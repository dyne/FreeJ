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

#define NOTHING  0x0
#define WAKEUP   0x1
#define GOTO     0x2
#define REFRESH  0x4

#ifdef RCSID
static char *rcsid = "$Id: program.cc,v 1.1.1.1 2004/06/04 21:16:33 tgc Exp $";
#endif

#define PRINT 0

//int debug = 0;

Program::Program(FlashMovie *movie, long n)
{
	long f;

        this->movie = movie;

	totalFrames = 0;

	dl = new DisplayList(movie);
	if (dl == NULL) return;
	frames = new Frame[n];
	if (frames == NULL) {
		delete dl;
		return;
	}

	nbFrames = 0;
	totalFrames = n;
	currentFrame = 0;
	loadingFrame = 0;
	movieWait = 1;
	nextFrame = currentFrame;
	for(f = 0; f < n; f++)
	{
		frames[f].controls = 0;
		frames[f].label = NULL;
	}

	movieStatus = MoviePlay;
	settings = 0;
}

Program::~Program()
{
    int i;
    Control *ctrl, *ctrl1;

    delete dl;

    if (frames != NULL) {
	    for(i=0;i<nbFrames;i++) {
		ctrl = frames[i].controls;
		if (frames[i].label) free(frames[i].label);
		while (ctrl != NULL) {
		    ctrl1 = ctrl->next;
		    ctrl->next = NULL;
		    delete ctrl;
		    ctrl = ctrl1;
		}
	    }

	    delete[] frames;
    }
}

void
Program::validateLoadingFrame()
{
	nbFrames = loadingFrame;
	loadingFrame++;
	movieWait = 0;
}

Frame	*
Program::getFrames()
{
	return frames;
}

long
Program::getNbFrames()
{
	return nbFrames;
}

DisplayList *
Program::getDisplayList()
{
	return dl;
}

long
Program::getCurrentFrame()
{
	return currentFrame;
}

void
Program::setCurrentFrame(long n)
{
	currentFrame = n;
	nextFrame = n;
	//refresh = 1;
}

void
Program::gotoFrame(GraphicDevice *gd, long frame)
{
	long f;

	//printf("GotoFrame %d  (Current = %d)\n", frame, currentFrame);
	dl->clearList();

	for(f=0; f <= frame; f++) {
		runFrame(gd, f, 0);
	}
}

long
Program::runFrame(GraphicDevice *gd, long f, long action)
{
	Control		*ctrl;
	Character	*character;
	Matrix		*matrix;
	Cxform		*cxform;
	long		 status = NOTHING;
	long		 update = 0;
	char		*name;

#if PRINT&1
	if (action) printf("Prog %x (dl=%x): Frame N° %d/%d\n", this, this->dl, f, nbFrames-1);
#endif
        movie->buttons_updated = 0;

	for(ctrl = frames[f].controls; ctrl; ctrl = ctrl->next)
	{
		switch (ctrl->type)
		{
			case ctrlPlaceObject:
			case ctrlPlaceObject2:
				character = 0;
				matrix = 0;
				cxform = 0;
				name = "";
				if (ctrl->flags & placeHasCharacter) {
					character = ctrl->character;
				}
				if (ctrl->flags & placeHasMatrix) {
					matrix = &ctrl->matrix;
				}
				if (ctrl->flags & placeHasColorXform) {
					cxform = &ctrl->cxform;
				}
				if (ctrl->flags & placeHasName) {
					name = ctrl->name;
				}
				if (!ctrl->clipDepth) {	// Ignore
					dl->placeObject(gd,character, ctrl->depth, matrix, cxform, name);
					update = 1;
				}
				break;
			case ctrlRemoveObject:
				character = ctrl->character;

				if (!character) break;	// Should not happen

				dl->removeObject(gd, character, ctrl->depth);
				if (action) {
					character->reset();
					update = 1;
				}
				break;
			case ctrlRemoveObject2:
				character = dl->removeObject(gd,NULL, ctrl->depth);
				if (character && action) {
					character->reset();
					update = 1;
				}
				break;
		// Actions
			case ctrlDoAction:
				if (action) {
					status = doAction(gd, ctrl->actionRecords);
				}
				break;
			case ctrlBackgroundColor:
				if (action) {
					if (gd->setBackgroundColor(ctrl->color)) {
						dl->bbox.xmin = -32768;
						dl->bbox.ymin = -32768;
						dl->bbox.xmax =  32768;
						dl->bbox.ymax =  32768;
					}
				}
				break;
		}
	}
        if (movie->buttons_updated) {
            dl->updateButtons(movie);
        }

	if (status & GOTO) {
		if (nextFrame < nbFrames) {
			gotoFrame(gd,nextFrame);
			if (nextFrame != f)
			if (movieStatus == MoviePaused) runFrame(gd,nextFrame);
			update = 1;
		}
	}

#if PRINT&1
	if (action) printf("Frame N° %d ready\n", f);
#endif
	return update;
}

long
Program::nestedMovie(GraphicDevice *gd, Matrix *mat, Cxform *cxform)
{
	if (movieStatus == MoviePlay) {
		// Movie Beeing Played
		advanceFrame();
		if (currentFrame == 0) {
			dl->clearList();
		}
		runFrame(gd, currentFrame);
		if (nbFrames == 1) {
			pauseMovie();
		}
	}

	return (movieStatus == MoviePlay);
}

long
Program::processMovie(GraphicDevice *gd)
{
	int wakeUp = 0;

#if PRINT&1
	printf("Prog %x (dl=%x): Current = %d     Next = %d    Wait = %d  Status = %d\n", this, this->dl, currentFrame, nextFrame, movieWait, movieStatus);
#endif

	if (movieStatus == MoviePlay && movieWait == 0) {
		// Movie Beeing Played
		advanceFrame();
		if (currentFrame == 0) {
			dl->clearList();
		}
		wakeUp |= runFrame(gd, currentFrame);
		wakeUp |= dl->updateSprites();
		if (nextFrame == nbFrames) {
			if (nbFrames != totalFrames) {
				movieWait = 1;
			} else if ((settings & PLAYER_LOOP) == 0) {
				pauseMovie();
			}
		}
	} else {
		wakeUp |= dl->updateSprites();
	}

	if (wakeUp) {
		render = 1;
	}

	return (wakeUp || movieStatus == MoviePlay);
}

/* timer (ms) -1 = delete timer */
void setFlashTimer(struct timeval *tv, int time_ms)
{
    if (time_ms == -1) {
        tv->tv_sec = -1;
    } else {
        gettimeofday(tv,0);
        
        tv->tv_usec += time_ms*1000;
        while (tv->tv_usec > 1000000) {
            tv->tv_usec -= 1000000;
            tv->tv_sec++;
        }
    }
}

int checkFlashTimer(struct timeval *tv)
{
    struct timeval now;

    if (tv->tv_sec == -1) return 0;

    gettimeofday(&now,0);
    return (now.tv_sec > tv->tv_sec ||
            (now.tv_sec == tv->tv_sec && now.tv_usec >= tv->tv_usec));
}

/* bbox */
typedef struct {
    long x1,y1,x2,y2;
} ButtonBoundingBox;


static void button_bbox_func(void *id, long y, long start, long end)
{
    ButtonBoundingBox *h = (ButtonBoundingBox *) id;

    if (y < h->y1) h->y1 = y;
    if (y > h->y2) h->y2 = y;
    if (start < h->x1) h->x1 = start;
    if (end > h->x2) h->x2 = end;
}

void computeBBox(FlashMovie *movie, Rect *rect, DisplayListEntry *e)
{
    ButtonBoundingBox bb;

    bb.x1 = LONG_MAX;
    bb.y1 = LONG_MAX;
    bb.x2 = LONG_MIN;
    bb.y2 = LONG_MIN;
    
    e->character->getRegion(movie->gd,&e->renderMatrix,&bb,button_bbox_func);
    
    rect->xmin = bb.x1 / FRAC;
    rect->xmax = bb.x2 / FRAC;
    rect->ymin = bb.y1;
    rect->ymax = bb.y2;
}

void transform_coords(long *x_ptr,long *y_ptr, long cx, long cy, long dx, long dy)
{
    long x,y,x1,y1;
    x = *x_ptr;
    y = *y_ptr;

    x -= cx;
    y -= cy;

    if (dx < 0) {
        /* left */
        x1 = - x;
        y1 = y;
    } else if (dy < 0) {
        /* up */
        y1 = x;
        x1 = -y;
    } else if (dy > 0) {
        /* down */
        y1 = x;
        x1 = y;
    } else {
        /* right */
        x1 = x;
        y1 = y;
    }
        
    *x_ptr = x1;
    *y_ptr = y1;
}

typedef struct {
    FlashMovie *movie;
    DisplayListEntry *emin,*cur_focus;
    long dmin;
    long w,cx,cy,dx,dy;
} ButtonFocus;

static int button_focus(void *opaque, Program *prg, DisplayListEntry *e)
{
    ButtonFocus *h=(ButtonFocus *)opaque;
    Rect rect;
    long d,x,y;

    if (e != h->cur_focus) {
        computeBBox(h->movie,&rect,e);
        x = (rect.xmin + rect.xmax) / 2;
        y = (rect.ymin + rect.ymax) / 2;
        
        /* transform the coords so that the angular sector is directed to the right */
        transform_coords(&x,&y,h->cx,h->cy,h->dx,h->dy);
        
        /* inside it ? */
        if ( x >= 0 && 
             (y - x - h->w) <= 0 && 
             (y + x + h->w) >= 0) {
            d = x*x + y*y;
            
            if (d < h->dmin) {
                h->dmin = d;
                h->emin = e;
            }
        }
    }
    return 0;
}

DisplayListEntry *moveFocus(FlashMovie *movie, long dx, long dy, 
                            DisplayListEntry *cur_focus)
{
    Rect cur_rect;
    ButtonFocus h;

    h.movie = movie;
    h.dx = dx;
    h.dy = dy;

    computeBBox(movie,&cur_rect,cur_focus);
    /* center */
    h.cx = (cur_rect.xmin + cur_rect.xmax) / 2;
    h.cy = (cur_rect.ymin + cur_rect.ymax) / 2;
    
    /* width/2 of the 45 degrees angular sector */
    if (dy != 0) {
        /* for vertical displacement, we have a larger width */
        h.w = (cur_rect.xmax - cur_rect.xmin) / 2;
    } else {
        /* zero width for horizontal displacement */
        h.w = 0;
    }

    /* now we select the nearest button in the angular sector */
    h.dmin = LONG_MAX;
    h.emin = NULL;
    h.cur_focus = cur_focus;

    exploreButtons(movie, &h, button_focus);
    
    return h.emin;
}

static int button_newfocus(void *opaque, Program *prg, DisplayListEntry *e)
{
    * (DisplayListEntry **)opaque = e;
    return 2;
}

static int button_nextfocus(void *opaque, Program *prg, DisplayListEntry *e)
{
    static int found = 0;
    DisplayListEntry **focus;

    focus = (DisplayListEntry **)opaque;
    if (found) {
    	*focus = e;
	found = 0;
	return 2;
    }
    if (e == *focus) {
    	found = 1;
    }
    return 0;
}


/* XXX: should not be here (one level upper) */
long
Program::handleEvent(GraphicDevice *gd, FlashEvent *fe)
{
    ActionRecord	*action;
    Program		*prog;
    long		 status = 0;
    DisplayListEntry *cur_focus, *new_focus;
    long dx,dy;
    int			 refresh;

    refresh = 0;

    switch(fe->type) {

    case FeKeyRelease:
        if (movie->mouse_active == 0) {
            
            if (movie->cur_focus) {
		movie->cur_focus->owner->updateBoundingBox(movie->cur_focus);
                movie->cur_focus->renderState = stateOver;
		movie->cur_focus->owner->updateBoundingBox(movie->cur_focus);
            }
        }
        break;

    case FeKeyPress:

        movie->mouse_active = 0;

        /* find the button which has the focus */
        cur_focus = movie->cur_focus;

        if (fe->key == FeKeyEnter) {
            /* selection */
            if (cur_focus) {
                /* select the button */
		cur_focus->owner->updateBoundingBox(cur_focus);
                cur_focus->renderState = stateDown;
	        ((Button *)cur_focus->character)->updateButtonState(cur_focus);
		cur_focus->owner->updateBoundingBox(cur_focus);

                movie->scheduledEvent.type = FeKeyRelease;
                movie->scheduledEvent.key = FeKeyEnter;

                setFlashTimer(&movie->scheduledTime, 250); /* 250 ms down */
            }
        } else {
            /* displacement */

            if (cur_focus == NULL) {
                /* no current focus : set one */
                exploreButtons(movie, &cur_focus, button_newfocus);
                if (cur_focus) {
                    cur_focus->renderState = stateOver;
		    ((Button *)cur_focus->character)->updateButtonState(cur_focus);
		    cur_focus->owner->updateBoundingBox(cur_focus);
                }
                movie->cur_focus = cur_focus;
            } else {
                /* move the focus (test) */
                switch(fe->key) {
                case FeKeyNext:
                    /* Next available */
		    cur_focus->owner->updateBoundingBox(cur_focus);
                    cur_focus->renderState = stateUp;
		    ((Button *)cur_focus->character)->updateButtonState(cur_focus);
		    cur_focus->owner->updateBoundingBox(cur_focus);
                    exploreButtons(movie, &cur_focus, button_nextfocus);
                    if (cur_focus) {
                        cur_focus->renderState = stateOver;
		        ((Button *)cur_focus->character)->updateButtonState(cur_focus);
		        cur_focus->owner->updateBoundingBox(cur_focus);
                    }
                    movie->cur_focus = cur_focus;
                    dx = 0;
                    dy = 0;
		    break;
                case FeKeyUp:
                    dx = 0;
                    dy = -1;
                    break;
                case FeKeyDown:
                    dx = 0;
                    dy = 1;
                    break;
                case FeKeyLeft:
                    dx = -1;
                    dy = 0;
                    break;
                case FeKeyRight:
                    dx = 1;
                    dy = 0;
                    break;
                default:
                    /* should not happen */
                    dx = 0;
                    dy = 0;
                    break;
                }

                if (dx != 0 || dy != 0) {

                    new_focus = moveFocus(movie, dx, dy, cur_focus);
                    if (new_focus) {
			cur_focus->owner->updateBoundingBox(cur_focus);
                        cur_focus->renderState = stateUp;
			((Button *)cur_focus->character)->updateButtonState(cur_focus);
			cur_focus->owner->updateBoundingBox(cur_focus);

		        if (computeActions(movie, &prog, &action)) {
			    status |= prog->doAction(gd, action);
		        }
			    
                        new_focus->renderState = stateOver;
			((Button *)new_focus->character)->updateButtonState(new_focus);
                        movie->cur_focus = new_focus;
			new_focus->owner->updateBoundingBox(new_focus);
                    } else {
		    	return 0;
		    }
                }
            }
	    if (movie->cur_focus == NULL) return 0;
        }
        break;

    case FeMouseMove:
        movie->mouse_active = 1;
        movie->mouse_x = fe->x * FRAC;
        movie->mouse_y = fe->y * FRAC;
        dl->updateButtons(movie);
        break;

    case FeButtonPress:
        movie->mouse_active = 1;
        movie->button_pressed = 1;
        dl->updateButtons(movie);
        break;

    case FeButtonRelease:
        movie->mouse_active = 1;
        movie->button_pressed = 0;
        dl->updateButtons(movie);
        break;
        
    default:
        return 0;
    }

    if (computeActions(movie, &prog, &action)) {
        status |= prog->doAction(gd, action);
    }

    if (status & REFRESH) {
        status |= WAKEUP;
        refresh = 1;
    }
    if (status & GOTO) {
        if (nextFrame < nbFrames) {
		gotoFrame(gd, nextFrame);
		if (movieStatus == MoviePaused) runFrame(gd,nextFrame);
		refresh = 1;
	}
    }

    if (refresh) {
		dl->updateSprites();
		render = 1;
    }
    return (refresh || movieStatus == MoviePlay);
}

long
Program::doAction(GraphicDevice *gd, ActionRecord *action)
{
	long status = NOTHING;
        long f;
	char *target = "";
	long skip = 0;

	while(action)
	{
		if (skip) skip--;
		else
		switch (action->action)
		{
			case ActionRefresh:
#if PRINT&2
				printf("Prog %x : Refresh\n", this);
#endif
				status |= REFRESH;
				break;
			case ActionGotoFrame:
#if PRINT&2
				printf("Prog %x : GotoFrame %d\n", this, action->frameIndex);
#endif
				if (target[0] == 0) {
					if (action->frameIndex < nbFrames) {
						currentFrame = action->frameIndex;
						pauseMovie();
						status |= WAKEUP|GOTO;
					}
				}
				break;
			case ActionGetURL:
#if PRINT&2
				printf("Prog %x : GetURL %s target = %s\n", this, action->url, action->target);
#endif
                                {
                                    int len,level;
                                    len = strlen(action->target);
                                    
                                    if (len > 6 && memcmp(action->target,"_level", 6) == 0) {
                                        level = atoi(action->target + 6);
                                        loadNewSwf(movie, action->url, level);
                                    } else {
                                        if (movie->getUrl) {
                                            movie->getUrl(action->url, action->target, movie->getUrlClientData);
                                        }
                                    }
                                }
				break;
			case ActionNextFrame:
				nextFrame = currentFrame+1;
				movieStatus = MoviePlay;
				status |= WAKEUP;
				break;
			case ActionPrevFrame:
				nextFrame = currentFrame-1;
				status |= WAKEUP|GOTO;
				break;
			case ActionPlay:
#if PRINT&2
				printf("Prog %x : Play\n", this);
#endif
				if (target[0] == 0) {
					movieStatus = MoviePlay;
					if ((status & GOTO) == 0) {
						if (currentFrame == nextFrame) advanceFrame();
					}
					status |= WAKEUP;
				}
				break;
			case ActionStop:
#if PRINT&2
				printf("Prog %x : Stop\n", this);
#endif
				if (target[0] == 0) {
					movieStatus = MoviePaused;
					nextFrame = currentFrame;
				}
				break;
			case ActionToggleQuality:
				break;
			case ActionWaitForFrame:
				if (action->frameIndex >= nbFrames) {
					skip = action->skipCount;
				}
				break;
			case ActionSetTarget:
#if PRINT&2
				printf("Prog %x : SetTarget '%s'\n", this, action->target);
#endif
				target = action->target;
				break;
			case ActionGoToLabel:
#if PRINT&2
				printf("Prog %x : GotoFrame '%s'\n", this, action->frameLabel);
#endif
                                f = searchFrame(gd, action->frameLabel, target);
                                if (f >= 0) {
				    currentFrame = f;
				    pauseMovie();
                                    status |= WAKEUP|GOTO;
                                } else {
                                    status |= REFRESH;
                                }
				break;
		}
		action = action->next;
	}
	return status;
}

void
Program::setCurrentFrameLabel(char *label)
{
    frames[loadingFrame].label = label;
}

void
Program::rewindMovie()
{
	currentFrame = 0;
	nextFrame = 0;
}

void
Program::pauseMovie()
{
	movieStatus = MoviePaused;
	nextFrame = currentFrame;
}

void
Program::continueMovie()
{
	movieStatus = MoviePlay;
}

void
Program::nextStepMovie()
{
	if (movieStatus == MoviePaused) {
		advanceFrame();
	}
}

void
Program::advanceFrame()
{
	currentFrame = nextFrame;
	nextFrame = currentFrame+1;
	if (currentFrame == nbFrames) {
		currentFrame = 0;
		nextFrame = 0;
		movieStatus = MoviePlay;
	}
}

void
Program::addControlInCurrentFrame(Control *ctrl)
{
	Control *c;

	ctrl->next = 0;
	if (frames[loadingFrame].controls == 0) {
		frames[loadingFrame].controls = ctrl;
	} else {
		for(c = frames[loadingFrame].controls; c->next; c = c->next);
		c->next = ctrl;
	}
}

void
Program::modifySettings(long flags)
{
	settings = flags;
}

long
Program::searchFrame(GraphicDevice *gd, char *label, char *target)
{
	long f;
        DisplayListEntry *e;
        Program *prg;

	// Current movie
	if (target[0] == 0) {
		for(f=0; f < nbFrames; f++)
		{
		    if (frames[f].label && !strcmp(label,frames[f].label)) {
			return f;
		    }
		}
	}

	// Kludge !!!
	for (e = dl->list; e; e = e->next) {
	    if (e->character->isSprite()) {
		prg = ((Sprite *)e->character)->program;
		f = prg->searchFrame(gd,label,"");
		if (f >= 0 && f < prg->nbFrames) {
		    prg->dl->updateBoundingBox(e);
		    prg->gotoFrame(gd, f);
		    prg->nextFrame = f;
		    prg->dl->updateBoundingBox(e);
		    return -1;
		}
	    }
	}

	return -1;
}

void loadNewSwf(FlashMovie *movie, char *url, int level)
{
    CInputScript *s,*prev,**l;

    if (movie->getSwf == NULL) return;

    for(s = movie->main, prev = 0; s != NULL; prev = s, s = s->next) {
    	if (s->level == level) {
		// Mark movie to be deleted
		s->level = -1;
		break;
	}
    }

    //printf("Unload movie @ %d\n", level);

    if (*url == 0) return;	// Just UnloadMovie

    s = new CInputScript(level);
    if (s == NULL) return;

    /* insert it in the right order */
    l = &movie->main;
    while (*l != NULL && (*l)->level < level) l = &(*l)->next;
    s->next = *l;
    *l = s;

    // Notify the external loader of a new movie to load
    movie->getSwf(url, level, movie->getSwfClientData);
}
