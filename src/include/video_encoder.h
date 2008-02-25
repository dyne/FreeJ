/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani <kysucix@dyne.org>
 *                2007 Denis Rojo       <jaromil@dyne.org>
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
 * "$Id$"
 *
 */

#ifndef __VIDEO_ENCODER_H__
#define __VIDEO_ENCODER_H__

#include <SDL.h>
#include <config.h>

#include <jsync.h>
#include <jutils.h>

#include <linklist.h>

#include <screen.h>

#include <ringbuffer.h>

#include <shout/shout.h>

class Context;

/**
 * Class describing the general interface of an encoder
 * Method implemented are:
 *   - VideoEncoder::set_output_name()
 *   - VideoEncoder::set_sdl_surface()
 *
 *   Virtual method to be implemented in an implementation:
 *   - VideoEncoder::init
 *   - VideoEncoder::set_encoding_parameter
 *   - VideoEncoder::write_frame
 *
 * @brief Layer parent abstract class
*/


//class VideoEncoder: public Entry,public JSyncThread{
class VideoEncoder: public Entry, public JSyncThread {

 public:
  
  VideoEncoder ();
  virtual ~VideoEncoder ();

  virtual bool init (Context *_env)                          = 0;  
  virtual int encode_frame ()                                = 0;
  virtual bool feed_video()                                  = 0;

  void run();
  bool cafudda();
  
  
  bool set_filedump(char *filename); ///< start to dump to filename, call with NULL to stop
  char filedump[512]; ////< filename to which encoder is writing dump

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

  bool streaming; ///< set true when streaming by js start_stream()

  ringbuffer_t *ringbuffer; ///< FIFO ringbuffer pipe from Jack

  Context *env;

  shout_t *ice;



 private:
  FILE *filedump_fd;
  char encbuf[1024*128];

};

#endif
