#ifndef _SWF_H_
#define _SWF_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <assert.h>
#include <limits.h>


#ifdef DUMP
#include "bitstream.h"
#endif

#include "flash.h"

extern int debug;

#include <stdint.h>

// Global Types
typedef uint32_t U32, *P_U32, **PP_U32;
typedef int32_t S32, *P_S32, **PP_S32;
typedef uint16_t U16, *P_U16, **PP_U16;
typedef int16_t S16, *P_S16, **PP_S16;
typedef uint8_t U8, *P_U8, **PP_U8;
typedef int8_t S8, *P_S8, **PP_S8;
typedef int32_t SFIXED, *P_SFIXED;
typedef int32_t SCOORD, *P_SCOORD;
typedef uint32_t BOOL;

#define ZOOM(v,f) ((v)/(f))

#include "matrix.h"
#include "cxform.h"
#include "rect.h"

#include <sys/time.h>
#define ST struct timeval t1,t2;
#define START gettimeofday(&t1,0)
#define STOP(msg) gettimeofday(&t2,0); printf("%s Delta = %d ms\n", msg, (t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000); fflush(stdout);

// Start Sound Flags
enum {
	soundHasInPoint		= 0x01,
	soundHasOutPoint	= 0x02,
	soundHasLoops		= 0x04,
	soundHasEnvelope	= 0x08

	// the upper 4 bits are reserved for synchronization flags
};

// Flags for Sound Format
enum SounfFlags {
	soundIsStereo		= 0x01,
	soundIs16bit		= 0x02,
	soundIsADPCMCompressed	= 0x10
};

// Flags for defining Button States
enum ButtonState {
	stateHitTest = 0x08,
	stateDown    = 0x04,
	stateOver    = 0x02,
	stateUp      = 0x01
};

// Actions
enum Action {
		// Internal actions
		ActionRefresh		= 0x00,
		ActionPlaySound		= 0x01,
		// Normal actions
		ActionGotoFrame		= 0x81,
		ActionGetURL		= 0x83,
		ActionNextFrame		= 0x04,
		ActionPrevFrame		= 0x05,
		ActionPlay		= 0x06,
		ActionStop		= 0x07,
		ActionToggleQuality	= 0x08,
		ActionStopSounds	= 0x09,
		ActionWaitForFrame	= 0x8a,
		ActionSetTarget		= 0x8b,
		ActionGoToLabel		= 0x8c
};

class Sound;

struct ActionRecord {
	Action			 action;

	// GotoFrame  & WaitForFrame
	long			 frameIndex;

	// GetURL
	char			*url;
	char			*target;

	// GotoLabel
	char			*frameLabel;

	// WaitForFrame
	long			 skipCount;

	// Sound
	Sound			*sound;

	struct ActionRecord	*next;
	
	ActionRecord() {
		frameLabel = 0;
		url = 0;
		target = 0;
		sound = 0;
	};

	~ActionRecord() {
		if (frameLabel) free(frameLabel);
		if (url) free(url);
		if (target) free(target);
	};
};

enum FontFlags {
	fontUnicode   = 0x20,
	fontShiftJIS  = 0x10,
	fontANSI      = 0x08,
	fontItalic    = 0x04,
	fontBold      = 0x02,
	fontWideCodes = 0x01
};

enum TextFlags {
	isTextControl = 0x80,

	textIsLarge   = 0x70,
	textHasFont   = 0x08,
	textHasColor  = 0x04,
	textHasYOffset= 0x02,
	textHasXOffset= 0x01
};

#ifndef NULL
#define NULL 0
#endif

// Tag values that represent actions or data in a Flash script.
enum
{ 
    stagEnd 			= 0,
    stagShowFrame 		= 1,
    stagDefineShape		= 2,
    stagFreeCharacter 		= 3,
    stagPlaceObject 		= 4,
    stagRemoveObject 		= 5,
    stagDefineBits 		= 6,
    stagDefineButton 		= 7,
    stagJPEGTables 		= 8,
    stagSetBackgroundColor	= 9,
    stagDefineFont		= 10,
    stagDefineText		= 11,
    stagDoAction		= 12,
    stagDefineFontInfo		= 13,
    stagDefineSound		= 14,	// Event sound tags.
    stagStartSound		= 15,
    stagStopSound		= 16,
    stagDefineButtonSound	= 17,
    stagSoundStreamHead		= 18,
    stagSoundStreamBlock	= 19,
    stagDefineBitsLossless	= 20,	// A bitmap using lossless zlib compression.
    stagDefineBitsJPEG2		= 21,	// A bitmap using an internal JPEG compression table.
    stagDefineShape2		= 22,
    stagDefineButtonCxform	= 23,
    stagProtect			= 24,	// This file should not be importable for editing.

    // These are the new tags for Flash 3.
    stagPlaceObject2		= 26,	// The new style place w/ alpha color transform and name.
    stagRemoveObject2		= 28,	// A more compact remove object that omits the character tag (just depth).
    stagDefineShape3		= 32,	// A shape V3 includes alpha values.
    stagDefineText2		= 33,	// A text V2 includes alpha values.
    stagDefineButton2		= 34,	// A button V2 includes color transform, alpha and multiple actions
    stagDefineBitsJPEG3		= 35,	// A JPEG bitmap with alpha info.
    stagDefineBitsLossless2 	= 36,	// A lossless bitmap with alpha info.
    stagDefineSprite		= 39,	// Define a sequence of tags that describe the behavior of a sprite.
    stagNameCharacter		= 40,	// Name a character definition, character id and a string, (used for buttons, bitmaps, sprites and sounds).
    stagFrameLabel			= 43,	// A string label for the current frame.
    stagSoundStreamHead2	= 45,	// For lossless streaming sound, should not have needed this...
    stagDefineMorphShape	= 46,	// A morph shape definition
    stagDefineFont2		= 48,

    notEnoughData		= 0xffff,	// Special code
};

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

extern int shape_size,shape_nb,shaperecord_size,shaperecord_nb,style_size,style_nb;

typedef void (*ScanLineFunc)(void *id, long y, long start, long end);

class Bitmap;
struct FlashMovie;


extern "C" {
#include "jpeglib.h"
};
extern "C" {
#include "zlib.h"
};

extern "C" {
#include "mad.h"
};

#include "graphic.h"
#include "character.h"
#include "bitmap.h"
#include "shape.h"
#include "displaylist.h"
#include "sound.h"
#include "button.h"
#include "font.h"
#include "text.h"
#include "adpcm.h"
#include "program.h"
#include "sprite.h"
#include "script.h"
#include "movie.h"

#endif /* _SWF_H_ */
