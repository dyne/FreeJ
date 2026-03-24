
/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
 *
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
#include <iostream>
#include <config.h>

#ifdef WITH_FFMPEG

#include <samplerate.h>

#include <math.h>

#include <string.h>

#include <screen.h>
#include <context.h>
#include <jutils.h>
#ifdef WITH_AUDIO
#include <jack/jack.h>
#endif
#include <ringbuffer.h>
#include <video_layer.h>

#include <jsparser_data.h>

#include <factory.h>
//#define DEBUG 1

#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

// our objects are allowed to be created trough the factory engine
FACTORY_REGISTER_INSTANTIATOR(Layer, VideoLayer, MovieLayer, ffmpeg);

VideoLayer::VideoLayer()
  :Layer() {

  grab_dv=false;
  set_name("VID");
  frame_number=0;
  av_buf=NULL;
  avformat_context=NULL;
  packet_len=0;
  frame_rate=0;
  play_speed=1;
  play_speed_control=1;
  seekable=true;
  to_seek = -1;

  audio_resampled_buf_len=0;
  audio_float_buf = NULL;
  audio_resampled_buf = NULL;

  video_codec_ctx = NULL;
  video_index = -1;
  video_codec = NULL;

  audio_codec_ctx = NULL;
  audio_index = -1;
  audio_codec = NULL;
  audio_buf = NULL;

  use_audio = true;

  backward_control=false;
  deinterlace_buffer = NULL;
  video_clock = 0;
  rgba_picture = NULL;
  frame_fifo.length = 0;
  jsclass = &video_layer_class;

  eos = new DumbCallback();
}

VideoLayer::~VideoLayer() {
	notice("Closing video %s", get_filename());
	delete eos;
	stop();
	close();
}

/*
 * lickme.txt!
 */

bool VideoLayer::_init() {
  func("VideoLayer::init");

  rgba_picture = av_frame_alloc();

  /* init variables */
  paused=false;
  user_play_speed=1;
  deinterlace_buffer=NULL;
  deinterlaced=false;

  mark_in=NO_MARK;
  mark_out=NO_MARK;
  return true;
}

int VideoLayer::new_picture(AVFrame *picture) {
	picture->format = AV_PIX_FMT_RGB32;
	picture->width  = video_codec_ctx->width;
	picture->height = video_codec_ctx->height;
	return av_frame_get_buffer(picture, 0);
}

void VideoLayer::free_picture(AVFrame *picture) {
	if(picture != NULL) {
		av_frame_unref(picture);
		av_free(picture);
	}
	if (audio_float_buf) free (audio_float_buf);
	if (audio_resampled_buf) free (audio_resampled_buf);
}

