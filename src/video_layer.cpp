/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
 *
 *  Code "inspired" from ffplay ;)
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

#ifdef WITH_AVCODEC 

#include <iostream>
#include <string.h>

#include <context.h>
#include <jutils.h>

#include <video_layer.h>
#include "avcodec.h"
#include "avformat.h"


VideoLayer::VideoLayer() 
    :Layer() {
	grab_dv=false;
	set_name("VIDEO");
	frame_number=0;
	av_buf=NULL;
	//	avformat_context=NULL;
	packet_len=0;
	frame_rate=0;
	play_speed=0;
	play_speed_control=0;
	seekable=true;
	enc=NULL;
    }

VideoLayer::~VideoLayer() {
    notice("Closing VID layer");
    free_av_stuff();
    close();
}

/*
 * lickme.txt!
 */

bool VideoLayer::init(Context *scr) {
    func("VideoLayer::init");
    _init(scr, enc->width, enc->height, 32);
    notice("VideoLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	    enc->width,enc->height,32,geo.size);
    notice("VideoLayer :: frame_rate[%d]",frame_rate);

    int saiz = avpicture_get_size( PIX_FMT_RGBA32, enc->width, enc->height );
    av_buf = (uint8_t *)malloc( saiz );
    av_picture = (AVPicture *)malloc( sizeof( AVPicture ) );
    /**
     * feed() function is called 25 times for second so we must correct the speed
     */
    play_speed=(25/frame_rate) - 1; 
    play_speed -= play_speed<<1;
    if(frame_rate==1)
	play_speed=0;
//    notice("play_speed: %d",play_speed);

    /* init variables */
    paused=false;
    user_play_speed=0;
    deinterlace_buffer=NULL;
    deinterlaced=false;

    mark_in=NO_MARK;
    mark_out=NO_MARK;
    return true;
}

bool VideoLayer::open(char *file) {
    int err=0;
    video_index=-1;
    func("VideoLayer::open(%s)",file);
    video_filename=strdup(file);
    AVInputFormat *av_input_format=NULL;
    AVFormatParameters avp, *av_format_par = NULL;

    /** init ffmpeg libraries */
    /* register all codecs, demux and protocols */
    av_register_all();

    /** make ffmpeg silent */
    av_log_set_level(AV_LOG_QUIET);

    func("VideoLayer :: Registered all codec and format");


    if( strncasecmp(file,"/dev/ieee1394/",14)==0) {
	notice("VideoLayer::found dv1394 device!\n");
	grab_dv=true;
	av_input_format = av_find_input_format("dv1394");
	av_format_par = &avp;
	memset(av_format_par, 0, sizeof(*av_format_par));

	/** shit XXX */
	av_format_par->width=160;
	av_format_par->height=128;
	av_format_par->frame_rate=25;
	av_format_par->frame_rate_base=1;

	av_format_par->device=file;
	av_format_par->standard="PAL";
	av_format_par->channel=0;
	file="";
    }

    /**
     * Open media with libavformat
     */
    err = av_open_input_file(&avformat_context, file, av_input_format, 0, av_format_par);
    if (err < 0) {
	error("VideoLayer :: open(%s) - can't open. Error %d", file,err);
	return false;
    }

    /**
     * Find info with libavformat
     */
    err = av_find_stream_info(avformat_context);
    if (err < 0) {
	error("VideoLayer :: could not find stream info");
	return false;
    }
    /**
     * Open codec if we find a video stream
     */
    for(int i=0;i<avformat_context->nb_streams;i++) {
	avformat_stream=avformat_context->streams[i];
	enc = &avformat_stream->codec;
	if(enc == NULL)
	    printf("enc nullo\n");
	//notice("VideoLayer:: Codec type= %d\n",enc->codec_type);
	/**
	 * Here we look for a video stream
	 */
	if (enc->codec_type == CODEC_TYPE_VIDEO) {
	    video_index=i;
	    codec = avcodec_find_decoder(enc->codec_id);
	    if(codec==NULL) {
		error("VideoLayer :: Could not find a suitable codec");
		return false;
	    }
	    if(avcodec_open(enc, codec)<0) {
		error("VideoLayer :: Could not open codec");
		return false;
	    }
	    else {
		frame_rate=enc->frame_rate/enc->frame_rate_base;
		notice("VideoLayer :: Using codec: %s",codec->name);
		if(strncasecmp(codec->name,"dvvideo",7)==0 || 
			strncasecmp(codec->name,"h263",4)==0) {
		    seekable=false;
		    notice("VideoLayer :: video codec not seekable");
		}
		//notice("VideoLayer :: codec height: %d",enc->height);
		//notice("VideoLayer :: codec width: %d",enc->width);
		break;
	    }
	}
    }
    if(video_index<0) {
	error("VideoLayer :: Could not open codec");
	return false;
    }
    avformat_stream=avformat_context->streams[video_index];
    enc = &avformat_stream->codec;
    set_filename(file);
    return true;
}

void *VideoLayer::feed() {
    int got_picture=0;
    int len1=0 ;
    int ret=0;
    bool got_it=false;
    double pts1=0;
    /**
     * follow user video loop
     */
    if(mark_in!=NO_MARK && mark_out!=NO_MARK && seekable) {
	if (get_master_clock()>=mark_out) 
	seek((int64_t)mark_in * AV_TIME_BASE/*D ART*/);
    }

    if(paused || play_speed_control<0) {
	play_speed_control++;
	return av_picture->data[0];
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
			if (pkt.pts != AV_NOPTS_VALUE) {
			    packet_pts = (double)pkt.pts / AV_TIME_BASE;
			}
			/* debug stuff
			   printf("pkt.data= %d\t",pkt.data);
			   printf("pkt.size= %d\t",pkt.size);
			   printf("pkt.pts= %d\t",pkt.pts);
			   printf("pkt.dts= %d\t",pkt.dts);
			   printf("pkt.duration= %d\n",pkt.duration);
			   printf("avformat_context->start_time= %d\n",avformat_context->start_time);
			   printf("avformat_context->duration= %0.3f\n",avformat_context->duration/AV_TIME_BASE);
			   printf("avformat_context->duration= %d\n",avformat_context->duration);
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
		 * Decode the packet and put i(n)t in(t) av_frame
		 */
		if(packet_len<=0) {
		    packet_len=pkt.size; // packet size is zero if packet contains only one frame
		    ptr=pkt.data; /* pointer to frame data */
		}
		/**
		 * In avcodec_get_frame_defaults() avcodec does:
		 *    memset(pic, 0, sizeof(AVFrame));
		 *    pic->pts= AV_NOPTS_VALUE;
		 */
		avcodec_get_frame_defaults(&av_frame);

		len1 = avcodec_decode_video(enc, &av_frame, &got_picture, ptr,packet_len);

		pts1=packet_pts;
		if (avformat_stream->codec.has_b_frames && 
			av_frame.pict_type != FF_B_TYPE) {
		    /* use last pts */
		    packet_pts = video_last_P_pts;
		    /* get the pts for the next I or P frame if present */
		    video_last_P_pts = pts1;
		}
		if (packet_pts != 0) {
		    /* update video clock with pts, if present */
		    video_clock = packet_pts;
		} else {
		    packet_pts = video_clock;
		}
		video_current_pts=packet_pts;
		video_current_pts_time=av_gettime();
		/* update video clock for next frame */
		double frame_delay = (double)avformat_stream->codec.frame_rate_base / 
		    (double)avformat_stream->codec.frame_rate;
		/* for MPEG2, the frame can be repeated, so we update the
		   clock accordingly */
		if (av_frame.repeat_pict) {
		    frame_delay += av_frame.repeat_pict * (frame_delay * 0.5);
		}
		video_clock += frame_delay;

		/* Debug pts code */
		/*
		   {
		   int ftype;
		   if (av_frame.pict_type == FF_B_TYPE)
		   ftype = 'B';
		   else if (av_frame.pict_type == FF_I_TYPE)
		   ftype = 'I';
		   else
		   ftype = 'P';
		   printf("frame_type=%c clock=%0.3f pts=%0.3f\n", 
		   ftype, get_master_clock(), pts1);
		   }
		   */
		AVFrame *src=&av_frame;
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
		    int dst_pix_fmt = PIX_FMT_RGBA32;
		    avformat_stream=avformat_context->streams[video_index];

		    /** Deinterlace input if requested */
		    if(deinterlaced)
			deinterlace((AVPicture *)src);
		    
		    avpicture_fill( av_picture, av_buf, dst_pix_fmt, enc->width, enc->width );
		    img_convert(av_picture, dst_pix_fmt, (AVPicture *)src, avformat_stream->codec.pix_fmt,
			    enc->width,
			    enc->height);
		}
	    }
	    av_free_packet(&pkt); /* sun's good. love's bad */
	} /* end of play_speed while() */
    } /* end of else branch */
    play_speed_control=play_speed;
    return av_picture->data[0];
}

