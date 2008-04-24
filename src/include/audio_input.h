/*  FreeJ
 *
 *  jack audio pipe
 *
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __JACK_AUDIO_H__
#define __JACK_AUDIO_H__

#include <inttypes.h>

#include <audio_jack.h>
#include <ringbuffer.h>

//#define SAMPLERATE 44100
//#define MAX_FRAMESPERBUFFER 1024*8




class AudioInput {
  
 public:
  
  AudioInput();
  ~AudioInput();

  bool init();
  bool start();
  void stop();

  //  void set_format(int chans, int srate); ///< set channels and samplerate

  int cafudda(); ///< called by Context to publish buffer in data


  int16_t *input;  ///< public audio buffer recorder
  int16_t *output; ///< public audio buffer playback

  //  int framesperbuffer; ///< desired frames per buffer chunk made available
  //  int frames;          ///< number of frames returned by last callback
  //  int bytesize;     ///< number of bytes returned by last callback

  int sample_rate;  ///< sample rate for soundcard initialization
  int channels;        ///< mono/stereo setting for soundcard
  int inputframes;

  bool initialized;
  bool started;

  // private, only used by callback and cafudda
  ringbuffer_t *input_pipe;
  ringbuffer_t *output_pipe;

 private:
  
  AudioCollector *audio;

};


#endif
