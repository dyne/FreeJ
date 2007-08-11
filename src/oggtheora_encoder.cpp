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

#include <context.h>
#include <sdl_screen.h>
#include <audio_input.h>

#include <oggtheora_encoder.h>


OggTheoraEncoder::OggTheoraEncoder() 
  : VideoEncoder() {
  func("OggTheoraEncoder object created");
  
  videoflag      = 0;
  audioflag      = 0;
  frame_finished = false;
  
  video_quality  = 16;  // it's ok for streaming
  vorbis_quality = 0.1; // it's ok for streaming
  video_bytesout = 0;
  videotime      = 0;

  picture_rgb = NULL;
  picture_yuv = NULL;

  set_name("encoder/theora");

}

OggTheoraEncoder::~OggTheoraEncoder() { // XXX TODO clear the memory !!
  func("OggTheoraEncoder:::~OggTheoraEncoder");
  
  // Set end of stream on ogg
  if (video_bytesout != 0) {
    videoflag = encode_video ( 1);
    
    flush_ogg (1);
    
    close_ogg_streams();
    
    if ( picture_yuv) { 
      //      avpicture_free(picture_yuv);
      free(picture_yuv);
    }
    if ( picture_rgb) {
      //      avpicture_free(picture_rgb);
      free(picture_rgb);
    }
  }
  
}

void OggTheoraEncoder::close_ogg_streams() {
	ogg_stream_clear ( &theora_ogg_stream);
	theora_clear ( &td);
}

