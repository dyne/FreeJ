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


#include <config.h>
#include <linklist.h>
#include <video_encoder.h>
#include <screen.h>

#ifdef WITH_OGGTHEORA
#include "theora/theora.h"
#include "vorbis/codec.h" // TODO vorbis encoding from mic
#include "vorbis/vorbisenc.h"

#include <avcodec.h>
#include <avformat.h>

//#define AUDIO_BUFFER_SIZE 3528 // sample_rate * channel / fps / bytes of sample format

class Context;

class OggTheoraEncoder: public VideoEncoder{

 public:
  
  OggTheoraEncoder(char *output_filename);
  ~OggTheoraEncoder();
  
  bool init(Context *_env, ViewPort *_screen);
  void set_encoding_parameter();
  bool set_video_quality(int quality);
  bool set_audio_quality(double quality);
  bool write_frame();
  bool isStarted();
  
  
 private:
  
  void convert_to_YUV420P();
  void run(); ///< Main loop
  AVFrame *picture_rgb;
  AVFrame *picture_yuv;

  int audio_buffer_size;

  bool init_ogg_streams();
  bool theora_init();
  bool vorbis_init();
  bool write_headers();
  bool write_theora_header();
  bool write_vorbis_header();

  bool flush_theora_header();
  bool flush_vorbis_header();

  bool flush_ogg (int end_of_stream);
  void close_ogg_streams();

  bool  init_yuv_frame();
  void  print_timing (double timebase);
  int encode_video( int end_of_stream);

  // audio 
  int encode_audio( int end_of_stream);

  double rint(double x);

  bool has_finished_frame();

//  bool use_audio;
  bool started;
  bool frame_finished;
  int video_quality;
  double vorbis_quality;

  /* video size */
  int video_x;
  int video_y;
  /* offsets for theora size constraints */
  int frame_x_offset; 
  int frame_y_offset;

  int audioflag;
  int videoflag;

  ogg_int64_t audio_bytesout;
  ogg_int64_t video_bytesout;

  ogg_page videopage;
  ogg_page audiopage;

  double videotime;
  double audiotime;

  FILE *video_fp;

  unsigned char        *yuvframe[2]; /* yuv 420 */
  signed char        *line;

  // 2 separate logical bitstreams for theora and vorbis
  ogg_stream_state theora_ogg_stream; 
  ogg_stream_state vorbis_ogg_stream; 
  
  ogg_page         opage; /* one Ogg bitstream page.  Vorbis packets are inside */
  ogg_packet       opacket; /* one raw packet of data for decode */

  theora_state     td;
  theora_info      theora_information;
  theora_comment   tc;

  vorbis_info      vorbis_information; /* struct that stores all the static vorbis bitstream
                          settings */
  vorbis_comment   vc; /* struct that stores all the user comments */

  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block     vb; /* local working space for packet->PCM decode */

  ViewPort         *screen;
  int16_t *audiobuffer;

};

#endif

#endif
