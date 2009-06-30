/*  FreeJ
 *  (c) Copyright 2008 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#ifndef __FREEJ_AUDIO_H__
#define __FREEJ_AUDIO_H__

#include <unicap.h>
#include <unicap_status.h>

#include <sys/types.h>

#include <context.h>


class AudioLayer: public Layer {

 public:
  AudioLayer();
  ~AudioLayer();

  bool open(const char *devfile);
  bool init(Context *freej);
  bool init(Context *freej, int width, int height);
  void *feed();
  void close();

  unsigned int Samplerate;
  unsigned int BufferLength;

 private:

  void AudioCallback_i(unsigned int);
  static void AudioCallback(void *, unsigned int);

  float *m_AudioBuffer;
  float *m_JackBuffer;
  unsigned int m_ProcessPos;
  unsigned int m_ProcessLength;
  bool   m_Processing;
  pthread_mutex_t* m_Mutex;
  float *m_Buffer;
};

#endif