bool VideoLayer::open(const char *file) {
  int err=0;
  video_index=-1;
  func("VideoLayer::open(%s)",file);

  const AVInputFormat *av_input_format = NULL;

  /* handle firewire cam */
  if( strncasecmp (file, "/dev/ieee1394/",14) == 0) {
    notice ("VideoLayer::found dv1394 device!\n");
    grab_dv = true;
    av_input_format = av_find_input_format("dv1394");
    file="";
  }

  /**
   * Open media with libavformat
   */
  err = avformat_open_input(&avformat_context, file, av_input_format, NULL);
  if (err < 0) {
    error("VideoLayer :: open(%s) - can't open. Error %d", file, err);
    return false;
  }
  func("VideoLayer :: file opened with success");

  /**
   * Find info with libavformat
   */
  err = avformat_find_stream_info(avformat_context, NULL);
  if (err < 0) {
    error("VideoLayer :: could not find stream info");
    return false;
  }
  func("VideoLayer :: stream info found");
  /* now we can begin to play (RTSP stream only) */
  av_read_play(avformat_context);

  /**
   * Open codec if we find a video stream
   */
  unsigned int i;
  for(i=0; i < avformat_context -> nb_streams; i++) {
    avformat_stream = avformat_context -> streams[i];
    AVCodecParameters *codecpar = avformat_stream->codecpar;
    if(codecpar == NULL) error("%s: AVCodecParameters is NULL", __PRETTY_FUNCTION__);

    switch(codecpar->codec_type) {

    /**
     * Here we look for a video stream
     */
    case AVMEDIA_TYPE_VIDEO:
      video_index = i;

      video_codec = avcodec_find_decoder(codecpar->codec_id);
      if(video_codec==NULL) {
	error("VideoLayer :: Could not find a suitable codec");
	return false;
      }

      video_codec_ctx = avcodec_alloc_context3(video_codec);
      if(!video_codec_ctx) {
	error("VideoLayer :: Could not allocate codec context");
	return false;
      }
      avcodec_parameters_to_context(video_codec_ctx, codecpar);

      if (avcodec_open2(video_codec_ctx, video_codec, NULL) < 0) {
	error("VideoLayer :: Could not open codec");
	avcodec_free_context(&video_codec_ctx);
	return false;

      } else { // correctly opened

	if(avformat_stream->r_frame_rate.den && avformat_stream->r_frame_rate.num)
	  frame_rate = av_q2d(avformat_stream->r_frame_rate);
	else
	  frame_rate = av_q2d(avformat_stream->time_base);

	func ("VideoLayer :: frame_rate den: %d", avformat_stream->time_base.den);
	func ("VideoLayer :: frame_rate num: %d", avformat_stream->time_base.num);

	// set the layer fps
	fps.set(frame_rate);
	/* this saves only file without full path! */
	set_filename (file);

	act ("%s (codec: %s) has resolution %dx%d and framerate %f",
	     get_filename(), video_codec->name,
	     video_codec_ctx->width, video_codec_ctx->height, frame_rate);

	break;
      }

      break; // //////////////// end of video section

    case AVMEDIA_TYPE_AUDIO:
      audio_index = i;

      audio_codec = avcodec_find_decoder(codecpar->codec_id);
      if(audio_codec==NULL) {
	error("VideoLayer :: Could not find a suitable codec for audio");
	break; // non-fatal, continue without audio
      }

      audio_codec_ctx = avcodec_alloc_context3(audio_codec);
      if(!audio_codec_ctx) {
	error("VideoLayer :: Could not allocate audio codec context");
	break;
      }
      avcodec_parameters_to_context(audio_codec_ctx, codecpar);

      if (avcodec_open2(audio_codec_ctx, audio_codec, NULL) < 0) {
	error("VideoLayer :: Could not open codec for audio");
	avcodec_free_context(&audio_codec_ctx);
	break;

      } else { // correctly opened
	audio_buf = (uint8_t*)calloc(AVCODEC_MAX_AUDIO_FRAME_SIZE, sizeof(int16_t));

	audio_channels = audio_codec_ctx->ch_layout.nb_channels;
	audio_samplerate = audio_codec_ctx->sample_rate;

	audio_float_buf = (float*) malloc(audio_channels * AVCODEC_MAX_AUDIO_FRAME_SIZE * sizeof(float));

	act("VideoLayer :: audio stream (codec: %s) has %u channels at samplerate %u",
	    audio_codec->name, audio_channels, audio_samplerate);

      }
      break; // /////// end of audio section


    case AVMEDIA_TYPE_SUBTITLE:
      act("stream has also subtitles");
      break;
    case AVMEDIA_TYPE_ATTACHMENT:
      act("stream has also attachment");
      break;
    case AVMEDIA_TYPE_DATA:
      act("stream has also a data carrier");
      break;
    default:
      act("stream has also an unknown codec stream");
      break;
    }

  } // done looking for streams

  if (video_index < 0) {
    error("VideoLayer :: Could not open codec");
    return false;
  }

  full_filename = strdup (file);

  geo.init(video_codec_ctx->width, video_codec_ctx->height, 32);
  func("VideoLayer :: w[%u] h[%u] size[%u]", geo.w, geo.h, geo.bytesize);
  func("VideoLayer :: frame_rate[%f]",frame_rate);

  // initialize picture
  if( new_picture(rgba_picture) < 0) {
    error("VideoLayer::error allocating picture");
    return false;
  }

#ifdef WITH_SWSCALE
  img_convert_ctx =
    sws_getContext(geo.w, geo.h, video_codec_ctx->pix_fmt, geo.w, geo.h,
		   AV_PIX_FMT_RGB32, SWS_BICUBIC,
		   NULL, NULL, NULL);
#endif

  // initialize frame fifo
  if(  new_fifo() < 0) {
    error("VideoLayer::error allocating fifo");
    return false;
  }

  // feed() function is called 25 times for second so we must correct the speed
  // TODO user should be able to select the clock speed
  if (play_speed != 25) {
    play_speed -= (25 / frame_rate);
    if ( frame_rate ==1)
      play_speed = 0;
  }
  func ("VideoLayer :: play_speed: %d",play_speed);

  opened = true;

  type = VIDEOLAYER;

  return true;
}

