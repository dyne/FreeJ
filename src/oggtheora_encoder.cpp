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
#include <screen.h>
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

  enc_y = enc_u = enc_v = enc_yuyv = NULL;


  set_name("encoder/theora");

}

OggTheoraEncoder::~OggTheoraEncoder() { // XXX TODO clear the memory !!
  func("OggTheoraEncoder:::~OggTheoraEncoder");

  oggmux_flush(&oggmux, 1);
  oggmux_close(&oggmux);

  //  if(enc_rgb24) free(enc_rgb24);

  if(audio_buf) free(audio_buf);

//   if(enc_y) free(enc_y);
//   if(enc_u) free(enc_u);
//   if(enc_v) free(enc_v);
//   if(enc_yuyv) free(enc_yuyv);

}

inline static int ilog(unsigned _v){
  int ret;
  for(ret=0;_v;ret++)_v>>=1;
  return ret;
}


bool OggTheoraEncoder::init (ViewPort *scr) {

  if(initialized) return true;

  screen = scr;

  oggmux.ringbuffer = ringbuffer;
  oggmux.bytes_encoded = 0;

  oggmux.audio_only = 0;

  if(use_audio && audio) {

    func("allocating encoder audio buffer of %u bytes",audio->buffersize);
    audio_buf = (float*)calloc(audio->buffersize, sizeof(float));

    oggmux.video_only = 0;
    oggmux.sample_rate = audio->samplerate;
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
  th_info_init (&oggmux.ti);

  oggmux.ti.pic_width                        = video_x;
  oggmux.ti.pic_height                       = video_y;
  oggmux.ti.frame_width                  = screen->w;
  oggmux.ti.frame_height                 = screen->h;
  oggmux.ti.pic_x                     = frame_x_offset;
  oggmux.ti.pic_y                     = frame_y_offset;
  oggmux.ti.fps_numerator                = 25; // env->fps.fps;
  oggmux.ti.fps_denominator              = 1;
  oggmux.ti.aspect_numerator             = 1;
  oggmux.ti.aspect_denominator           = 1;
  //  oggmux.ti.colorspace                   = OC_CS_ITU_REC_470BG;
  oggmux.ti.aspect_denominator           = TH_CS_UNSPECIFIED;
  //	oggmux.ti.colorspace                   = OC_CS_UNSPECIFIED;
  // #ifndef HAVE_64BIT
  oggmux.ti.pixel_fmt                  = TH_PF_420;
  // #endif
  oggmux.ti.target_bitrate               = video_bitrate;
  oggmux.ti.quality                      = theora_quality;

  int keyint = 64; // default keyframe interval
  oggmux.ti.keyframe_granule_shift = ilog(keyint-1);

  //  oggmux.ti.dropframes_p                 = 0; // was 0
  //  oggmux.ti.quick_p                      = 1;
  //  oggmux.ti.keyframe_auto_p              = 1;
//   oggmux.ti.keyframe_frequency           = 64;
//   oggmux.ti.keyframe_frequency_force     = 64;
//   oggmux.ti.keyframe_data_target_bitrate = (unsigned int) (video_bitrate * 1.5);
//   oggmux.ti.keyframe_auto_threshold      = 80;
//   oggmux.ti.keyframe_mindistance         = 8;
//   oggmux.ti.noise_sensitivity            = 1;
//   oggmux.ti.sharpness                    = 1;

  oggmux.td = th_encode_alloc(&oggmux.ti);
  if(!oggmux.td) {
    error("cannot allocate a theora encoder, encoding parameters might be invalid");
    return false;
  }
  // set the keyframe interval
  //  th_encode_ctl(oggmux.td, TH_ENCCTL_SET_KEYFRAME_FREQUENCY_FORCE, &keyint, sizeof(keyint-1));

  
  if( ! oggmux_init(&oggmux) ) {
    error("error initialising Ogg/Theora video encoder");
    return false;
  }
  
  enc_y     = malloc( screen->w * screen->h);
  enc_u     = malloc((screen->w * screen->h) /2);
  enc_v     = malloc((screen->w * screen->h) /2);
  enc_yuyv   = (uint8_t*)malloc(  screen->size );
  
  act("initialization succesful");
  initialized = true;
  
  return true;
}


int OggTheoraEncoder::encode_frame() {
  
  encode_video ( 0);

  if (use_audio) encode_audio ( 0);
  
  oggmux_flush(&oggmux, 0);

  bytes_encoded = oggmux.video_bytesout + oggmux.audio_bytesout;

  audio_kbps = oggmux.akbps;
  video_kbps = oggmux.vkbps;

  // just pass the reference for the status
  status = &oggmux.status[0];

  return bytes_encoded;

}



int OggTheoraEncoder::encode_video( int end_of_stream) {
  //  yuv_buffer          yuv;
  th_ycbcr_buffer ycbcr;
  
  /* take picture and convert it to yuv420 */
  //  if (!env) warning("OggTheoraEncoder::encode_video called with NULL environment");

  // picture was feeded in the right format by feed_video
  
  /* Theora is a one-frame-in,one-frame-out system; submit a frame
     for compression and pull out the packet */
  ycbcr[0].width   = video_x;
  ycbcr[0].height  = video_y;
  ycbcr[0].stride  = video_x;
  ycbcr[0].data = (unsigned char*)enc_y;

  ycbcr[1].width   = video_x /2;
  ycbcr[1].height  = video_y /2;
  ycbcr[1].stride  = video_x /2;
  ycbcr[1].data = (unsigned char*)enc_u;

  ycbcr[2].width   = video_x /2;
  ycbcr[2].height  = video_y /2;
  ycbcr[2].stride  = video_x /2;
  ycbcr[2].data = (unsigned char*)enc_v;

  
  /* encode image */
  oggmux_add_video (&oggmux, ycbcr, end_of_stream);
  return 1;
}

int OggTheoraEncoder::encode_audio( int end_of_stream) {

#ifdef WITH_SOUND
  //  num = env->audio->framesperbuffer*env->audio->channels*sizeof(int16_t);
  func("going to encode %u bytes of audio", audio->buffersize);
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
