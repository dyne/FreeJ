/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani <kysucix@dyne.org>
 *                2007 Denis Roio       <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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
 * "$Id$"
 *
 */

#ifndef __VIDEO_ENCODER_H__
#define __VIDEO_ENCODER_H__

#include <SDL2/SDL.h>
#include <config.h>

#include <jsync.h>
#include <jutils.h>

#include <linklist.h>

#include <screen.h>

#include <ringbuffer.h>

#include <shout/shout.h>

#include <iostream>

class Context;
class AudioCollector;
class FPS;
class ViewPort;

/**
 * Abstract class describing the general interface of a VideoEncoder
 *
 * 
 * Method implemented are:
 *   - VideoEncoder::set_output_name()
 *   - VideoEncoder::set_sdl_surface()
 *
 *   Virtual method to be implemented in an implementation:
 *   - VideoEncoder::init
 *   - VideoEncoder::set_encoding_parameter
 *   - VideoEncoder::write_frame
 *
 * @brief Encodes output from Screen
*/


//class VideoEncoder: public Entry,public JSyncThread{
class VideoEncoder: public Entry, public JSyncThread {

 public:
  
  VideoEncoder ();
  virtual ~VideoEncoder ();

  double getStreamRate();

  virtual bool init (ViewPort *scr) = 0;  ///< pure virtual function to implement
  virtual int encode_frame ()       = 0;  ///< pure virtual function to implement

  void thread_setup(); ///< setup data needed in encoding thread
  void thread_loop(); ///< encoding thread main loop
  void thread_teardown(); ///< ending phase of encoding thread

  bool set_filedump(const char *filename); ///< start to dump to filename, call with NULL to stop
  bool filedump_close(); ///stops to dump in the file and close it
  char filedump[512]; ////< filename to which encoder is writing dump

  char *status; ///< string updated with encoder status

  int video_quality; ///< quality of video encoding: range 0-100
  int video_bitrate; ///< video encoding bitrate (default is 0: VBR on the quality value)
  int audio_quality; ///<  quality of audio encoding: range 0-100
  int audio_bitrate; ///< audio encoding bitrate (default is 0: VBR on the quality value)

  int audio_kbps; ///< encoded audio, kilobit per second 
  int video_kbps; ///< encoded video, kilobit per second
  int bytes_encoded; ///< encoded bytes in total in the last encoding pass

  // now in jsync
  // bool quit; ///< flag it up if encoder has to quit

  bool active; ////< flag to de/activate the encoder

  bool initialized; ///< true if encoder had been initialized
  bool write_to_disk; ///< true if encoder should write to a file
  bool write_to_stream; ///< true if encoder should write to a stream
  
  bool use_audio; ///< true if the encoded data should include also audio
  AudioCollector *audio; ///< holding the pointer to the @AudioCollector class
  float *audio_buf; ///< audio buffer (local copy for the encoder)

  bool streaming; ///< set true when streaming by js start_stream()

  ringbuffer_t *ringbuffer; ///< FIFO ringbuffer pipe from Jack

  FPS *fps;

  shout_t *ice;

  ViewPort *screen;

  void *enc_y;
  void *enc_u;
  void *enc_v;
  void *enc_yuyv;

 private:
  FILE *filedump_fd;
//   char encbuf[1024*128];
//   char encbuf[1024*2096];
  char *encbuf;
  struct timeval m_ActualTime, m_OldTime, m_lastTime;
  double m_StreamRate;
  int 	 m_Streamed;
  double m_ElapsedTime;


};

#endif
