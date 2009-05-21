/*  FreeJ - Ogg/Vorbis/Theora encoder
 *
 *  (c) Copyright 2005 Silvano Galliani <kysucix@dyne.org>
 *                2007 Denis Rojo       <jaromil@dyne.org>
 *
 * ogg/vorbis/theora code learned from encoder_example and ffmpeg2theora
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

#include <config.h>

#ifdef WITH_OGGTHEORA

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <context.h>
#include <sdl_screen.h>
#include <audio_collector.h>

#include <oggtheora_encoder.h>


OggTheoraEncoder::OggTheoraEncoder() 
  : VideoEncoder() {
  func("OggTheoraEncoder object created");
  
  video_quality  = 16;  // it's ok for streaming
  audio_quality =  10; // it's ok for streaming

  //  picture_rgb = NULL;
  //  enc_rgb24 = NULL;

  use_audio = false;
  audio = NULL;
  audio_buf = NULL;

  init_info(&oggmux);
  theora_comment_init(&oggmux.tc);


  set_name("encoder/theora");

}

OggTheoraEncoder::~OggTheoraEncoder() { // XXX TODO clear the memory !!
  func("OggTheoraEncoder:::~OggTheoraEncoder");
  
  oggmux_flush(&oggmux, 1);
  oggmux_close(&oggmux);
  
  //  if(enc_rgb24) free(enc_rgb24);

  if(audio_buf) free(audio_buf);
}


bool OggTheoraEncoder::init (Context *_env) {

  if(initialized) return true;

  env = _env;
  
  screen = env->screen;

  oggmux.ringbuffer = ringbuffer;
  oggmux.bytes_encoded = 0;

  oggmux.audio_only = 0;

  if(use_audio && audio) {

    func("allocating encoder audio buffer of %u bytes",audio->BufferLength);
    audio_buf = (float*)calloc(audio->BufferLength, sizeof(float));

    oggmux.video_only = 0;
    oggmux.sample_rate = audio->Samplerate;
    oggmux.channels = 1; // only 1 channel jack support for now
    oggmux.vorbis_quality = audio_quality / 100;
    oggmux.vorbis_bitrate = audio_bitrate;
    
  } else {

    oggmux.video_only = 1;
    use_audio = false;

  }


  /* Set up Theora encoder */

  int theora_quality = (int) ( (video_quality * 63) / 100);
  int w                   = screen->w;
  int h                   = screen->h;
  func("VideoEncoder: encoding theora to quality %u", theora_quality);
  /* Theora has a divisible-by-sixteen restriction for the encoded video size */
  /* scale the frame size up to the nearest /16 and calculate offsets */
  video_x = ( (w + 15) >> 4) << 4;
  video_y = ( (h + 15) >> 4) << 4;
  /* We force the offset to be even.
     This ensures that the chroma samples align properly with the luma
     samples. */
  frame_x_offset = ( (video_x - w ) / 2) &~ 1;
  frame_y_offset = ( (video_y - h ) / 2) &~ 1;


  /* video settings here */
  theora_info_init (&oggmux.ti);

  oggmux.ti.width                        = video_x;
  oggmux.ti.height                       = video_y;
  oggmux.ti.frame_width                  = screen->w;
  oggmux.ti.frame_height                 = screen->h;
  oggmux.ti.offset_x                     = frame_x_offset;
  oggmux.ti.offset_y                     = frame_y_offset;
  oggmux.ti.fps_numerator                = env->fps_speed;
  oggmux.ti.fps_denominator              = 1;
  oggmux.ti.aspect_numerator             = 0;
  oggmux.ti.aspect_denominator           = 0;
  oggmux.ti.colorspace                   = OC_CS_ITU_REC_470BG;
  //	oggmux.ti.colorspace                   = OC_CS_UNSPECIFIED;
  // #ifndef HAVE_64BIT
  oggmux.ti.pixelformat                  = OC_PF_420; // was OC_PF_420 with ccvt
  // #endif
  oggmux.ti.target_bitrate               = video_bitrate;
  oggmux.ti.quality                      = theora_quality;
  
  oggmux.ti.dropframes_p                 = 1;
  oggmux.ti.quick_p                      = 1;
  oggmux.ti.keyframe_auto_p              = 1;
  oggmux.ti.keyframe_frequency           = 25;
  oggmux.ti.keyframe_frequency_force     = 25;
  oggmux.ti.keyframe_data_target_bitrate = (unsigned int) (video_bitrate);
  oggmux.ti.keyframe_auto_threshold      = 80;
  oggmux.ti.keyframe_mindistance         = 8;
  oggmux.ti.noise_sensitivity            = 1;
  oggmux.ti.sharpness                    = 1;

  oggmux_init(&oggmux);
  

  
  act("initialization succesful");
  initialized = true;
  
  return true;
}


int OggTheoraEncoder::encode_frame() {
  
  encode_video ( 0);

  if (use_audio) encode_audio ( 0);
  
  oggmux_flush(&oggmux, 0);

  bytes_encoded = oggmux.video_bytesout+oggmux.audio_bytesout;

  audio_kbps = oggmux.akbps;
  video_kbps = oggmux.vkbps;

  // just pass the reference for the status
  status = &oggmux.status[0];

  return bytes_encoded;

}



int OggTheoraEncoder::encode_video( int end_of_stream) {
  yuv_buffer          yuv;
  
  /* take picture and convert it to yuv420 */
  if (!env) warning("OggTheoraEncoder::encode_video called with NULL environment");

  // picture was feeded in the right format by feed_video
  
  /* Theora is a one-frame-in,one-frame-out system; submit a frame
     for compression and pull out the packet */
  yuv.y_width   = video_x;
  yuv.y_height  = video_y;
  //  yuv.y_stride  = picture_yuv->linesize [0];
  yuv.y_stride = video_x;
  
  yuv.uv_width  = video_x >> 1;
  yuv.uv_height = video_y >> 1;
  //  yuv.uv_stride = picture_yuv->linesize [1];
  yuv.uv_stride = video_x >> 1;

   yuv.y = (uint8_t *) enc_y;
   yuv.u = (uint8_t *) enc_u;
   yuv.v = (uint8_t *) enc_v;
  
  /* encode image */
  oggmux_add_video (&oggmux, &yuv, end_of_stream);
  return 1;
}

int OggTheoraEncoder::encode_audio( int end_of_stream) {

#ifdef WITH_SOUND
  //  num = env->audio->framesperbuffer*env->audio->channels*sizeof(int16_t);
  func("going to encode %u bytes of audio", audio->BufferLength);
  ///// QUAAAA
  //  oggmux_add_audio (oggmux_info *info, int16_t * readbuffer, int bytesread, int samplesread,int e_o_s);
  //  oggmux_add_audio(&oggmux, env->audio->input,
  //		   read,
  //		   read / env->audio->channels /2,
  //		   end_of_stream );
  //  audio->get_audio(audio_buf);

  // WAS:
//   oggmux_add_audio_float(&oggmux, audio_buf,
// 			 audio->BufferLength, end_of_stream);


#endif

  return 1;
}




#endif
