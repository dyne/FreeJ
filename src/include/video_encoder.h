/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
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
#include <linklist.h>
#include <jutils.h>
#include <jsync.h>
#include <screen.h>
#include <pipe.h>

#include <portaudio.h>

#define USE_PORTAUDIO_V19

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
class VideoEncoder: public Entry {

 public:
  
  VideoEncoder (char *output_filename);
  virtual ~VideoEncoder ();
  
  virtual void set_encoding_parameter ()                     = 0;
  virtual bool set_video_quality (int quality)               = 0;
  virtual bool set_audio_quality (double quality)               = 0;
  virtual bool write_frame ()                                = 0;
  virtual bool has_finished_frame ()                         = 0;
  virtual bool isStarted ()                                  = 0;
  virtual bool init (Context *_env, ViewPort *viewport)      = 0;
  
  bool set_output_name (char * output_filename);
  bool set_sdl_surface (SDL_Surface *surface);
  char *get_filename();

  void stream_it(bool s);
  void handle_audio(bool audio);
  bool is_stream();
  bool stop_audio_stream();
  bool start_audio_stream();

  bool is_audio_inited(); // really bad XXX

  
  // visible for the sub-classes
 protected:

  char *filename;
  bool started;

  SDL_Surface *surface;
  
  Context *env;

  bool _init(Context *_env);

  /* fifo to handle input audio */
  Pipe *coda;
  bool use_audio;
  bool write_to_disk;
  bool stream;
  double sample_rate;

  private:

  bool init_audio();
  bool audio_started;

#ifdef USE_PORTAUDIO_V19
  PaStream *pa_audioStream;
#else
  PortAudioStream   *pa_audioStream;	
#endif


};

#endif
