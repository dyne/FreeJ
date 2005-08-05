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
#ifndef _SOUND_H_
#define _SOUND_H_

#define GET_SOUND_RATE_CODE(f) (((f)&0x0c)>>2)
#define GET_SOUND_FORMAT_CODE(f) (((f)&0xf0)>>4)

struct SoundList {
	long	 rate;
	long	 stereo;
	long	 sampleSize;
	long	 nbSamples;
	long	 remaining;
	char	*current;
	char	*original;
	char	*originalMp3;
	long	 remainingMp3;
	char	*currentMp3;
	struct mad_stream mp3Stream;
	struct mad_frame mp3Frame;
	struct mad_synth mp3Synth;
	long	 skipSize;

	SoundList *next;
	SoundList *prev;

};

class Sound : public Character {
	long	 	 format;
	long		 soundRate;	// In hz
	long		 stereo;	// True if stereo sound
	long		 sampleSize;	// 1 or 2 bytes

	char		*samples;		// Array of samples
	long		 nbSamples;
	long		 soundSize;
	int		 playbackStarted;
	SoundList	*sound;		// The sound when playing

public:
	Sound(long id);
	~Sound();
	void    	 setSoundFlags(long f);
	char		*setNbSamples(long n);
	char		*setBuffer(long n);
	char		*resetBuffer(long n);
	void		 setSoundSize(long n);
	void		 setPlaybackStarted();
	void		 setSound(SoundList *sl);

	long		 getFormat();
	long		 getRate();
	long		 getChannel();
	long		 getNbSamples();
	long		 getSampleSize();
	char		*getSamples();
	long		 getSoundSize();
	long		 getPlaybackStarted();
	SoundList	 *getSound();
};

class SoundMixer {

	SoundList	*list;

// Class variables
static  long		 dsp;		// Descriptor for /dev/dsp
static  char *		 buffer;	// DMA buffer
static	long		 blockSize;
static	long		 nbInst;	// Number of instances

	// Sound Device Capabilities
static	long		 soundRate;	// In hz
static	long		 stereo;	// True if stereo sound
static	long		 sampleSize;	// 1 or 2 bytes

public:
	SoundMixer(char*);
	~SoundMixer();

	void		 startSound(Sound *sound);	// Register a sound to be played
	void		 stopSounds();		// Stop every current sounds in the instance

	long		 playSounds();		// Actually play sounds of all instances
	long		 fillSoundBuffer(SoundList *, char *buffer, long bufferSize); // Fill sound buffer
private:
	int SoundMixer::Mp3Scale(mad_fixed_t sample);
	void SoundMixer::Mp3Decompress(SoundList *sl);
	void SoundMixer::uninitMp3Sounds(SoundList *sl);
};

#endif /* _SOUND_H_ */