void VideoLayer::close() {
    if(frame_number!=0)
	av_free_packet(&pkt);
    if(enc != NULL) 
	avcodec_close(enc);
    if(avformat_context) { 
	av_close_input_file(avformat_context);
	avformat_context=NULL;
    }
}

void VideoLayer::free_av_stuff() {
    if(!av_picture) free(av_picture);
    if(!av_buf) free(av_buf);
    if(!deinterlace_buffer) free(deinterlace_buffer);
    if(!video_filename) free(video_filename);
}
bool VideoLayer::keypress(char key) {
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
	    
	case 'i': /* set mark in */
	    set_mark_in(); 
	    break;

	case 'o': /* set mark out */
	    set_mark_out(); 
	    break;

	case 'u': /* Swith deinterlace */
	    if(deinterlaced)
		deinterlaced=false;
	    else
		deinterlaced=true;
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
bool VideoLayer::relative_seek(double increment) {
    int ret=0;
    lock_feed();
    double current_time=get_master_clock();
    //    printf("master_clock(): %f\n",current_time);
    current_time+=increment;
    /**
     * Check the seek time is correct! 
     * It should not be before or after the beginning and the end of the movie
     */
    if (current_time<0)  // beginning
	current_time=0;
    /** 
     * Forward in video as a loop 
     */
    else  { // beginning
	while(current_time>(avformat_context->duration/AV_TIME_BASE))  {
	    current_time=current_time - (avformat_context->duration/AV_TIME_BASE);
	}
    }

    //    printf("VideoLayer::seeking to: %f\n",current_time);
    ret=seek((int64_t)current_time*AV_TIME_BASE);
    if (ret < 0) {
	unlock_feed();
	error("VideoLayer :: Error seeking file: %d",ret);
	return false;
    }
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
    if(!seekable) {
	if(seeking_at_beginning_of_stream) {
	    /** close and reopen the stream*/
	    {
		close();
		open(video_filename);
	    }
	    return 0;
	}
	else {
	    error("VideoLayer :: video codec %s not seekable!",codec->name);
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
    ret = av_seek_frame(avformat_context, video_index,timestamp);
    /**
     * Flush buffers, should be called when seeking or when swicthing to a different stream.
     */
    avcodec_flush_buffers(enc);

    if(ret<0) {
	seekable=false;
	if(seeking_at_beginning_of_stream) {
	    /** close and reopen the stream*/
	    {
	    close();
	    open(video_filename);
	    return 0;
	    }
	}
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
	notice("avi pause : %s",(paused)?"on":"off");
	show_osd();
    }
void VideoLayer::deinterlace(AVPicture *picture) {
    int size;
    AVPicture *picture2;
    AVPicture picture_tmp;

    /* create temporary picture */
    size = avpicture_get_size(enc->pix_fmt, enc->width, enc->height);
    if(deinterlace_buffer==NULL)
	deinterlace_buffer = (uint8_t *)av_malloc(size);
    if (!deinterlace_buffer)
	return ;

    picture2 = &picture_tmp;
    avpicture_fill(picture2, deinterlace_buffer, enc->pix_fmt, enc->width, enc->height);

    if(avpicture_deinterlace(picture2, picture, 
		enc->pix_fmt, enc->width, enc->height) < 0) {
	/* if error, do not deinterlace */
	av_free(deinterlace_buffer);
	deinterlace_buffer = NULL;
	picture2 = picture;
    }
    if (picture != picture2)
	*picture = *picture2;
//    av_free(deinterlace_buffer);
    return;
}

#endif
