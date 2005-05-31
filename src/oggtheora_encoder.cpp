/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani aka kysucix <kysucix@dyne.org>
 *  learned from encoder_example and ffmpeg2theora :)
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
#include <oggtheora_encoder.h>
#include <string.h>
#include <context.h>
#include <sdl_screen.h>
#include <stdlib.h>

#include <portaudio.h>

OggTheoraEncoder::OggTheoraEncoder(char *output_filename) 
	:VideoEncoder(output_filename) {
		func("OggTheoraEncoder::OggTheoraEncoder::OggTheoraEncoder object created");

		use_audio      = false;

		videoflag      = 0;
		audioflag      = 0;
		frame_finished = false;
		started	       = false;
		video_quality  = 16; // it's ok for streaming

	}
OggTheoraEncoder::~OggTheoraEncoder() { // XXX TODO clear the memory !!
	func("OggTheoraEncoder:::~OggTheoraEncoder");

	// Set end of stream on ogg
	if (video_bytesout != 0) {
		videoflag = encode_video ( 1);

		flush_ogg (1);

		close_ogg_streams();

		if ( !picture_yuv) { 
			//		avpicture_free(picture_yuv);
			//		free(picture);
		}
		if ( !picture_rgb) {
			//		avpicture_free(picture);
			//		free(picture);
		}
	}
}
void OggTheoraEncoder::close_ogg_streams() {
	ogg_stream_clear ( &theora_ogg_stream);
	theora_clear ( &td);
}

	bool OggTheoraEncoder::init (Context *_env, ViewPort *_screen) {
		if(started)
			return true;

		/* open the file */
		video_fp = fopen (get_filename(), "w");

		if (! _init(_env))
			return false;

		screen = _screen;

		init_ogg_streams();

		theora_init();

		if (! init_yuv_frame())
			return false;

		if (use_audio) {
			if(vorbis_init ()) {
				audiobuffer = NULL;
				audiobuffer = (int16_t *) jalloc (audiobuffer, AUDIO_BUFFER_SIZE);
			}
			else
				use_audio = false;
		}

		// write theora and vorbis header and flush them
		write_headers ();

		// init picture_rgb
		{
			picture_rgb = avcodec_alloc_frame ();
			picture_rgb -> data[0]     = NULL;
			picture_rgb -> data[1]     = NULL;
			picture_rgb -> data[2]     = NULL;
			picture_rgb -> linesize[0] = video_x * 4;
		}

		picture_yuv = avcodec_alloc_frame ();

		int size = avpicture_get_size (PIX_FMT_YUV420P, video_x, video_y);
		uint8_t *video_outbuf = (uint8_t *) av_malloc (size);
		avpicture_fill ((AVPicture *)picture_yuv, video_outbuf, PIX_FMT_YUV420P, video_x, video_y);

		started = true;

		return true;
	}
