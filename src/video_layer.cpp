
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

#include <config.h>

#ifdef WITH_FFMPEG


#include <string.h>

#include <screen.h>
#include <context.h>
#include <jutils.h>

#include <ringbuffer.h>

#include <video_layer.h>

#include <jsparser_data.h>


// #define DEBUG 1


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

  video_codec_ctx = NULL;
  video_index = -1;
  video_codec = NULL;

  audio_codec_ctx = NULL;
  audio_index = -1;
  audio_codec = NULL;
  audio_buf = NULL;

  use_audio = false;

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

bool VideoLayer::init(Context *freej) {
  func("VideoLayer::init");
  
  rgba_picture = (AVPicture *)calloc(1, sizeof(AVPicture));
  
  /* init variables */
  paused=false;
  user_play_speed=1;
  deinterlace_buffer=NULL;
  deinterlaced=false;
  
  mark_in=NO_MARK;
  mark_out=NO_MARK;
  env = freej;
  return true;
}

int VideoLayer::new_picture(AVPicture *picture) {
	memset(picture,0,sizeof(AVPicture));
	return avpicture_alloc(picture, PIX_FMT_RGB32,
			       video_codec_ctx->width,
			       video_codec_ctx->height);

}
void VideoLayer::free_picture(AVPicture *picture) {
	if(picture != NULL) {
		if (picture->data[0])
			avpicture_free(picture);
		free(picture);
	}
}

