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
#include "graphic16.h"
#include "graphic24.h"
#include "graphic32.h"

#ifdef RCSID
static char *rcsid = "$Id$";
#endif

// Interface with standard C
extern "C" {

FlashHandle
FlashNew()
{
	FlashMovie *fh;

	fh = new FlashMovie;

        fh->main = new CInputScript;

	return (FlashHandle)fh;
}

int
FlashParse(FlashHandle flashHandle, int level, char *data, long size)
{
	FlashMovie *fh;
	CInputScript *script;
	int status = FLASH_PARSE_ERROR;

	fh = (FlashMovie *)flashHandle;

	for(script = fh->main; script != NULL; script = script->next) {
		if (script->level == level) {
			status = script->ParseData(fh, data, size);

			if (status & FLASH_PARSE_START) {
				fh->msPerFrame = 1000/fh->main->frameRate;
				script->program->rewindMovie();
			}
			break;
		}
	}

	return status;
}

void
FlashGetInfo(FlashHandle flashHandle, struct FlashInfo *fi)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fi->version = fh->main->m_fileVersion;
	fi->frameRate = fh->main->frameRate;
	fi->frameCount = fh->main->frameCount;
	fi->frameWidth = fh->main->frameRect.xmax - fh->main->frameRect.xmin;
	fi->frameHeight = fh->main->frameRect.ymax - fh->main->frameRect.ymin;
}

long FlashGraphicInit(FlashHandle flashHandle, FlashDisplay *fd)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	switch (fd->bpp) {
		case 4:
			fh->gd = new GraphicDevice32(fd);
			break;
		case 3:
			fh->gd = new GraphicDevice24(fd);
			break;
		case 2:
			fh->gd = new GraphicDevice16(fd);
			break;
		default:
			fprintf(stderr, "Unsupported depth\n");
	}

	fh->gd->setMovieDimension(fh->main->frameRect.xmax - fh->main->frameRect.xmin,
				  fh->main->frameRect.ymax - fh->main->frameRect.ymin);

	return 1;	// Ok
}

void
FlashSoundInit(FlashHandle flashHandle, char *device)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->sm = new SoundMixer(device);
}

void
FlashZoom(FlashHandle flashHandle, int zoom)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->gd->setMovieZoom(zoom);
}

void
FlashOffset(FlashHandle flashHandle, int x, int y)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->gd->setMovieOffset(x,y);
}

long
FlashExec(FlashHandle flashHandle, long flag, 
          FlashEvent *fe, struct timeval *wakeDate)
{
	FlashMovie *fh;
	long wakeUp = 0;

	fh = (FlashMovie *)flashHandle;

	if (fh->main == NULL) return 0; // Not ready
	if (fh->main->program == NULL) return 0; // Not ready
	if (fh->main->program->nbFrames == 0) return 0; // Still not ready
	if (fh->gd == 0) return 0;

	switch (flag & FLASH_CMD_MASK) {
		case FLASH_STOP:
			fh->main->program->pauseMovie();
			wakeUp = 0;
			break;
		case FLASH_CONT:
			fh->main->program->continueMovie();
			wakeUp = FLASH_STATUS_WAKEUP;
			break;
		case FLASH_REWIND:
			fh->main->program->rewindMovie();
			wakeUp = 0;
			break;
		case FLASH_STEP:
			fh->main->program->nextStepMovie();
			wakeUp = 0;
			break;
	}

	if (flag & FLASH_WAKEUP) {
		// Compute next wakeup time
		gettimeofday(wakeDate,0);
		wakeDate->tv_usec += fh->msPerFrame*1000;
		if (wakeDate->tv_usec > 1000000) {
			wakeDate->tv_usec -= 1000000;
			wakeDate->tv_sec++;
		}
                
		// Play frame
                wakeUp = fh->processMovie(fh->gd, fh->sm);
	}

        if (checkFlashTimer(&fh->scheduledTime)) {
            if (fh->handleEvent(fh->gd, fh->sm, &fh->scheduledEvent)) {
                wakeUp = 1;
            }
            
            setFlashTimer(&fh->scheduledTime, -1);
        }

	if (flag & FLASH_EVENT) {
            wakeUp = fh->handleEvent(fh->gd, fh->sm, fe);
            if (wakeUp) {
                /* Wake up at once, except for mouse move (40 ms after) */
                gettimeofday(wakeDate,0);
                if (fe->type == FeMouseMove) {
                    wakeDate->tv_usec += 40*1000;
                    if (wakeDate->tv_usec > 1000000) {
			wakeDate->tv_usec -= 1000000;
			wakeDate->tv_sec++;
                    }
                }
            }
	}

	return wakeUp || (fh->scheduledTime.tv_sec != -1);
}

void FlashSetGetSwfMethod(FlashHandle flashHandle, void (*getSwf)(char *url, int level, void *clientData), void *clientData)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->getSwf = getSwf;
	fh->getSwfClientData = clientData;
}


void
FlashSetCursorOnOffMethod(FlashHandle flashHandle, void (*cursorOnOff)(int , void *), void *clientData)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->cursorOnOff = cursorOnOff;
	fh->cursorOnOffClientData = clientData;
}

void
FlashSetGetUrlMethod(FlashHandle flashHandle, void (*getUrl)(char *, char *, void *), void *clientData)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->getUrl = getUrl;
	fh->getUrlClientData = clientData;
}

void
FlashClose(FlashHandle flashHandle)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	delete fh;
}

void
FlashSettings(FlashHandle flashHandle, long settings)
{
	FlashMovie *fh;

	fh = (FlashMovie *)flashHandle;

	fh->main->program->modifySettings( settings );
}

int shape_size,shape_nb,shaperecord_size,shaperecord_nb,style_size,style_nb;

void flash_dump(void)
{
    printf("flash: shape_size=%d (nb=%d)\n",shape_size,shape_nb);
    printf("flash: shaperecord_size=%d (nb=%d)\n",shaperecord_size,shaperecord_nb);
    printf("flash: style_size=%d (nb=%d)\n",style_size,style_nb);
}

}; /* end of extern C */
