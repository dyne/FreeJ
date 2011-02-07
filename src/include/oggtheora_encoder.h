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

#ifndef __OGGTHEORA_ENCODER_h__
#define __OGGTHEORA_ENCODER_h__

#include <math.h>
#include <wave.h>
#include <config.h>
#include <linklist.h>
#include <video_encoder.h>
#include <audio_jack.h>
#include <fstream>
#ifdef WITH_OGGTHEORA

#include <theorautils.h>

/*
extern "C" {
#ifdef HAVE_LIBAVCODEC_AVCODEC_H
#   include <libavcodec/avcodec.h>
#elif defined(HAVE_FFMPEG_AVCODEC_H)
#   include <ffmpeg/avcodec.h>
#else
#   include <avcodec.h>
#endif

#ifdef HAVE_LIBAVFORMAT_AVFORMAT_H
#   include <libavformat/avformat.h>
#elif defined(HAVE_FFMPEG_AVFORMAT_H)
#   include <ffmpeg/avformat.h>
#else
#   include <avformat.h>
#endif
}
*/


// #define AUDIO_BUFFER_SIZE 3528 // sample_rate * channel / fps / bytes of sample format

class Context;

class OggTheoraEncoder: public VideoEncoder {

 public:
  
  OggTheoraEncoder();
  ~OggTheoraEncoder();
  
  bool init(ViewPort *scr);

  int encode_frame();
  WaveFile	wave;
  size_t	rv;
  
private:

  oggmux_info oggmux; // theorautils object

  int encode_video(int end_of_stream);
  int encode_audio(int end_of_stream);

  // void *enc_rgb24;

  /* video size */
  int video_x;
  int video_y;
  /* offsets for theora size constraints */
  int frame_x_offset; 
  int frame_y_offset;


  unsigned char *yuvframe[2]; /* yuv 420 */
  
  float *m_buffStream;
};

#endif

#endif