bool VideoLayer::open(const char *file) {
  AVCodecContext *enc; // tmp
  int err=0;
  video_index=-1;
  func("VideoLayer::open(%s)",file);

  if (env == NULL) {
    error("VideoLayer :: open(%s) - can't open. VideoLayer has not been initialized.", file);
    return false;
  }


  AVInputFormat *av_input_format = NULL;
  AVFormatParameters avp, *av_format_par = NULL;
  av_format_par = &avp;
  memset (av_format_par, 0, sizeof (*av_format_par));
  av_format_par->width=0;
  av_format_par->width=0;
  av_format_par->time_base  = (AVRational){1, 25};
  av_format_par->pix_fmt=PIX_FMT_RGB32;

  /* handle firewire cam */
  if( strncasecmp (file, "/dev/ieee1394/",14) == 0) {
    notice ("VideoLayer::found dv1394 device!\n");
    grab_dv = true;
    av_input_format = av_find_input_format("dv1394");

    /** shit XXX */
    av_format_par -> width             = 720;
    av_format_par -> height            = 576;
#if LIBAVCODEC_BUILD  >=     4754
    av_format_par -> time_base.num   = 25;
    av_format_par -> time_base.den   = 1;
#else
    av_format_par -> frame_rate      = 25;
    av_format_par -> frame_rate_base = 1;
#endif
    // field removed in recent ffmpeg API (todo: check LIBAVCODEC_BUILD)
    //    av_format_par -> device          = file;
    av_format_par -> standard        = "pal";
    //	av_format_par->channel=0;
    file="";
  }

  /** 
   * The callback is called in blocking functions to test regulary if
   * asynchronous interruption is needed. -EINTR is returned in this
   * case by the interrupted function. 'NULL' means no interrupt
   * callback is given.  
   */
  url_set_interrupt_cb(NULL);


  /**
   * Open media with libavformat
   */
  err = av_open_input_file (&avformat_context, file, av_input_format, 0, av_format_par);
  if (err < 0) {
    error("VideoLayer :: open(%s) - can't open. Error %d", file, err);
    return false;
  }
  func("VideoLayer :: file opened with success");

  /**
   * Find info with libavformat
   */
  err = av_find_stream_info(avformat_context);
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
    enc = avformat_stream->codec;
    if(enc == NULL) error("%s: AVCodecContext is NULL", __PRETTY_FUNCTION__);

    switch(enc->codec_type) {

    /**
     * Here we look for a video stream
     */
    case CODEC_TYPE_VIDEO:
      //      enc->flags |= CODEC_FLAG_LOOP_FILTER;
      video_index = i;
      video_codec_ctx = enc;

      video_codec = avcodec_find_decoder (video_codec_ctx -> codec_id);
      if(video_codec==NULL) {
	error("VideoLayer :: Could not find a suitable codec");
	return false;
      }
      
      if (avcodec_open(video_codec_ctx, video_codec) < 0) {
	error("VideoLayer :: Could not open codec");
	return false;
	
      } else { // correctly opened
	
#if LIBAVCODEC_BUILD  >=     4754
	frame_rate = enc -> time_base.den / 
	  enc -> time_base.num;
	AVRational rational = enc -> time_base;
	func ("VideoLayer :: frame_rate den: %d", enc -> time_base .den);
	func ("VideoLayer :: frame_rate num: %d", enc -> time_base .num);
#else
	frame_rate = video_codec_ctx->frame_rate / 
	  video_codec_ctx->frame_rate_base;
#endif
	// set the layer fps
	fps.set(frame_rate);
	/* this saves only file without full path! */
	set_filename (file);

	act ("%s (codec: %s) has resolution %dx%d and framerate %d",
	     get_filename(), video_codec->name,
	     video_codec_ctx->width, video_codec_ctx->height, frame_rate);

	break;
      }
      
      break; // //////////////// end of video section

    case CODEC_TYPE_AUDIO:
      audio_index = i;
      audio_codec_ctx = enc;

      audio_codec = avcodec_find_decoder(audio_codec_ctx -> codec_id);
      if(audio_codec==NULL) {
	error("VideoLayer :: Could not find a suitable codec for audio");
	return false;
      }
      if (avcodec_open(audio_codec_ctx, audio_codec) < 0) {
	error("VideoLayer :: Could not open codec for audio");
	return false;
	
      } else { // correctly opened

	audio_buf = (uint8_t*)calloc((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2, 1);
	
	audio_channels = audio_codec_ctx->channels;
	audio_samplerate = audio_codec_ctx->sample_rate;

	act("audio stream (codec: %s) has %u channels at samplerate %u",
	    audio_codec->name, audio_channels, audio_samplerate);

      }
      break; // /////// end of audio section


    case CODEC_TYPE_SUBTITLE:
      act("stream has also subtitles");
      break;
    case CODEC_TYPE_ATTACHMENT:
      act("stream has also attachment");
      break;
    case CODEC_TYPE_DATA:
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

  _init(video_codec_ctx->width, video_codec_ctx->height);
  func("VideoLayer :: w[%u] h[%u] size[%u]", geo.w, geo.h, geo.size);
  func("VideoLayer :: frame_rate[%d]",frame_rate);

  // initialize picture
  if( new_picture(rgba_picture) < 0) {
    error("VideoLayer::error allocating picture");
    return false;
  }

#ifdef WITH_SWSCALE
  img_convert_ctx =
    sws_getContext(geo.w, geo.h, video_codec_ctx->pix_fmt, geo.w, geo.h,
		   PIX_FMT_RGB32, SWS_BICUBIC, 
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
    //	play_speed -= play_speed << 1;
    if ( frame_rate ==1)
      play_speed = 0;
  }
  func ("VideoLayer :: play_speed: %d",play_speed);

  opened = true;

  return true;
}

void *VideoLayer::feed() {
  int got_picture=0;
  int len1=0 ;
  int ret=0;
  bool got_it=false;

  /**
   * follow user video loop
   */
  if(mark_in!=NO_MARK && mark_out!=NO_MARK && seekable) {
    if (get_master_clock()>=mark_out)
      seek((int64_t)mark_in * AV_TIME_BASE/*D ART*/);
  }
  
  if(paused)
    return rgba_picture->data[0];
  
  
  
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
	  func ("video read packet");
	else if(pkt.stream_index == audio_index)
	  func ("audio read packet");
	func ("pkt.data= %p\t",pkt.data);
	func ("pkt.size= %d\t",pkt.size);
	func ("pkt.pts/dts= %d/%d\t",pkt.pts, pkt.dts);
	func ("pkt.duration= %d\n",pkt.duration);
	func ("avformat_context->start_time= %d\n",avformat_context->start_time);
	func ("avformat_context->duration= %0.3f\n",avformat_context->duration/AV_TIME_BASE);
	func ("avformat_context->duration= %d\n",avformat_context->duration);
#endif
	
	/* TODO(shammash): this may be good for streams but breaks
	 * looping in files, needs fixing. */
	// 	      if(!pkt.duration) continue;
	
	// 	      if(!pkt.size || !pkt.data) {
	// 		return NULL;
	// 	      }
	
	
	/**
	 * check eof and loop
	 */
	if(ret!= 0) {
	  eos->notify();
	  //	  eos->dispatcher->do_jobs(); /// XXX hack hack hack
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
      
      AVFrame *yuv_picture=&av_frame;
      if(len1<0) {
	//	  error("VideoLayer::Error while decoding frame");
	func("one frame only?");
	return NULL;
      }
      else if (len1 == 0) {
	packet_len=0;
	return NULL;
      }
      
      /**
       * We've found a picture
       */
      ptr += len1;
      packet_len -= len1;
      if (got_picture!=0) {
	got_it=true;
	avformat_stream=avformat_context->streams[video_index];
	
	/** Deinterlace input if requested */
	if(deinterlaced)
	  deinterlace((AVPicture *)yuv_picture);
	
#ifdef WITH_SWSCALE
	sws_scale(img_convert_ctx, yuv_picture->data, yuv_picture->linesize,
		  0, video_codec_ctx->height,
		  rgba_picture->data, rgba_picture->linesize);	  
#else
	/**
	 * yuv2rgb
	 */
	img_convert(rgba_picture, PIX_FMT_RGB32, (AVPicture *)yuv_picture,
		    video_codec_ctx->pix_fmt, 
		    //avformat_stream.codec->pix_fmt,
		    video_codec_ctx->width,
		    video_codec_ctx->height);
#endif
	// memcpy(frame_fifo.picture[fifo_position % FIFO_SIZE]->data[0],rgba_picture->data[0],geo.size);
	/* TODO move */
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
	
	//			    avpicture_get_size(PIX_FMT_RGBA32, enc->width, enc->height));
	fifo_position++;
      }
    } // end video packet decoding
    

    ////////////////////////
    // audio packet decoding
    else if(pkt.stream_index == audio_index) {
      if(use_audio) {
	len1 = decode_audio_packet();
	ringbuffer_write(screen->audio, (const char*)audio_buf, len1);
      }
    }
    
    av_free_packet(&pkt); /* sun's good. love's bad */
    
  } // end of while(!got_it)
  
  return frame_fifo.picture[fifo_position-1]->data[0];
}


int VideoLayer::decode_audio_packet() {
  int data_size, res;
  
  res = avcodec_decode_audio2(audio_codec_ctx, (int16_t *)audio_buf,
			      &data_size, pkt.data, pkt.size);
  
  if(res < 0) {
    /* if error, skip frame */
    pkt.size = 0;
    return 0;
  }
  /* We have data, return it and come back for more later */
  return data_size;
}
   
int VideoLayer::decode_video_packet(int *got_picture) {
	double pts1 = 0;
	/**
	 * Decode the packet and put i(n)t in(t) av_frame
	 */
	if (packet_len <= 0) {
		packet_len = pkt.size; // packet size is zero if packet contains only one frame
		ptr        = pkt.data; /* pointer to frame data */
	}
	/**
	 * In avcodec_get_frame_defaults() avcodec does:
	 *    memset(pic, 0, sizeof(AVFrame));
	 *    pic->pts= AV_NOPTS_VALUE;
	 */

	avcodec_get_frame_defaults (&av_frame);
	
	int lien = avcodec_decode_video(video_codec_ctx, &av_frame,
					got_picture, ptr,packet_len);
	
	pts1 = packet_pts;
	if (packet_pts != 0) {
		/* update video clock with pts, if present */
		video_clock = packet_pts;
	} else {
		packet_pts = video_clock;
	}
	video_current_pts=packet_pts;

	video_current_pts_time=av_gettime();

	/* update video clock for next frame */
	double frame_delay ;
#if LIBAVCODEC_BUILD  >=     4754
	frame_delay = av_q2d (avformat_stream -> time_base);
#else
	frame_delay = (double)avformat_stream->codec.frame_rate_base /
		(double)avformat_stream->codec.frame_rate;
#endif

	/* for MPEG2, the frame can be repeated, so we update the
	   clock accordingly */
	if (av_frame.repeat_pict) {
		frame_delay += av_frame.repeat_pict * (frame_delay * 0.5);
	}
	video_clock += frame_delay;

	/* Debug pts code */
	{
		int ftype;
		if (av_frame.pict_type == FF_B_TYPE)
			ftype = 'B';
		else if (av_frame.pict_type == FF_I_TYPE)
			ftype = 'I';
		else
			ftype = 'P';
		//		func("frame_type=%c clock=%0.3f pts=%0.3f",
		//				ftype, get_master_clock(), pts1);
	}
	return lien;
}

void VideoLayer::close() {
  if(frame_number!=0)
    av_free_packet(&pkt);
  if(video_codec_ctx)
    if(video_codec_ctx->codec)
      avcodec_close(video_codec_ctx);
  
  if(audio_codec_ctx) {
    if(audio_codec_ctx->codec)
      avcodec_close(audio_codec_ctx);
    if(audio_buf) free(audio_buf);
  }

#ifdef HAVE_LIB_SWSCALE
  sws_freeContext(img_convert_ctx);
#endif
  
  if(avformat_context) {
    av_close_input_file(avformat_context);
  }
  free_fifo();
  if(rgba_picture) free_picture(rgba_picture);
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
		frame_fifo.picture[s] = (AVPicture *)malloc(sizeof(AVPicture));
		AVPicture *tmp_picture = frame_fifo.picture[s];
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
	}
}
// bool VideoLayer::keypress(int key) {
// 	switch(key) {
// 		case 'k':
// 			forward();
// 			break;
// 		case 'j':
// 			backward();
// 			break;
// 		case 'p': /* pause */
// 			pause();
// 			break;
// 		case 'm': /* increase playing speed */
// 			more_speed();
// 			break;
// 		case 'n': /* decrease playing speed */
// 			less_speed();
// 			break;
// 			/*
// 			   case 'b':
// 			   if(backward_control) {
// 			   backward_control=false;
// 			   show_osd("backward off");
// 			   }
// 			   else {
// 			   backward_control=true;
// 			   show_osd("backward on");
// 			   }
// 			//	    backward_one_keyframe();
// 			break;
// 			*/

