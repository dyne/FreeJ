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
#include "movie.h"

FlashMovie::FlashMovie() 
{
	gd = NULL;
	sm = NULL;
	getSwf = NULL;
	getUrl = NULL;
	cursorOnOff = NULL;
	buttons_updated = 0;
	scheduledTime.tv_sec = -1;
	cur_focus = NULL;
	lost_over = NULL;
	msPerFrame = 0;

	/* mouse handling */
	mouse_active = 0;
	mouse_x = -1;
	mouse_y = -1;
	button_pressed = 0;
	refresh = 1;
}
    
FlashMovie::~FlashMovie() 
{
	CInputScript *n;

	while (main != NULL) {
		n = main->next;
		delete main;
		main = n;
	}

	if (gd) delete gd;
	if (sm) delete sm;
}

int
FlashMovie::processMovie(GraphicDevice *gd, SoundMixer *sm)
{
	CInputScript *script;
	int wakeUp = 0;

	if (sm && sm->playSounds()) {
		wakeUp = 1;
	}
	for (script = this->main; script != NULL; script = script->next) {
		if (script->program == NULL) continue;
		if (script->program->nbFrames == 0) continue;
		if (script->program->processMovie(gd,sm)) {
			wakeUp = 1;
		}
	}
	renderMovie();
	return wakeUp;
}

int
FlashMovie::handleEvent(GraphicDevice *gd, SoundMixer *sm, FlashEvent *event)
{
	int wakeUp = 0;

	if (sm && sm->playSounds()) {
		wakeUp = 1;
	}
	if (this->main == 0) return 0;
	if (this->main->program == 0) return 0;
	if (this->main->program->handleEvent(gd, sm, event)) {
		wakeUp = 1;
	}
	renderMovie();
	return wakeUp;
}

/* current focus bigger and translated if needed */
void
FlashMovie::renderFocus()
{
	Rect rect,boundary;
	Matrix mat;

	if (mouse_active || !cur_focus) return;

	/* rect is the bbox in screen coordinates */

        // Compute the bounding box in screen coordinates
        cur_focus->character->getBoundingBox(&boundary,cur_focus);
        mat = (*gd->adjust) * cur_focus->renderMatrix;
        transformBoundingBox(&rect, &mat, &boundary, 1);

	gd->drawBox(rect.xmin, rect.ymin, rect.xmax, rect.ymax);
}

void 
FlashMovie::renderMovie()
{
	CInputScript *script,*prev,*next;
	Rect clipping;
	Matrix identity;

	clipping.reset();

	// First pass to update the clipping region
	for (script = this->main; script != NULL; script = script->next) {
		if (script->level == -1) {
			clipping.xmin = -32768;
			clipping.ymin = -32768;
			clipping.xmax =  32767;
			clipping.ymax =  32767;
			continue;
		}
		if (script->program == NULL) continue;
		if (script->program->dl->bbox.xmin == LONG_MAX) continue;
		transformBoundingBox(&clipping, &identity, &script->program->dl->bbox, 0);
		script->program->render = 0;
	}

	if (clipping.xmin == LONG_MAX) return;

	// Update the clipping region
	gd->updateClippingRegion(&clipping);
	gd->clearCanvas();

	// Second pass to render the movie
	for (script = this->main; script != NULL; script = script->next) {
		if (script->level == -1) continue;
		if (script->program == NULL) continue;
		script->program->dl->render(gd);
	}
	renderFocus();

	// Final pass to delete some movies
	script = this->main;
	prev = 0;
	while (script != NULL) {
		if (script->level == -1) {
			next = script->next;
			if (prev == 0) {
				this->main = next;
			} else {
				prev->next = next;
			}
			delete script;
			script = next;
		} else {
			prev = script;
			script = script->next;
		}
	}
}