void OggTheoraEncoder::set_encoding_parameter() { // boom. kysucix.
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
	if (video_serial == audio_serial) 
		audio_serial++;

	// Initialize logical bitstream buffers
	ogg_stream_init (&theora_ogg_stream, video_serial);

	if (use_audio)
		ogg_stream_init (&vorbis_ogg_stream, audio_serial);

	return true;
}
bool OggTheoraEncoder::theora_init() { // TODO freejrc &co
	int fps                 = 25;
	int video_bit_rate      = 0; // 0 autotune
	int compression_quality = video_quality; // 16 it's ok for streaming
	int sharpness           = 1;
	int w                   = screen->w;
	int h                   = screen->h;

	video_bytesout          = 0;
	videotime               = 0;

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
	theora_information.pixelformat                  = OC_PF_420;
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
	int ret              = 0;

	float audio_quality  = 0.1;
	int audio_channels   = 1; // TODO parameters
	int audio_hertz      = 44100;
	int audio_bitrate    = 64000;

	// init bytes count variable
	audio_bytesout       = 0;
	audiotime            = -1;

	vorbis_info_init (&vorbis_information);

	if (audio_quality > -99)
		ret = vorbis_encode_init_vbr (&vorbis_information, audio_channels, 
				audio_hertz, audio_quality);
	else
		ret = vorbis_encode_init (&vorbis_information, audio_channels, 
				audio_hertz, -1, audio_bitrate, -1);
	if (ret) {
		error("OggTheoraEncoder::vorbis_init() Vorbis encoder could not set up a mode according to\n"
				"the requested quality or bitrate.\n\n");
		return false;
	}

	vorbis_comment_init (&vc);

	vorbis_comment_add_tag (&vc, "ENCODER", PACKAGE );

	vorbis_analysis_init (&vd, &vorbis_information);

	vorbis_block_init (&vd, &vb);

	return true;
}
void OggTheoraEncoder::run() {
	started=true;
	while(true) {
		//		wait();

		write_frame();
		//		signal_feed();
	}
}
bool OggTheoraEncoder::write_frame() {
	//	frame_finished = false;

	if (use_audio)
		audioflag = encode_audio ( 0);

	videoflag = encode_video ( 0);

	flush_ogg (0);

	//	frame_finished = true;
	return true;
}
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
bool OggTheoraEncoder::write_headers() {
	if (! (write_theora_header() && 
				write_vorbis_header()) 
	   ) {
		error ("OggTheoraEncoder::write_headers() can't write headers");
		return false;
	}
	if(! (flush_theora_header() && 
				flush_vorbis_header()) 
	  ) {
		error ("OggTheoraEncoder::write_headers() can't flush headers");
		return false;
	}

	return true;
}
int OggTheoraEncoder::encode_video( int end_of_stream) {
	yuv_buffer          yuv;

	/* take picture and convert it to yuv420 */
	if(env==NULL)
		notice("env null");

	screen->lock();

	picture_rgb -> data[0]     = (uint8_t *) screen-> get_surface ();
	img_convert ((AVPicture *)picture_yuv, PIX_FMT_YUV420P, (AVPicture *)picture_rgb, 
			PIX_FMT_RGBA32, video_x, video_y);

	screen->unlock();

	/* Theora is a one-frame-in,one-frame-out system; submit a frame
	   for compression and pull out the packet */
	yuv.y_width   = video_x;
	yuv.y_height  = video_y;
	yuv.y_stride  = picture_yuv->linesize[0];

	yuv.uv_width  = video_x / 2;
	yuv.uv_height = video_y / 2;
	yuv.uv_stride = picture_yuv->linesize[1];

	yuv.y = (uint8_t *) picture_yuv->data[0];
	yuv.u = (uint8_t *) picture_yuv->data[1]; 
	yuv.v = (uint8_t *) picture_yuv->data[2];

	/* encode image */
	theora_encode_YUVin (&td, &yuv);

	theora_encode_packetout (&td, end_of_stream, &opacket); // TODO eos variable

	ogg_stream_packetin (&theora_ogg_stream, &opacket);

	videoflag = 1;
	return videoflag;
}
int OggTheoraEncoder::encode_audio( int end_of_stream) {
	int number_of_sample = 0;
	int count            = 0;
	int audio_channels   = 1; // maybe TODO

	float **vorbis_buffer; // internal buffer for libvorbis

	if (end_of_stream)
		vorbis_analysis_wrote (&vd, 0);
	else {
		// take audio from fifo queue
		int in = coda -> read (AUDIO_BUFFER_SIZE , audiobuffer);
		if (in == -1)
			func(" COSAAAAAAAAAAAAA ????????????");

		number_of_sample  = AUDIO_BUFFER_SIZE / audio_channels / 2;

		vorbis_buffer = vorbis_analysis_buffer (&vd, number_of_sample);
		/* uninterleave samples */
		for (int i=0; i < number_of_sample; i++){
			//			for(int j=0; j < audio_channels; j++){
			//				vorbis_buffer [j][i] = ( (audiobuffer [count+1] << 8) |
			//						(0x00ff & (int) audiobuffer [count]) )  / 32768.f;

			vorbis_buffer[0][i] = audiobuffer[count++] / 32768.f;
			//			}
		}
		/* tell the library how much we actually submitted */
		vorbis_analysis_wrote (&vd, number_of_sample);
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
	func ("OggTheoraEncoder::initing yuv frame");

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
		error ("Internal Ogg library error.");
		return false;
	}

	// write header and body to file

	fwrite (opage.header, 1, opage.header_len, video_fp); /* TODO USE libshout */
	fwrite (opage.body ,1, opage.body_len, video_fp);

	// STOP BUFFER NOW! ... GOOOOOOOOO :)
	if (stream) {
		env -> shouter -> send (opage.header, opage.header_len);
		env -> shouter -> send (opage.body, opage.body_len);
	}

	// create theora headers
	theora_comment_init (&tc);
	theora_comment_add_tag (&tc, "ENCODER",PACKAGE ); /* XXX Fuck it doesn't work now */
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
			error ("Internal Ogg library error.");
			return false;
		}

		fwrite (opage.header, 1, opage.header_len, video_fp);
		fwrite (opage.body, 1, opage.body_len, video_fp);

		// STOP BUFFER NOW! ... GOOOOOOOOO :)
		if (stream) {
			env -> shouter -> send (opage.header, opage.header_len);
			env -> shouter -> send (opage.body, opage.body_len);
		}

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

		if(use_audio)
			func ("diff: %f", audiotime - videotime);

		while ( end_of_stream ||
				( (videotime <= audiotime || !use_audio) && 
				  (videoflag == 1) ) ) {
			videoflag = 0;
			while (ogg_stream_pageout (&theora_ogg_stream, &videopage) > 0) {
				videotime = theora_granule_time (&td, ogg_page_granulepos (&videopage));
				/* flush a video page */

				video_bytesout += fwrite (videopage.header, 1, videopage.header_len, video_fp);
				video_bytesout += fwrite (videopage.body, 1, videopage.body_len, video_fp);

				// STOP BUFFER NOW! ... GOOOOOOOOO :)
				if (stream) {
					env -> shouter -> send (videopage.header, videopage.header_len);
					env -> shouter -> send (videopage.body, videopage.body_len);
				}

				//				print_timing (videotime);

				videoflag = 1;
				flushloop = 1;
			}
			if (end_of_stream)
				break;
		}


		while (use_audio && 
				(end_of_stream || 
				 ((audiotime < videotime)  && (audioflag == 1)) )) {

			audioflag = 0;
			while (ogg_stream_pageout (&vorbis_ogg_stream, &audiopage) > 0) {
				/* flush an audio page */
				audiotime = vorbis_granule_time (&vd,ogg_page_granulepos (&audiopage));

				audio_bytesout += fwrite (audiopage.header, 1,audiopage.header_len, video_fp);
				audio_bytesout += fwrite (audiopage.body, 1,audiopage.body_len, video_fp);

				// STOP BUFFER NOW! ... GOOOOOOOOO :)
				if (stream) {
					env -> shouter -> send (audiopage.header, audiopage.header_len);
					env -> shouter -> send (audiopage.body, audiopage.body_len);
				}

				//				print_timing (audiotime);
				//				akbps = rint (info->audio_bytesout * 8. / info->audiotime * .001);

				audioflag = 1;
				flushloop = 1;
			}
			if (end_of_stream)
				break;
		}
	}
	return true;
}
bool OggTheoraEncoder::flush_theora_header() {
	/* Flush the rest of our headers. This ensures
	   the actual data in each stream will start
	   on a new page, as per spec. */
	while(1) {
		int result = ogg_stream_flush (&theora_ogg_stream, &opage);
		if (result < 0) {
			error("OggTheoraEncoder:flush_theora_header() Internal Ogg library error.");
			return false;
		}
		if (result == 0)
			break;

		fwrite (opage.header, 1, opage.header_len, video_fp);
		fwrite (opage.body, 1, opage.body_len, video_fp);

		// STOP BUFFER NOW! ... GOOOOOOOOO :)
		if (stream) {
			env -> shouter -> send (opage.header, opage.header_len);
			env -> shouter -> send (opage.body, opage.body_len);
		}

	}
	return true;
}
bool OggTheoraEncoder::flush_vorbis_header() {
	if (use_audio) {
		while (1) {
			int result = ogg_stream_flush (&vorbis_ogg_stream, &opage);

			if (result < 0){
				/* can't get here */
				error ("OggTheoraEncoder:flush_vorbis_header()Internal Ogg library error.");
				return false;
			}
			if (result == 0)
				break;

			fwrite (opage.header, 1, opage.header_len, video_fp);
			fwrite (opage.body, 1, opage.body_len, video_fp);

			// STOP BUFFER NOW! ... GOOOOOOOOO :)
			if (stream) {
				env -> shouter -> send (opage.header, opage.header_len);
				env -> shouter -> send (opage.body, opage.body_len);
			}
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
bool OggTheoraEncoder::isStarted() {
	return started;
}
bool OggTheoraEncoder::set_video_quality(int quality) {
	if (quality>63 || quality<0) {
		error("Error setting video quality! (range 0-63)");
		return false;
	}
	video_quality = quality;
	return true;
}
	bool OggTheoraEncoder::has_finished_frame() {
		if (!frame_finished)
			//		wait_feed();
			return true;
	}
#endif
