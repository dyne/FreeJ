/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
	setname("VIDEO");
	frame_number=0;
	av_buf=NULL;
	//	avformat_context=NULL;
	packet_len=0;
	play_speed=0;
	frame_rate=0;
	play_speed_control=0;
	mark_in=0;
	mark_out=0;
    }

VideoLayer::~VideoLayer() {
    close();
}

/*
 * lickme.txt!
 */

bool VideoLayer::init(Context *scr) {
    func("VideoLayer::init");
    paused=false;
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
    //    notice("VideoLayer :: w[%u] h[%u] bpp[%u] size[%u]",
    //	    geo.w,geo.h,geo.bpp,geo.size);
    return true;
}

bool VideoLayer::open(char *file) {
    func("VideoLayer::open(%s)",file);
    AVInputFormat *av_input_format=NULL;
    AVFormatParameters avp, *av_format_par = NULL;

    av_register_all();
    //notice("VideoLayer :: Registered all codec and format");


    if( strncasecmp(file,"/dev/ieee1394/",14)==0) {
	notice("VideoLayer::found dv1394 device!\n");
	grab_dv=true;
	av_input_format = av_find_input_format("dv1394");
	av_format_par = &avp;
	memset(av_format_par, 0, sizeof(*av_format_par));

	/** XXX */
	av_format_par->width=160;
	av_format_par->height=128;
	av_format_par->frame_rate=25;
	av_format_par->frame_rate_base=1;

	av_format_par->device=file;
	av_format_par->standard="PAL";
	av_format_par->channel=0;
	file="";
    }
    /* register all codecs, demux and protocols */

    /**
     * Open media with libavformat
     */
    int err=0;
    /**
     * I *want* it raw, I *want* it bad
     */
    if(grab_dv) {
	err=av_open_input_file(&avformat_context, "", av_input_format, 0, av_format_par);
	printf("VideoLayer:: Grabbing dv\n");
    }
    else
	err = av_open_input_file(&avformat_context, file, av_input_format, 0, av_format_par);
    if (err < 0) {
	error("VideoLayer::open(%s) - can't open ", file);
	//	printf("Error number: %d",err);
	return false;
    }
    err = av_find_stream_info(avformat_context);
    if (err < 0) {
	error("VideoLayer::could not find codec parameters");
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
	//	notice("VideoLayer::Codec type= %d\n",enc->codec_type);
	if (enc->codec_type == CODEC_TYPE_VIDEO) {
	    video_index=i;
	    codec = avcodec_find_decoder(enc->codec_id);
	    if(codec==NULL) {
		error("VideoLayer::Could not find a suitable codec");
		return false;
	    }
	    if(avcodec_open(enc, codec)<0) {
		error("VideoLayer::Could not open codec");
		return false;
	    }
	    else {
		frame_rate=enc->frame_rate/enc->frame_rate_base;
		//		notice("VideoLayer::Opening codec: %s",codec->name);
		//		notice("VideoLayer::codec height: %d",enc->height);
		//		notice("VideoLayer::codec width: %d",enc->width);
		break;
	    }
	}
    }
    avformat_stream=avformat_context->streams[video_index];
    enc = &avformat_stream->codec;
    set_filename(file);
    return(true);
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
    if(mark_in!=0 && mark_out!=0) {
	if (get_master_clock()>=mark_out) 
	seek(mark_in * AV_TIME_BASE/*D ART*/);
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
		    avpicture_fill( av_picture, av_buf, PIX_FMT_RGBA32, enc->width, enc->width );
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
    notice("Closing AVI layer");
    av_free_packet(&pkt);
    if(!enc) avcodec_close(enc);
    if(!av_picture) free(av_picture);
    if(!av_buf) free(av_buf);
    if(!avformat_context) av_close_input_file(avformat_context);
}

bool VideoLayer::keypress(SDL_keysym *keysym) {
    switch(keysym->sym) {
	case SDLK_RIGHT:
	    forward();
	    break;
	case SDLK_LEFT:
	    backward();
	    break;
	case SDLK_KP0: /* pause */
	    pause();
	    break;
	case SDLK_f: /* faster */
	    //	    printf("faster: %d\n",play_speed);
	    more_speed();
	    break;
	case SDLK_s: /* slower */
	    //	    printf("slower: %d\n",play_speed);
	    less_speed();
	    break;

	case SDLK_i: /* set mark in */
	    set_mark_in(); 
	    break;

	case SDLK_o: /* set mark out */
	    set_mark_out(); 
	    break;

	case SDLK_n: 
	default:
	    break;
    }
    return true;
}
bool VideoLayer::set_mark_in() {
    if (mark_in == 0)
	mark_in = get_master_clock();
    else
	mark_in = 0;
    notice("mark_in: %f", mark_in);
    show_osd();
    return true;
}
bool VideoLayer::set_mark_out() {
    if (mark_out == 0)
	mark_out = get_master_clock();
    else
	mark_out = 0;
    notice("mark_out: %f", mark_out);
    show_osd();
    return true;
}
bool VideoLayer::more_speed() {
    return set_speed(1);
}
bool VideoLayer::less_speed() {
    return set_speed(-1);
}
bool VideoLayer::set_speed(int speed) {
    play_speed+=speed;
    play_speed_control=play_speed;
}
bool VideoLayer::forward() {
    relative_seek(+10);
    return true;
}
bool VideoLayer::backward() {
    relative_seek(-10);
    return true;
}
bool VideoLayer::relative_seek(int increment) {
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
	error("VideoLayer::Error seeking file: %d",ret);
	return false;
    }
    unlock_feed();
    show_osd("seek to %f\%",current_time);
    return true;
}
/**
 * Warning! doesn't lock feed
 */
int VideoLayer::seek(int64_t timestamp) {
    return av_seek_frame(avformat_context, video_index,timestamp);
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

#endif