void *VideoLayer::feed() {
  int got_picture=0;
  int len1=0 ;
  int ret=0;
  bool got_it=false;

  double now = get_master_clock();


  if(paused)
    return rgba_picture->data[0];

  /**
   * follow user video loop
   */
  if(mark_in!=NO_MARK && mark_out!=NO_MARK && seekable) {
    if (now >= mark_out)
      seek((int64_t)mark_in * AV_TIME_BASE);
  }

  // operate seek if was requested
  if(to_seek>=0) {
    seek(to_seek);
    to_seek = -1;
  }

  got_it=false;

  while (!got_it) {


    if(packet_len<=0) {
      /**
       * Read one packet from the media and put it in pkt
       */
      while(1) {
#ifdef DEBUG
	func("av_read_frame ...");
#endif
	ret = av_read_frame(avformat_context, &pkt);

#ifdef DEBUG
	if(pkt.stream_index == video_index)
	  std::cout << "video read packet";
	else if(pkt.stream_index == audio_index)
	  std::cout << "audio read packet";
	std::cout << " pkt.data=" << pkt.data;
	std::cout << " pkt.size=" << pkt.size;
	std::cout << " pkt.pts/dts=" << pkt.pts << "/" << pkt.dts << std::endl;
	std::cout << "pkt.duration=" << pkt.duration;
	std::cout << " avformat_context->start_time=" << avformat_context->start_time;
	std::cout << " avformat_context->duration=" << avformat_context->duration/AV_TIME_BASE << std::endl;
	std::cout << "avformat_context->duration=" << avformat_context->duration << std::endl;
#endif

	/**
	 * check eof and loop
	 */
	if(ret!= 0) {	//does not enter if data are available
	  eos->notify();
	  ret = seek(avformat_context->start_time);
	  if (ret < 0) {
	    error("VideoLayer::could not loop file");
	    return rgba_picture->data[0];
	  }
	  continue;
	} else if( (pkt.stream_index == video_index)
		   || (pkt.stream_index == audio_index) )
	  break; /* exit loop */
      }
    } // loop break after a known index is found


    frame_number++;

    /**
     * Decode video
     */
    if(pkt.stream_index == video_index) {

      len1 = decode_video_packet(&got_picture);
      /* always reset so we read a new packet next iteration */
      packet_len = 0;

      AVFrame *yuv_picture=&av_frame;
      if(len1<0) {
	func("one frame only?");
	return NULL;
      }
      else if (len1 == 0) {
	return NULL;
      }

      /**
       * We've found a picture
       */
      if (got_picture!=0) {
	got_it=true;
	avformat_stream=avformat_context->streams[video_index];

	/** Deinterlace input if requested */
	if(deinterlaced)
	  deinterlace(yuv_picture);

#ifdef WITH_SWSCALE
	sws_scale(img_convert_ctx,
		  (const uint8_t * const *)yuv_picture->data,
		  yuv_picture->linesize,
		  0, video_codec_ctx->height,
		  rgba_picture->data, rgba_picture->linesize);
#else
	/**
	 * yuv2rgb (img_convert removed in modern FFmpeg - use sws_scale)
	 */
#endif
	if(fifo_position == FIFO_SIZE)
	  fifo_position=0;

	/* workaround since sws_scale conversion from YUV
	   returns an buffer RGBA with alpha set to 0x0  */
	{
	  register int bufsize = ( rgba_picture->linesize[0] * video_codec_ctx->height ) /4;
	  int32_t *pbuf =  (int32_t*)rgba_picture->data[0];

	  for(; bufsize>0; bufsize--) {
	    *pbuf = (*pbuf | alpha_bitmask);
	    pbuf++;
	  }
	}

	jmemcpy(frame_fifo.picture[fifo_position]->data[0],
		rgba_picture->data[0],
		rgba_picture->linesize[0] * video_codec_ctx->height);

	fifo_position++;
      }
    } // end video packet decoding


    ////////////////////////
    // audio packet decoding
    else if(pkt.stream_index == audio_index) {
      if(use_audio && screen) {
	int data_size;
	len1 = decode_audio_packet(&data_size);
	if (len1 > 0)  {
	  int samples = data_size/sizeof(uint16_t);
	  long unsigned int m_SampleRate = screen->m_SampleRate?*(screen->m_SampleRate):48000;
	  double m_ResampleRatio = (double)(m_SampleRate)/(double)audio_samplerate;
	  long unsigned max_buf = ceil(AVCODEC_MAX_AUDIO_FRAME_SIZE * m_ResampleRatio * audio_channels);

	  if (audio_resampled_buf_len < max_buf) {
		if (audio_resampled_buf) free (audio_resampled_buf);
		audio_resampled_buf = (float*) malloc(max_buf * sizeof(float));
		audio_resampled_buf_len = max_buf;
	  }

	  src_short_to_float_array ((const short*) audio_buf, audio_float_buf, samples);
	  if (m_ResampleRatio == 1.0)
	  {
	    ringbuffer_write(screen->audio, (const char*)audio_float_buf,  samples*sizeof(float));
	    time_t *tm = (time_t *)malloc(sizeof(time_t));
	    time (tm);
	  }
	  else
	  {
	    src_short_to_float_array ((const short*) audio_buf, audio_float_buf, samples);

	    SRC_DATA src_data;
	    int offset = 0;

            do {
	      src_data.input_frames  = samples/audio_channels;
	      src_data.output_frames = audio_resampled_buf_len/audio_channels - offset;
	      src_data.end_of_input  = 0;
	      src_data.src_ratio     =  m_ResampleRatio;
	      src_data.input_frames_used = 0;
	      src_data.output_frames_gen = 0;
	      src_data.data_in       = audio_float_buf + offset;
	      src_data.data_out      = audio_resampled_buf + offset;

	      src_simple (&src_data, SRC_SINC_MEDIUM_QUALITY, audio_channels) ;
	      ringbuffer_write(screen->audio,
			       (const char*)audio_resampled_buf,
			       src_data.output_frames_gen * audio_channels *sizeof(float));

	      offset += src_data.input_frames_used * audio_channels;
	      samples -= src_data.input_frames_used * audio_channels;

	      if (samples>0)
		warning("resampling left: %i < %i",
			src_data.input_frames_used, samples/audio_channels);

	    } while (samples > audio_channels);
	  }
	}
      }
    }

    av_packet_unref(&pkt); /* sun's good. love's bad */

  } // end of while(!got_it)

  return frame_fifo.picture[fifo_position-1]->data[0];
}


