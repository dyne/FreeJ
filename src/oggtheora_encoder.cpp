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
#include <audio_input.h>

#include <oggtheora_encoder.h>


OggTheoraEncoder::OggTheoraEncoder() 
  : VideoEncoder() {
  func("OggTheoraEncoder object created");
  
  video_quality  = 16;  // it's ok for streaming
  audio_quality =  10; // it's ok for streaming

  picture_rgb = NULL;
  picture_yuv = NULL;

  use_audio = true;

  init_info(&oggmux);
  theora_comment_init(&oggmux.tc);


  set_name("encoder/theora");

}

OggTheoraEncoder::~OggTheoraEncoder() { // XXX TODO clear the memory !!
  func("OggTheoraEncoder:::~OggTheoraEncoder");
  
  oggmux_flush(&oggmux, 1);
  oggmux_close(&oggmux);
  
  if ( picture_yuv) { 
    //      avpicture_free(picture_yuv);
    free(picture_yuv);
  }
  if ( picture_rgb) {
    //      avpicture_free(picture_rgb);
    free(picture_rgb);
  }
}


bool OggTheoraEncoder::init (Context *_env) {

  if(initialized) return true;

  env = _env;
  
  screen = env->screen;

  oggmux.ringbuffer = ringbuffer;
  oggmux.bytes_encoded = 0;

  oggmux.audio_only = 0;
  if(use_audio) oggmux.video_only = 0;
  else oggmux.video_only = 1;

  oggmux.sample_rate = env->audio->sample_rate;
  oggmux.channels = env->audio->channels;
  oggmux.vorbis_quality = audio_quality / 100;
  oggmux.vorbis_bitrate = audio_bitrate;

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
  oggmux.ti.fps_numerator                = env->fps_speed * 1000000;
  oggmux.ti.fps_denominator              = 1000000;
  oggmux.ti.aspect_numerator             = 0;
  oggmux.ti.aspect_denominator           = 0;
  oggmux.ti.colorspace                   = OC_CS_ITU_REC_470BG;
  //	oggmux.ti.colorspace                   = OC_CS_UNSPECIFIED;
#ifndef HAVE_64BIT
  oggmux.ti.pixelformat                  = OC_PF_420;
#endif
  oggmux.ti.target_bitrate               = video_bitrate;
  oggmux.ti.quality                      = theora_quality;
  
  oggmux.ti.dropframes_p                 = 0;
  oggmux.ti.quick_p                      = 1;
  oggmux.ti.keyframe_auto_p              = 1;
  oggmux.ti.keyframe_frequency           = 64;
  oggmux.ti.keyframe_frequency_force     = 64;
  oggmux.ti.keyframe_data_target_bitrate = (unsigned int) (video_bitrate * 1.5);
  oggmux.ti.keyframe_auto_threshold      = 80;
  oggmux.ti.keyframe_mindistance         = 8;
  oggmux.ti.noise_sensitivity            = 1;
  oggmux.ti.sharpness                    = 1;

  oggmux_init(&oggmux);


  
  if (! init_yuv_frame()) {
    error("initialization of yuv frame buffer for encoder %s failed", name);
    return false;
  }
  
  

  func("init picture_rgb for colorspace conversion (avcodec)");
  picture_rgb = avcodec_alloc_frame ();
  picture_rgb -> data[0]     = NULL;
  picture_rgb -> data[1]     = NULL;
  picture_rgb -> data[2]     = NULL;
  picture_rgb -> linesize[0] = video_x * 4;

  func("init picture_yuv for colorspace conversion (avcodec)");  
  picture_yuv = avcodec_alloc_frame ();
  
  int size = avpicture_get_size (PIX_FMT_YUV420P, video_x, video_y);
  uint8_t *video_outbuf = (uint8_t *) av_malloc (size);
  avpicture_fill ((AVPicture *)picture_yuv, video_outbuf, PIX_FMT_YUV420P, video_x, video_y);
  
  act("initialization succesful");
  initialized = true;
  
  return true;
}


int OggTheoraEncoder::encode_frame() {
  
  encode_video ( 0);

  if (use_audio) encode_audio ( 0);
  
  oggmux_flush(&oggmux, 0);

  audio_kbps = oggmux.akbps;
  video_kbps = oggmux.vkbps;
  bytes_encoded = oggmux.bytes_encoded;
  return bytes_encoded;

}


bool OggTheoraEncoder::feed_video() {  
  /* Convert picture from rgb to yuv420 */
  picture_rgb -> data[0]     = (uint8_t *) screen-> get_surface ();
  img_convert ((AVPicture *)picture_yuv, PIX_FMT_YUV420P,
	       (AVPicture *)picture_rgb, PIX_FMT_RGBA32,
	       video_x, video_y);

  return true;
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
  yuv.y_stride  = picture_yuv->linesize [0];
  
  yuv.uv_width  = video_x / 2;
  yuv.uv_height = video_y / 2;
  yuv.uv_stride = picture_yuv->linesize [1];
  
  yuv.y = (uint8_t *) picture_yuv->data [0];
  yuv.u = (uint8_t *) picture_yuv->data [1]; 
  yuv.v = (uint8_t *) picture_yuv->data [2];
  
  /* encode image */
  oggmux_add_video (&oggmux, &yuv, end_of_stream);
  
  return 1;
}

int OggTheoraEncoder::encode_audio( int end_of_stream) {
  int num;
  int read;

  num = env->audio->framesperbuffer*env->audio->channels*sizeof(int16_t);
  
  read = ringbuffer_read(env->audio->input_pipe, (char*)env->audio->input, num); 
  ///// QUAAAA
  //  oggmux_add_audio (oggmux_info *info, int16_t * readbuffer, int bytesread, int samplesread,int e_o_s);
  oggmux_add_audio(&oggmux, env->audio->input,
		   read,
		   read / env->audio->channels /2,
		   end_of_stream );
  
  return 1;
}

bool OggTheoraEncoder::init_yuv_frame() {
  func ("OggTheoraEncoder init yuv frame");
  
  yuvframe[0]    = NULL;
  yuvframe[1]    = NULL;
  
  /* initialize the double frame buffer */
  yuvframe[0] = (uint8_t *)jalloc ( yuvframe[0], video_x * video_y * 3/2);
  yuvframe[1] = (uint8_t *)jalloc ( yuvframe[1], video_x * video_y * 3/2);
  
  if (yuvframe[0] == NULL || yuvframe[1] ==NULL ) {
    error("OggTheoraEncoder::init_yuv_frame() can't get memory :(");
    return false;	
  }
  
  /* clear initial frame as it may be larger than actual video data */
  /* fill Y plane with 0x10 and UV planes with 0X80, for black data */
  memset (yuvframe[0], 0x10, video_x * video_y);
  memset (yuvframe[0] + video_x * video_y, 0x80, video_x * video_y / 2);
  memset (yuvframe[1], 0x10,video_x*video_y);
  memset (yuvframe[1] + video_x * video_y, 0x80, video_x * video_y / 2);
  return true;
}


#endif
