/*  FreeJ movie layer
 *
 *  (c) Copyright 2001-2002 Silvano Galliani <kysucix@dyne.org>
 *                2007-2008 Denis Rojo  <jaromil@dyne.org>
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

#include <context.h>
#include <jutils.h>

#include <video_layer.h>

#include <jsparser_data.h>


#define DEBUG 1


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
		enc=NULL;
		backward_control=false;
		deinterlace_buffer = NULL;
		video_clock = 0;
		rgba_picture = NULL;
		frame_fifo.length = 0;
		jsclass = &video_layer_class;
	}

VideoLayer::~VideoLayer() {
	notice("Closing video %s", get_filename());
	stop();
	close();
}

/*
 * lickme.txt!
 */

bool VideoLayer::init(Context *freej) {
  func("VideoLayer::init");

  // initialize picture  
  rgba_picture = (AVPicture*) calloc(1,sizeof(AVPicture));
  if( avpicture_alloc(rgba_picture,PIX_FMT_RGBA32,enc->width, enc->height) <0) {
    error("VideoLayer::init cannot allocate picture buffer");
    return false;
  }
  
  /* init variables */
  paused=false;
  user_play_speed=1;
  
  mark_in=NO_MARK;
  
  /* init variables */
  paused=false;
  user_play_speed=1;
  
  mark_in=NO_MARK;
  mark_out=NO_MARK;
  env = freej;
  return true;
}

int VideoLayer::new_picture(AVPicture *picture) {
	memset(picture,0,sizeof(AVPicture));
	return avpicture_alloc(picture,PIX_FMT_RGBA32,enc->width, enc->height);

}
void VideoLayer::free_picture(AVPicture *picture) {
	if(picture != NULL) {
		if (picture->data[0])
			avpicture_free(picture);
		free(picture);
	}
}

bool VideoLayer::open(char *file) {
  int err=0;
  video_index=-1;
  func("VideoLayer::open(%s)",file);

  AVInputFormat *av_input_format=NULL;
  AVFormatParameters avp, *av_format_par = NULL;
  av_format_par = &avp;
  memset (av_format_par, 0, sizeof (*av_format_par));

  /** init ffmpeg libraries */
  /* register all codecs, demux and protocols */
  av_register_all();

  /** make ffmpeg silent */
  av_log_set_level(AV_LOG_QUIET);

  func("VideoLayer :: Registered all codec and format");

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
		av_format_par -> device          = file;
		av_format_par -> standard        = "pal";
		//	av_format_par->channel=0;
		file="";
	}
	frame_number++;

	/**
	 * Decode video
	 */
	len1 = decode_packet(&got_picture);


	if(len1<0) {
	  error("VideoLayer::Error while decoding frame");
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

	  /**
	   * yuv2rgb
	   */
	  img_convert(rgba_picture, PIX_FMT_RGBA32,
		      (AVPicture *)av_frame,
		      enc->pix_fmt,
		      enc->width,
		      enc->height);

	}

      }

      av_free_packet(&pkt); /* sun's good. love's bad */

    } /* end of play_speed while() */

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
		if(backward_control) {
			backward_one_keyframe();
		}

		if(paused || play_speed_control<0) {
			// play_speed_control++; WTF ???????????
			return rgba_picture->data[0];
		}
		else {
			while(play_speed_control>=0) {
				got_it=false;
				play_speed_control--;
				while (!got_it) {
					if(packet_len<=0) {
						/**
						 * Read one packet from the media and put it in pkt
						 */
						while(1) {
							ret = av_read_frame(avformat_context, &pkt);
							/*
							   func ("pkt.data= %d\t",pkt.data);
							   func ("pkt.size= %d\t",pkt.size);
							   func ("pkt.pts= %d\t",pkt.pts);
							   func ("pkt.dts= %d\t",pkt.dts);
							   func ("pkt.duration= %d\n",pkt.duration);
							   func ("avformat_context->start_time= %d\n",avformat_context->start_time);
							   func ("avformat_context->duration= %0.3f\n",avformat_context->duration/AV_TIME_BASE);
							   func ("avformat_context->duration= %d\n",avformat_context->duration);
							   */

							/**
							 * check eof and loop
							 */
							if(ret!= 0) {
								ret=seek(avformat_context->start_time);
								if (ret < 0) {
									error("VideoLayer::could not loop file");
									return NULL;
								}
								continue;
								error("VideoLayer::Error while reading packet");
							}
							else if(pkt.stream_index == video_index)
								break; /* exit loop */
						}
					}
					frame_number++;

					/**
					 * Decode video
					 */
					len1 = decode_packet(&got_picture);


					AVFrame *yuv_picture=&av_frame;
					if(len1<0) {
						error("VideoLayer::Error while decoding frame");
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

						/**
						 * yuv2rgb
						 */
						img_convert(rgba_picture, PIX_FMT_RGBA32, (AVPicture *)yuv_picture, 
                       enc->pix_fmt, 
                        //avformat_stream.codec->pix_fmt,
								enc->width,
								enc->height);

						//		    memcpy(frame_fifo.picture[fifo_position % FIFO_SIZE]->data[0],rgba_picture->data[0],geo.size);
						/* TODO move */
						if(fifo_position == FIFO_SIZE)
							fifo_position=0;

						jmemcpy(frame_fifo.picture[fifo_position]->data[0],rgba_picture->data[0],rgba_picture->linesize[0] * enc->height);
						//			    avpicture_get_size(PIX_FMT_RGBA32, enc->width, enc->height));
						fifo_position++;
					}
				}
				av_free_packet(&pkt); /* sun's good. love's bad */
			} /* end of play_speed while() */
		} /* end of else branch */
		play_speed_control=play_speed;
		//    return rgba_picture->data[0];
		return frame_fifo.picture[fifo_position-1]->data[0];
	}

  play_speed_control=play_speed;

  // return a buffer of size: rgba_picture->linesize[0] * enc->height
  return rgba_picture->data[0];
}