int VideoLayer::decode_audio_packet() {
  return decode_audio_packet(NULL);
}

int VideoLayer::decode_audio_packet(int *data_size) {
  int ret;

  ret = avcodec_send_packet(audio_codec_ctx, &pkt);
  if (ret < 0) {
    pkt.size = 0;
    if (data_size) *data_size = 0;
    return 0;
  }

  AVFrame *audio_frame = av_frame_alloc();
  ret = avcodec_receive_frame(audio_codec_ctx, audio_frame);
  if (ret == 0) {
    /* Copy audio data - note: assumes packed/interleaved format.
     * For planar formats (e.g. FLTP from AAC), only first channel is copied. */
    int nb_channels = audio_codec_ctx->ch_layout.nb_channels;
    int bytes_per_sample = av_get_bytes_per_sample((enum AVSampleFormat)audio_frame->format);
    int total_bytes = audio_frame->nb_samples * nb_channels * bytes_per_sample;
    if (total_bytes > AVCODEC_MAX_AUDIO_FRAME_SIZE * 2)
      total_bytes = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;
    memcpy(audio_buf, audio_frame->data[0], total_bytes);
    if (data_size) *data_size = total_bytes;
    av_frame_free(&audio_frame);
    return pkt.size;
  }

  av_frame_free(&audio_frame);
  if (data_size) *data_size = 0;
  return 0;
}