// 		case 'i': /* set mark in */
// 			set_mark_in();
// 			break;

// 		case 'o': /* set mark out */
// 			set_mark_out();
// 			break;

// 		case 'u': /* Swith deinterlace */
// 			if(deinterlaced)
// 				deinterlaced=false;
// 			else
// 				deinterlaced=true;
// 			break;

// 		default:
// 			break;
// 	}
// 	return true;
// }
bool VideoLayer::set_mark_in() {
	if (mark_in == NO_MARK) {
		mark_in = get_master_clock();
		notice("mark_in: %f", mark_in);
	}
	else {
		mark_in = NO_MARK;
		notice("mark_in deleted");
	}
	show_osd();
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
	show_osd();
	return true;
}

bool VideoLayer::relative_seek(double increment) {
	int ret=0;
	lock_feed();
	double current_time=get_master_clock();
	//    printf("master_clock(): %f\n",current_time);
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

	//    printf("VideoLayer::seeking to: %f\n",current_time);
	ret = seek ((int64_t) current_time * AV_TIME_BASE);
	if (ret < 0) {
		unlock_feed ();
		error ("Can't seek file: %s", get_filename());
		return false;
	}
	else
		show_osd("seek to %.1f\%",current_time);
	unlock_feed();
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
  /**
   * HERE sick
   */
  func("SEEKING");
  ret = av_seek_frame(avformat_context, video_index,timestamp
#if (LIBAVFORMAT_BUILD >= 4618) 
		      ,AVSEEK_FLAG_BACKWARD
#endif
		      );
  //#else
  //    ret = av_seek_frame(avformat_context, video_index,timestamp);

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
    avcodec_flush_buffers(video_codec_ctx);
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
  show_osd();
}
void VideoLayer::deinterlace(AVPicture *picture) {
  int size;
  AVPicture *picture2;
  AVPicture picture_tmp;
  
  /* create temporary picture */
  size = avpicture_get_size(video_codec_ctx->pix_fmt,
			    video_codec_ctx->width,
			    video_codec_ctx->height);
  
  /* allocate only first time */
  if(deinterlace_buffer==NULL)
    deinterlace_buffer = (uint8_t *)av_malloc(size);
  if (!deinterlace_buffer)
    return ;
  
  picture2 = &picture_tmp;
  avpicture_fill(picture2, deinterlace_buffer,
		 video_codec_ctx->pix_fmt,
		 video_codec_ctx->width,
		 video_codec_ctx->height);
  
  if(avpicture_deinterlace(picture2, picture,
			   video_codec_ctx->pix_fmt,
			   video_codec_ctx->width,
			   video_codec_ctx->height) < 0) {
    /* if error, do not deinterlace */
    //	av_free(deinterlace_buffer);
    //	deinterlace_buffer = NULL;
    picture2 = picture;
  }
  if (picture != picture2)
    *picture = *picture2;
  //    av_free(deinterlace_buffer);
  return;
}

#endif
