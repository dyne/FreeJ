/*///////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////// */
#ifndef _FLASH_H_
#define _FLASH_H_

#define PLUGIN_NAME "Shockwave Flash"
#define FLASH_VERSION_STRING "Version 0.4.10"

/* Flags to pass to FlashExec */
#define FLASH_WAKEUP 0x01
#define FLASH_EVENT  0x02
#define FLASH_CMD    0x04

/* Mask to extract commands */
#define FLASH_CMD_MASK 0xf0
/* Commands */
#define FLASH_STOP     0x10	/* Pause the movie */
#define FLASH_CONT     0x20	/* Continue the movie after pause */
#define FLASH_REWIND   0x30	/* Rewind the movie and pause */
#define FLASH_STEP     0x40	/* Frame by frame operation */

/* return codes of FlashExec */
#define FLASH_STATUS_WAKEUP   0x01    /* FlashExec must be called again after a given time */

struct FlashInfo {
	long frameRate;
	long frameCount;
	long frameWidth;
	long frameHeight;
	long version;
};

/* Player settings */
#define PLAYER_LOOP	(1<<0)
#define PLAYER_QUALITY	(1<<1)
#define PLAYER_MENU	(1<<2)

/* Parser status */
#define FLASH_PARSE_ERROR     0
#define FLASH_PARSE_START     1
#define FLASH_PARSE_NEED_DATA 2
#define FLASH_PARSE_EOM       4
#define FLASH_PARSE_WAKEUP    8
#define FLASH_PARSE_OOM       16      /* Out Of Memory */

typedef void *FlashHandle;

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

enum FlashEventType {
    FeNone,
    FeMouseMove,
    FeButtonPress,
    FeButtonRelease,
    FeRefresh,
    FeKeyPress,
    /* internal events */
    FeKeyRelease,
};

enum FlashKey {
    FeKeyUp = 1,
    FeKeyDown,
    FeKeyLeft,
    FeKeyRight,
    FeKeyEnter,
    FeKeyNext
};



typedef struct FlashEvent {
    enum FlashEventType type;
    int		 x,y;		/* Mouse coordinates, 
                                   relative to upper-left window corner */
    enum FlashKey key;
} FlashEvent;

typedef struct FlashDisplay {
    void *pixels;
    int bpl;      /* bytes per line */
    int width;
    int height;
    int depth;
    int bpp;
    int flash_refresh;
    /* Clipping region */
    int clip_x, clip_y;
    int clip_width, clip_height;
} FlashDisplay;

extern FlashHandle FlashNew();
extern void FlashGetInfo(FlashHandle fh, struct FlashInfo *fi);
extern long FlashGraphicInit(FlashHandle fh, FlashDisplay *fd);
extern void FlashSoundInit(FlashHandle fh, char *device);
extern int  FlashParse(FlashHandle fh, int level, char *data, long size);
extern long FlashExec(FlashHandle fh, long flag, FlashEvent *fe, struct timeval *wakeDate);
extern void FlashClose(FlashHandle fh);
extern void FlashSetGetUrlMethod(FlashHandle flashHandle, void (*getUrl)(char *, char *, void *), void *);
extern void FlashSetGetSwfMethod(FlashHandle flashHandle, void (*getSwf)(char *url, int level, void *clientData), void *clientData);
extern void FlashSetCursorOnOffMethod(FlashHandle flashHandle, void (*cursorOnOff)(int , void *), void *clientData);
extern void FlashZoom(FlashHandle fh, int zoom);
extern void FlashOffset(FlashHandle fh, int x, int y);
extern void FlashSettings(FlashHandle fh, long settings);

#if defined(__cplusplus) || defined(c_plusplus)
};
#endif

#endif /* _FLASH_H_ */