bool OggTheoraEncoder::init (Context *_env) {

  if(initialized) return true;

  env = _env;
  
  screen = env->screen;
  
  init_ogg_streams();
  
  theora_init();

  
  if (! init_yuv_frame()) {
    error("initialization of yuv frame buffer for encoder %s failed", name);
    return false;
  }
  
  
  if (use_audio) {
    if(! vorbis_init () ) {
      warning("initialization of vorbis audio for encoder %s failed", name);
      use_audio = false;
    }
  }
  
  // write theora and vorbis header and flush them
  write_headers ();
  

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


// OGG INIT
bool OggTheoraEncoder::init_ogg_streams() {
  int video_serial, audio_serial;
  
  srand(time(NULL));
  
  // rando-mice
  video_serial = rand();
  if (use_audio)
    audio_serial = rand();
  
  // the serial number of audio and video streams must be different
  if (video_serial == audio_serial) audio_serial++;
  
  // Initialize logical bitstream buffers
  ogg_stream_init (&theora_ogg_stream, video_serial);
  
  if (use_audio)
    ogg_stream_init (&vorbis_ogg_stream, audio_serial);
  
  return true;
}
bool OggTheoraEncoder::theora_init() { // TODO freejrc &co
	int fps                 = env -> fps_speed;
	int video_bit_rate      = 0; // 0 autotune
	int compression_quality = video_quality; // 16 it's ok for streaming
	int sharpness           = 1;
	int w                   = screen->w;
	int h                   = screen->h;


	/* Set up Theora encoder */

	/* Theora has a divisible-by-sixteen restriction for the encoded video size */
	/* scale the frame size up to the nearest /16 and calculate offsets */
	video_x = ( (w + 15) >> 4) << 4;
	video_y = ( (h + 15) >> 4) << 4;

	/* We force the offset to be even.
	   This ensures that the chroma samples align properly with the luma
	   samples. */
	frame_x_offset = ( (video_x - w ) / 2) &~ 1;
	frame_y_offset = ( (video_y - h ) / 2) &~ 1;

	theora_info_init (&theora_information);

	theora_information.width                        = video_x;
	theora_information.height                       = video_y;
	theora_information.frame_width                  = w;
	theora_information.frame_height                 = h;
	theora_information.offset_x                     = frame_x_offset;
	theora_information.offset_y                     = frame_y_offset;
	theora_information.fps_numerator                = 1000000 * fps;
	theora_information.fps_denominator              = 1000000;
	theora_information.aspect_numerator             = 0 ;
	theora_information.aspect_denominator           = 0 ;
	theora_information.colorspace                   = OC_CS_ITU_REC_470BG;
	//	theora_information.colorspace                   = OC_CS_UNSPECIFIED;
#ifndef HAVE_64BIT
	theora_information.pixelformat                  = OC_PF_420;
#endif
	theora_information.target_bitrate               = video_bit_rate;
	theora_information.quality                      = compression_quality;

	theora_information.dropframes_p                 = 1;
	theora_information.quick_p                      = 1;
	theora_information.keyframe_auto_p              = 1;
	theora_information.keyframe_frequency           = 64;
	theora_information.keyframe_frequency_force     = 64;
	theora_information.keyframe_data_target_bitrate = (unsigned int) (video_bit_rate * 1.5);
	theora_information.keyframe_auto_threshold      = 80;
	theora_information.keyframe_mindistance         = 8;
	theora_information.noise_sensitivity            = 1;
	theora_information.sharpness                    = sharpness;

	theora_encode_init (&td, &theora_information);
	theora_info_clear (&theora_information);

	return true;
}


bool OggTheoraEncoder::vorbis_init() {
  act("initializing audio encoder");
  int ret              = 0;
  
  int audio_channels   = env->audio->channels; // mono or stereo
  int audio_hertz      = env->audio->sample_rate; // sample_rate;
  int audio_bitrate    = 32000;
  
  // init bytes count variable
  audio_bytesout       = 0;
  audiotime            = -1;
  
  vorbis_info_init (&vorbis_information);
  
  if (vorbis_quality > -99)
    ret = vorbis_encode_init_vbr (&vorbis_information, audio_channels, 
				  audio_hertz, vorbis_quality);
  else
    ret = vorbis_encode_init (&vorbis_information, audio_channels, 
			      audio_hertz, -1, audio_bitrate, -1);
  if (ret) {
    error("OggTheoraEncoder::vorbis_init() Vorbis encoder could not set up a mode according to the requested quality or bitrate.\n\n");
    return false;
  }
  
  vorbis_comment_init (&vc);
  
  vorbis_comment_add_tag (&vc, "ENCODER", PACKAGE );
  
  vorbis_analysis_init (&vd, &vorbis_information);
  
  vorbis_block_init (&vd, &vb);
  
  return true;
}

bool OggTheoraEncoder::encode_frame() {
  // frame_finished = false;
  
  if (use_audio)
    audioflag = encode_audio ( 0);
  
  videoflag = encode_video ( 0);
  
  flush_ogg (0);

  //  write_headers();
  
  // frame_finished = true;
  return true;

}
/*
void OggTheoraEncoder::print_timing(double timebase) {

	int akbps      = 0;
	int vkbps      = 0;
	int hundredths = (int) timebase * 100 - (long) timebase * 100;
	int seconds    = (long) timebase % 60;
	int minutes    = ((long) timebase / 60) % 60;
	int hours      = (long) timebase / 3600;

	vkbps = (int) rint (video_bytesout*8. / timebase * .001 );
	akbps = (int) rint (audio_bytesout*8. / timebase * .001);


	func ("\r      %d:%02d:%02d.%02d audio: %dkbps video: %dkbps                 ", // XXX broken
			hours,minutes,seconds,hundredths,akbps,vkbps);

	func ("video_bytesout: %ld",video_bytesout);
	func ("audio_bytesout: %ld",video_bytesout);

}
*/

bool OggTheoraEncoder::write_headers() {
  bool res = true;

  if(! write_theora_header() ) {
    error("error writing video headers");
    res = false;
  }

  if(use_audio) {
    if(! write_vorbis_header() ) {
      error("error writing audio headers");
      res = false;
    }
  }

  if(! flush_theora_header() ) {
    error("error flushing video headers");
    res = false;
  }

  if(use_audio) {
    if(! flush_vorbis_header() ) {
      error("error flishing audio headers");
      res = false;
    }
  }

  return res;
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
  theora_encode_YUVin (&td, &yuv);
  
  theora_encode_packetout (&td, end_of_stream, &opacket); // TODO eos variable
  
  ogg_stream_packetin (&theora_ogg_stream, &opacket);
  
  videoflag = 1;
  return videoflag;
}

int OggTheoraEncoder::encode_audio( int end_of_stream) {
  register int c;
  float **vorbis_buffer; // internal buffer for libvorbis
  
  if (end_of_stream) vorbis_analysis_wrote (&vd, 0);
  
  else {
    // take audio
    
    vorbis_buffer = vorbis_analysis_buffer (&vd, env->audio->framesperbuffer);
    
    // uninterleave samples ?
    // yes if it isn't mono

    if(env->audio->channels == 2) { // STEREO
      
      for (c=0 ; c < env->audio->framesperbuffer; c++) {
	vorbis_buffer[0][c] = (float)(env->audio->input[c*2] / 32768.f);
	vorbis_buffer[1][c] = (float)(env->audio->input[(c*2)+1] / 32768.f);
      }

    } else { // MONO
      
      for (c=0 ; c < env->audio->framesperbuffer; c++)
	vorbis_buffer[0][c] = (float)(env->audio->input[c] / 32768.f);

    }
      

    //	func ("OggTheoraEncoder:: %d audio sample to encode", number_of_sample);
    //  if (number_of_sample == 0)
    //    error ("OggTheoraEncoder:: 0 audio sample to encode!!");
    
    /* tell the library how much we actually submitted */
    vorbis_analysis_wrote (&vd, env->audio->framesperbuffer);
  }
  
  /* 
   * vorbis does some data preanalysis, then divvies up blocks
   * for more involved (potentially parallel) processing.  
   * Get a single block for encoding now.
   */
  while (vorbis_analysis_blockout (&vd, &vb) == 1 ){
    
    // analysis, assume we want to use bitrate management 
    vorbis_analysis (&vb, NULL);
    vorbis_bitrate_addblock (&vb);
    
    // weld packets into the bitstream 
    while (vorbis_bitrate_flushpacket (&vd, &opacket)) {
      ogg_stream_packetin (&vorbis_ogg_stream, &opacket);
    }
  }
  audioflag = 1; // lalalala
  return audioflag;
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

bool OggTheoraEncoder::write_theora_header() {
  /* write the bitstream header packets with proper page interleave */
  
  // first packet will get its own page automatically
  theora_encode_header (&td, &opacket);
  
  ogg_stream_packetin (&theora_ogg_stream, &opacket);
  
  // read complete pages from the stream buffer
  if (ogg_stream_pageout (&theora_ogg_stream, &opage) != 1){
    error ("OggTheoraEncoder::write_theora_header : ogg_stream_pageout returns error");
    return false;
  }

  // write header and body
  //  encpipe->write(opage.header_len, (void*) opage.header);
  //  encpipe->write(opage.body_len,   (void*) opage.body);
  //  write(fifopipe[1], opage.header, opage.header_len);
  //  write(fifopipe[1], opage.body, opage.body_len);
  jack_ringbuffer_write(ringbuffer, (const char*)opage.header, opage.header_len);
  jack_ringbuffer_write(ringbuffer, (const char*)opage.body, opage.body_len);

  // later will be written to file if(write_to_disk
  // and to stream if(stream)
  
  // create theora headers
  theora_comment_init (&tc);
  theora_comment_add_tag (&tc, "ENCODER",PACKAGE );
  theora_encode_comment (&tc, &opacket);
  ogg_stream_packetin (&theora_ogg_stream, &opacket);
  theora_encode_tables (&td, &opacket);
  ogg_stream_packetin (&theora_ogg_stream, &opacket);
  
  return true;
}

bool OggTheoraEncoder::write_vorbis_header() {

  if (use_audio) {

    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
    
    vorbis_analysis_headerout (&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin (&vorbis_ogg_stream, &header); // automatically placed in its own page
    
    if (ogg_stream_pageout (&vorbis_ogg_stream, &opage) != 1) {
      error ("OggTheoraEncoder::write_vorbis_header : ogg_stream_pageout returns error");
      return false;
    }
    
    //    encpipe->write(opage.header_len, (void*) opage.header);
    //    encpipe->write(opage.body_len,   (void*) opage.body);
    //    write(fifopipe[1], opage.header, opage.header_len);
    //    write(fifopipe[1], opage.body, opage.body_len);
    jack_ringbuffer_write(ringbuffer, (const char*) opage.header, opage.header_len);
    jack_ringbuffer_write(ringbuffer, (const char*) opage.body, opage.body_len);

    
    // remaining vorbis header packets 
    ogg_stream_packetin (&vorbis_ogg_stream, &header_comm);
    ogg_stream_packetin (&vorbis_ogg_stream, &header_code);

  }

  return true;
}

bool OggTheoraEncoder::flush_ogg (int end_of_stream) {

  int flushloop = 1; // ugly! :)
  
  while(flushloop) {
    flushloop =  0;
    
    //    if(use_audio) func ("diff: %f", audiotime - videotime);

    
    while ( end_of_stream ||
	    ( (videotime <= audiotime || !use_audio) && 
	      (videoflag == 1) ) ) {
      videoflag = 0;
      while (ogg_stream_pageout (&theora_ogg_stream, &videopage) > 0) {
	//  func ("OggTheoraEncoder:: flush_ogg() video page size is %ld",videopage-> body_len);
	if (videopage.body_len == 0)
	  warning("OggTheoraEncoder::flush_ogg : videopage body is empty");
	
	videotime = theora_granule_time (&td, ogg_page_granulepos (&videopage));
	/* flush a video page */
	
	//	encpipe->write( videopage.header_len, (void*) videopage.header);
	//	encpipe->write( videopage.body_len,   (void*) videopage.body);
	//	write(fifopipe[1], videopage.header, videopage.header_len);
	//	write(fifopipe[1], videopage.body, videopage.body_len);
	jack_ringbuffer_write(ringbuffer,(const char*) videopage.header, videopage.header_len);
	jack_ringbuffer_write(ringbuffer,(const char*) videopage.body, videopage.body_len);

	
	// print_timing (videotime);
	
	videoflag = 1;
	flushloop = 1;
      }
      if (end_of_stream) break;
    }

    
    while (use_audio &&
	   (end_of_stream || ((audiotime < videotime) && (audioflag == 1)) )) {
      
      audioflag = 0;
      while (ogg_stream_pageout (&vorbis_ogg_stream, &audiopage) > 0) {

	if (audiopage.body_len == 0)
	  warning("OggTheoraEncoder::flush_ogg : audiopage body is empty");

	/* flush an audio page */
	audiotime = vorbis_granule_time (&vd,ogg_page_granulepos (&audiopage));
	
	//	encpipe->write(audiopage.header_len, (void*) audiopage.header);
	//	encpipe->write(audiopage.body_len,   (void*) audiopage.body);
	//	write(fifopipe[1], audiopage.header, audiopage.header_len);
	//	write(fifopipe[1], audiopage.body, audiopage.body_len);
	jack_ringbuffer_write(ringbuffer, (const char*) audiopage.header, audiopage.header_len);
	jack_ringbuffer_write(ringbuffer, (const char*) audiopage.body, audiopage.body_len);

	// print_timing (audiotime);
	// akbps = rint (info->audio_bytesout * 8. / info->audiotime * .001);
	
	audioflag = 1;
	flushloop = 1;
      }
      if (end_of_stream) break;
    }
  }
  return true;
}

bool OggTheoraEncoder::flush_theora_header() {
  /* Flush the rest of our headers. This ensures
     the actual data in each stream will start
     on a new page, as per spec. */
  int result;

  while(1) {

    result = ogg_stream_flush (&theora_ogg_stream, &opage);

    if (result < 0) {
      error("OggTheoraEncoder:flush_theora_header : ogg_stream_flush returns error");
      return false;
    }
    
    if (result == 0) break;
 
    //    encpipe->write(opage.header_len, (void*) opage.header);
    //    encpipe->write(opage.body_len,   (void*) opage.body);
    //    write(fifopipe[1], opage.header, opage.header_len);
    //    write(fifopipe[1], opage.body, opage.body_len);
    jack_ringbuffer_write(ringbuffer, (const char*) opage.header, opage.header_len);
    jack_ringbuffer_write(ringbuffer, (const char*) opage.body, opage.body_len);
  
  }
  return true;
}

bool OggTheoraEncoder::flush_vorbis_header() {
  int result;

  if (use_audio) {

    while (1) {

      result = ogg_stream_flush (&vorbis_ogg_stream, &opage);
      
      if (result < 0) {
	error ("OggTheoraEncoder::flush_vorbis_header : ogg_stream_flush returns error");
	return false;
      }
      
      if (result == 0) break;
      
      //      encpipe->write(opage.header_len, (void*) opage.header);
      //      encpipe->write(opage.body_len,   (void*) opage.body);
      //      write(fifopipe[1], opage.header, opage.header_len);
      //      write(fifopipe[1], opage.body, opage.body_len);
      jack_ringbuffer_write(ringbuffer, (const char*) opage.header, opage.header_len);
      jack_ringbuffer_write(ringbuffer, (const char*) opage.body, opage.body_len);
      
    }
  }
  return true;
}

double OggTheoraEncoder::rint(double x)
{
  if (x < 0.0)
    return (double)(int)(x - 0.5);
  else
    return (double)(int)(x + 0.5);
}

bool OggTheoraEncoder::set_video_quality(int quality) {
  if (quality>63 || quality<0) {
    error("Error setting video quality! (range 0-63)");
    return false;
  }
  video_quality = quality;
  return true;
}

bool OggTheoraEncoder::set_audio_quality (double quality) {
  if (quality > 10 || quality < -1) {
    error("Error setting video quality! (range from -1 to 10) (use -1 for lowest quality, smallest file)");
    return false;
  }
  vorbis_quality = quality / 10;
  return true;
}

bool OggTheoraEncoder::has_finished_frame() {
  //  if (!frame_finished) wait_feed();
  if(frame_finished) return true;
  return false; /// QUAAA was false;
}

#endif
