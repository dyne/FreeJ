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


#include <iostream>
#include <string.h>

#include <context.h>
#include <jutils.h>

#include <video_layer.h>
#include "avcodec.h"
#include "avformat.h"



VideoLayer::VideoLayer() 
    :Layer() {
	setname("AVI");
	frame_number=0;
	av_buf=NULL;
	//avformat_context=NULL;
	packet_len=0;
    }

VideoLayer::~VideoLayer() {
    close();
}

/*
 * Here we have to anal-ize the media and identify the codec and the format.
 * So we can better initialize livabcodec
 */

bool VideoLayer::init(Context *scr) {
    func("VideoLayer::init");
    paused=false;
    notice("VideoLayer :: Registered all codec and format");
    _init(scr, enc->width, enc->height, 32);
    notice("VideoLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	    enc->width,enc->height,32,geo.size);

    int saiz = avpicture_get_size( PIX_FMT_RGBA32, enc->width, enc->height );
    av_buf = (uint8_t *)malloc( saiz );
    av_picture = (AVPicture *)malloc( sizeof( AVPicture ) );
    notice("VideoLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	    geo.w,geo.h,geo.bpp,geo.size);
    return true;
}

bool VideoLayer::open(char *file) {
    func("VideoLayer::open(%s)",file);
    AVInputFormat *av_input_format=NULL;
    AVFormatParameters avp, *av_format_par = NULL;

    av_register_all();


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
    if(grab_dv) {
        err=av_open_input_file(&avformat_context, "", av_input_format, 0, av_format_par);
	printf("Grabbing dv\n");
    }
    else
	err = av_open_input_file(&avformat_context, file, av_input_format, 0, av_format_par);
    if (err < 0) {
	error("VideoLayer::open(%s) - can't open ", file);
	printf("Error number: %d",err);
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
		notice("VideoLayer::Opening codec: %s",codec->name);
		notice("VideoLayer::codec height: %d",enc->height);
		notice("VideoLayer::codec width: %d",enc->width);
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
	if(paused) 
	    return av_picture->data[0];
	int got_picture=0;
	int len1=0 ;
	int ret=0;
	bool got_it=false;
	double pts1=0;

	/**
	 * Read one packet from the media ant put it in pkt
	 */
	while (!got_it) {
	    if(packet_len<=0) {
		while(1) {
		    ret = av_read_frame(avformat_context, &pkt);
		    if (pkt.pts != AV_NOPTS_VALUE) {
			packet_pts = (double)pkt.pts / AV_TIME_BASE;
		    }
		    /**
		     * check eof and loop
		     */
		    if(pkt.data == NULL || pkt.size == 0) {
			ret=av_seek_frame(avformat_context, video_index,avformat_context->start_time);
			if (ret < 0) {
			    error("VideoLayer::could not loop file");
			    return NULL;
			}
			continue;
		    }
		    if(ret!= 0) {
			error("VideoLayer::Error while reading packet");
		    }
		    else if(pkt.stream_index == video_index)
			break;
		}
	    }
	    frame_number++;

	    /**
	     * Decode the packet and put i(n)t in(t) av_frame
	     */
	    if(packet_len<=0) {
		packet_len=pkt.size; // packet size
		ptr=pkt.data;
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
	    
	    /* Debug pts code
	    {
		int ftype;
		if (av_frame.pict_type == FF_B_TYPE)
		    ftype = 'B';
		else if (av_frame.pict_type == FF_I_TYPE)
		    ftype = 'I';
		else
		    ftype = 'P';
		printf("frame_type=%c clock=%0.3f pts=%0.3f\n", 
			ftype, packet_pts, pts1);
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
	case SDLK_KP0: 
	    pause();
	    break;
	default:
	    break;
    }
    return true;
}
bool VideoLayer::forward() {
    seek(+10);
    return true;
}
bool VideoLayer::backward() {
    seek(-10);
    return true;
}
bool VideoLayer::seek(int increment) {
    int ret=0;
    lock_feed();
    double current_time=get_master_clock();
//    printf("master_clock(): %f\n",current_time);
    current_time+=increment;
    if (current_time<0) {
	unlock_feed();
	return true;
    }
//   printf("current_time: %f\n",current_time);
    ret=av_seek_frame(avformat_context, video_index,(int64_t)current_time*AV_TIME_BASE);
    if (ret < 0) {
	error("VideoLayer::Error seeking file");
	return false;
    }
    unlock_feed();
    show_osd("seek to %f\%",current_time);
    return true;
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

