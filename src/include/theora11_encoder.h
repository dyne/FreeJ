/*  FreeJ
 *  (c) Copyright 2009 Denis Roio <jaromil@dyne.org>
 *
 * This source code  is free software; you can  redistribute it and/or
 * modify it under the terms of the GNU Public License as published by
 * the Free Software  Foundation; either version 3 of  the License, or
 * (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but  WITHOUT ANY  WARRANTY; without  even the  implied  warranty of
 * MERCHANTABILITY or FITNESS FOR  A PARTICULAR PURPOSE.  Please refer
 * to the GNU Public License for more details.
 *
 * You should  have received  a copy of  the GNU Public  License along
 * with this source code; if  not, write to: Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __THEORA11_ENCODER_H__
#define __THEORA11_ENCODER_H__


#include <config.h>
#include <linklist.h>
#include <video_encoder.h>

#ifdef WITH_OGGTHEORA

#include <theora/theoraenc.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>

class Context;

class Theora11Encoder: public Videoencoder {
  

 public:

  Theora11Encoder();
  ~Theora11Encoder();


  bool init(Context *_env);
  
  bool feed_video();
  
  int encode_frame();

 private:

  int encode_video(int end_of_stream);
  int encode_audio(int end_of_stream);

  /* ogg_stream_state takes physical pages, weld into a logical stream
    of packets, used for skeleton stream */

  ogg_stream_state vo; ///< logical stream
  ogg_stream_state to; ///< logical stream
  ogg_stream_state so; ///< logical stream

  ogg_page         og; ///< one Ogg bitstream page.  Vorbis packets are inside
  ogg_packet       op; ///< one raw packet of data for decode

  th_enc_ctx      *td; ///< Theora encoder context
  th_info          ti; ///< Theora info (video bitstream settings)
  th_comment       tc; ///< Theora comment

  vorbis_info      vi; ///< Vorbis info (audio bitstream settings)
  vorbis_comment   vc; ///< comments

  vorbis_dsp_state vd; ///< central working state for the packet->PCM decoder
  vorbis_block     vb; ///< local working space for packet->PCM decode

  int rand_serial; ///< serial number for logical bitstreams

  int frame_w;
  int frame_h;
  int pic_w;
  int pic_h;
  int pic_x;
  int pic_y;
  int video_fps_n;
  int video_fps_d;
  int video_par_n;
  int video_par_d;
  
  /* video size */
/*   int video_x; */
/*   int video_y; */
/*   /\* offsets for theora size constraints *\/ */
/*   int frame_x_offset;  */
/*   int frame_y_offset; */
  
  unsigned char *yuvframe[2]; /* yuv 420 */

};

#endif