int VideoLayer::decode_video_packet(int *got_picture) {
	*got_picture = 0;
	int ret;

	ret = avcodec_send_packet(video_codec_ctx, &pkt);
	if (ret < 0) return ret;

	ret = avcodec_receive_frame(video_codec_ctx, &av_frame);
	if (ret == 0) {
		*got_picture = 1;
	}
	/* Update timing info */
	video_current_pts = (av_frame.pts != AV_NOPTS_VALUE)
	    ? av_frame.pts * av_q2d(avformat_stream->time_base)
	    : video_clock;
	video_current_pts_time = av_gettime();

	double frame_delay = av_q2d(avformat_stream->time_base);
	if (av_frame.repeat_pict) {
		frame_delay += av_frame.repeat_pict * (frame_delay * 0.5);
	}
	video_clock += frame_delay;

	return pkt.size;
}

void VideoLayer::close() {
  if(frame_number!=0) {
	func("free packet");
    av_packet_unref(&pkt);
  }

  if(video_codec_ctx) {
	func("close video codec");
    avcodec_free_context(&video_codec_ctx);
  }

  if(audio_codec_ctx) {
	func("close audio codec");
    avcodec_free_context(&audio_codec_ctx);
    if(audio_buf) {
	func("free audio buffer");
	 free(audio_buf);
	 audio_buf = NULL;
    }
  }

#ifdef WITH_SWSCALE
  if(img_convert_ctx)
    sws_freeContext(img_convert_ctx);
#endif

  if(avformat_context) {
    avformat_close_input(&avformat_context);
  }
  if(rgba_picture) free_picture(rgba_picture);
  rgba_picture = NULL;
  if(deinterlace_buffer) free(deinterlace_buffer);
}

/*
 * allocate fifo
 */
int VideoLayer::new_fifo() {
  fifo_position=0;
	int ret;
	// loop throught fifo
	for ( int s = 0; s < FIFO_SIZE; s++) {
		frame_fifo.picture[s] = av_frame_alloc();
		AVFrame *tmp_picture = frame_fifo.picture[s];
		ret = new_picture(tmp_picture);
		if ( ret < 0)
			return -1;
		frame_fifo.length++;
	}
	return 0;
}

void VideoLayer::free_fifo() {
	for ( int s = 0; s < frame_fifo.length; s++) {
		free_picture(frame_fifo.picture[s]);
		frame_fifo.picture[s] = NULL;
	}
}