int VideoLayer::decode_packet(int *got_picture) {
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

  avcodec_get_frame_defaults (av_frame);

  int lien = avcodec_decode_video(enc, av_frame, got_picture, ptr,packet_len);

  pts1 = packet_pts;
  /*
    if (avformat_stream->codec.has_b_frames &&
    av_frame.pict_type != FF_B_TYPE) {
    // use last pts 
    packet_pts = video_last_P_pts;
    // get the pts for the next I or P frame if present 
    video_last_P_pts = pts1;
    }
  */
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
  if (av_frame->repeat_pict) {
    frame_delay += av_frame->repeat_pict * (frame_delay * 0.5);
  }
  video_clock += frame_delay;

  /* Debug pts code */
  {
    int ftype;
    if (av_frame->pict_type == FF_B_TYPE)
      ftype = 'B';
    else if (av_frame->pict_type == FF_I_TYPE)
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
	if(enc != NULL) {
		if(enc->codec) {
			avcodec_close(enc);
		}
	}
	if(avformat_context) {
		av_close_input_file(avformat_context);
	}
	free_fifo();
	if(rgba_picture) free_picture(rgba_picture);
	if(deinterlace_buffer) free(deinterlace_buffer);
}


bool VideoLayer::keypress(int key) {
  switch(key) {
  case 'k':
    forward();
    break;
  case 'j':
    backward();
    break;
  case 'p': /* pause */
    pause();
    break;
  case 'm': /* increase playing speed */
    more_speed();
    break;
  case 'n': /* decrease playing speed */
    less_speed();
    break;
    /*
      case 'b':
      if(backward_control) {
      backward_control=false;
      show_osd("backward off");
      }
      else {
      backward_control=true;
      show_osd("backward on");
      }
      //	    backward_one_keyframe();
      break;
    */

  case 'i': /* set mark in */
    set_mark_in();
    break;

  case 'o': /* set mark out */
    set_mark_out();
    break;

  default:
    break;
  }
  return true;
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
void VideoLayer::more_speed() {
  set_speed(1);
}
void VideoLayer::less_speed() {
  set_speed(-1);
}
void VideoLayer::set_speed(int speed) {
  user_play_speed+=speed;
  play_speed+=speed;
  play_speed_control=play_speed;
  show_osd("speed is %d",user_play_speed);
}
bool VideoLayer::forward() {
  relative_seek(+10);
  return true;
}
bool VideoLayer::backward() {
  relative_seek(-10);
  return true;
}
bool VideoLayer::backward_one_keyframe() {
  relative_seek(-1);
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
  if (strcmp(codec->name,"rawvideo")==0)
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
      notice("this video is not seekable!",codec->name);
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
  else // seek with success
    /**
     * Flush buffers, should be called when seeking or when swicthing to a different stream.
     */
    avcodec_flush_buffers(enc);
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

/**
 * Allocate and initialise an AVFrame.
 * this one is from ffmpeg2theora, thanks Jan ;)
 */
AVFrame *VideoLayer::frame_alloc (int pix_fmt, int width, int height) {
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;

    picture = avcodec_alloc_frame ();
    if (!picture)
        return NULL;
    size = avpicture_get_size (pix_fmt, width, height);
    picture_buf = (uint8_t*)av_malloc (size);
    if (!picture_buf){
        av_free (picture);
        return NULL;
    }
    avpicture_fill ((AVPicture *) picture, picture_buf,
            pix_fmt, width, height);
    return picture;
}



#endif