bool VideoLayer::set_mark_in() {
	if (mark_in == NO_MARK) {
		mark_in = get_master_clock();
		notice("mark_in: %f", mark_in);
	}
	else {
		mark_in = NO_MARK;
		notice("mark_in deleted");
	}
	return true;
}
bool VideoLayer::set_mark_out() {
	if (mark_out == NO_MARK) {
		mark_out = get_master_clock();
		notice("mark_out: %f", mark_out);
	}
	else {
		mark_out = NO_MARK;
		notice("mark_out deleted");
	}
	return true;
}

bool VideoLayer::relative_seek(double increment) {
	int ret=0;
	double current_time=get_master_clock();
	current_time += increment;
	/**
	 * Check the seek time is correct!
	 * It should not be before or after the beginning and the end of the movie
	 */
	if (current_time < 0)  // beginning
		current_time = 0;
	/**
	 * Forward in video as a loop
	 */
	else  { // beginning
		while(current_time > (avformat_context -> duration / AV_TIME_BASE))  {
			current_time = current_time - (avformat_context->duration / AV_TIME_BASE);
		}
	}

	ret = seek ((int64_t) current_time * AV_TIME_BASE);
	if (ret < 0) {
		error ("Can't seek file: %s", get_filename());
		return false;
	}
	else
		notice("seek to %.1f%%",current_time);
	return true;
}
/**
 * Warning! doesn't lock feed
 */
int VideoLayer::seek(int64_t timestamp) {
  /* return value */
  int ret=0;
  bool seeking_at_beginning_of_stream=false;
  /** mark-{in|out} in AV_TIME_BASE unit */
  int64_t mark_in_av_time_base;
  int64_t mark_out_av_time_base;

  if(timestamp==avformat_context->start_time)
    seeking_at_beginning_of_stream=true;
  /**
   * handle bof by closing and reopening file when media it's not seekable
   */
  if (strcmp(video_codec->name,"rawvideo")==0)
    seekable=false;

  if(!seekable) {
    if(seeking_at_beginning_of_stream) {
      /** close and reopen the stream*/
      {
	close();
	open(full_filename);
      }
      return 0;
    }
    else {
      warning("%s video is not seekable!",video_codec->name);
      return -1;
    }
  }

  mark_in_av_time_base = (int64_t) mark_in * AV_TIME_BASE;
  mark_out_av_time_base = (int64_t) mark_out * AV_TIME_BASE;

  /** mark-in and mark-out seek */
  if ( mark_in != NO_MARK && mark_out != NO_MARK ) {
    if ( timestamp < mark_in_av_time_base )
      timestamp=mark_in_av_time_base++;
    else if ( timestamp > mark_out_av_time_base )
      timestamp=mark_out_av_time_base++;
  }
  func("SEEKING");
  ret = av_seek_frame(avformat_context, video_index, timestamp,
		      AVSEEK_FLAG_BACKWARD);

  if(ret<0) {
    seekable=false;
    if(seeking_at_beginning_of_stream) {
      /** close and reopen the stream*/
      {
	close();
	open(full_filename);
	return 0;
      }
    }
  }
  else { // seek with success
    // Flush buffers, should be called when seeking or when swicthing to a different stream.
    if (video_codec_ctx)
	avcodec_flush_buffers(video_codec_ctx);
    if (audio_codec_ctx)
	avcodec_flush_buffers(audio_codec_ctx);
  }
  return 0;
}

double VideoLayer::get_master_clock() {
	double delta = (av_gettime() - video_current_pts_time) / 1000000.0;
	return (video_current_pts+delta);
}

void VideoLayer::pause() {
  if(paused)
    paused=false;
  else
    paused=true;
  notice("pause : %s",(paused)?"on":"off");
}

void VideoLayer::deinterlace(AVFrame *picture) {
  /* avpicture_deinterlace was removed in FFmpeg 4.x - deinterlacing disabled */
  (void)picture;
  return;
}

#endif
