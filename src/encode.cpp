/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <kysucix@dyne.org>
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
#include <encode.h>
#include <string.h>
#include <context.h>
#include <sdl_screen.h>

Encode::Encode(Context *_env) { 
  env=_env;
  func("Encode::Encode::Encode object created");
}
Encode::~Encode() {
  avcodec_close(codec);
  free_av_objects();
  
  // finish writing the file
  av_write_trailer(afc);
  // free the streams
  for(int i = 0; i < afc->nb_streams; i++) {
    free(&afc->streams[i]);
  }
  
  if (!(aof->flags & AVFMT_NOFILE)) {
    /* close the output file */
    url_fclose(&afc->pb);
  }
  /* free the stream */
  free(afc);
}

void Encode::free_av_objects() {
  free(picture);
  free(tmp_picture);
  free(video_outbuf);
}

char *Encode::init(char *output_filename) { 
    if(!output_filename)
    {
	error("Encode:init::Invalid filename");
    }
    // save filename in the object
    filename=strdup(output_filename);

    // initialize libavcodec, and register all codecs and formats
    av_register_all();

    aof = guess_format(NULL,output_filename,NULL);
    if(!aof) {
	error("Encodec::init::Can't find a correct format for %s. I will use avi",output_filename);
	aof=guess_format("avi",NULL,NULL);
    }

    // create format context
    afc = av_alloc_format_context();
    if(!afc) {
	error("Encode::init::Error allocating AVFormatContext!");
	return NULL;
    }
    afc->oformat=aof;
    if(sizeof(filename)>MAX_FILE_LENGHT) {
	error("Encode::init::Filename longer than %d!",MAX_FILE_LENGHT);
	return NULL;
    }
    // copy filename inside the struct afc
    snprintf(afc->filename, sizeof(afc->filename), "%s", filename);

    // create stream and set parameter
    if (aof->video_codec != CODEC_ID_NONE) {
	video_stream = av_new_stream(afc,0);
	if(!video_stream) {
	    error("Encode::init::Error allocating AVFormatContext!");
	    return NULL;
	}
	set_encoding_parameter();
    }
    // set the output parameters in avcodec
    if (av_set_parameters(afc, NULL) < 0) {
        error("Encode::init::Invalid output format parameters\n");
        exit(1);
    }
    // DEBUG stuff
    dump_format(afc, 0, output_filename, 1);

    // find the video encoder
    AVCodecContext *acc=&video_stream->codec;
    AVCodec *codec = avcodec_find_encoder(acc->codec_id);
    if(!codec) {
	error("Encode:init::Couldn't find encoder");
	return NULL;
    }

    /* open the codec */
    if (avcodec_open(acc, codec) < 0) {
	error("Encode:init::Couldn't open codec");
	return NULL;
    }
    video_outbuf = NULL;
    if (!(afc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        video_outbuf_size = 200000;
        video_outbuf = (uint8_t *)av_malloc(video_outbuf_size);
	if(!video_outbuf)
	    return NULL;
    }

    /* allocate the encoded raw picture */
    picture = avcodec_alloc_frame();
    //picture = (AVPicture *)malloc(sizeof(AVPicture));

    if (!picture) {
        error("Could not allocate picture\n");
	return NULL;
    }
    int size = avpicture_get_size(acc->pix_fmt, acc->width, acc->height);
    uint8_t *video_outbuf_tmp=(uint8_t *)av_malloc(size);
    if (!video_outbuf_tmp) {
	av_free(picture);
	return NULL;
    }
    avpicture_fill((AVPicture *)picture, video_outbuf_tmp, acc->pix_fmt, acc->width, acc->height);

    /* if the output format is not YUV420P, then a temporary YUV420P
       picture is needed too. It is then converted to the required
       output format */
    tmp_picture = NULL;
    if (acc->pix_fmt != PIX_FMT_YUV420P) {
	AVFrame *picture_tmp = avcodec_alloc_frame();
	if (!picture_tmp) {
	    fprintf(stderr, "Could not allocate temporary picture\n");
	    exit(1);
	}
	tmp_picture=(uint8_t *)av_malloc(size);
	if (!tmp_picture) {
	    av_free(picture_tmp);
	    return NULL;
	}
	avpicture_fill((AVPicture *)picture_tmp, tmp_picture, PIX_FMT_YUV420P, acc->width, acc->height);
    }
    open();
}

bool Encode::write_frame() {
    AVFrame *picture_ptr;
    int ret=0;
    int packet_size;
    double video_pts;

    codec=&video_stream->codec;

//    video_pts = (double)video_stream->pts.val * video_stream->time_base.num / video_stream->time_base.den;

    avcodec_alloc_frame();
    prepare_image(picture_ptr);

    int size=-1;
    if(STREAM_IS_RAW(afc)) {
	packet_size=sizeof(AVPicture);
    }
    else {
        size = avcodec_encode_video(codec, video_outbuf, video_outbuf_size, picture);
	picture_ptr=(AVFrame *)video_outbuf;
	packet_size=size;
    }

    /* if zero size, it means the image was buffered */
    if(size!=0) {
	AVPacket av_packet;
	av_init_packet(&av_packet);

	// packet parameters
	av_packet.pts= codec->coded_frame->pts;
	if(size==-1 || IS_KEYFRAME(codec))
	    av_packet.flags |= PKT_FLAG_KEY;
	av_packet.stream_index= video_stream->index;
	av_packet.data= (uint8_t *)picture_ptr;
	av_packet.size= packet_size;

	// write to HARD disk
	ret = av_write_frame(afc, &av_packet);
    }

    if(ret != 0) {
	error("Encode :: Error while writing frame number %d",frame_count);
	return false;
    }
    frame_count++;
}
AVFrame *Encode::prepare_image(AVFrame *picture_ptr) {
    SdlScreen *sdl_screen;
//    if(frame_count>STREAM_NB_FRAMES) {
//	picture_ptr=NULL;
//    }
//    else {
	//	if(!STREAM_IS_YUV420P(codec))
	//	    convert_to_YUV420P();
	//	else
	//	    create_dummy_image(picture,codec);
	sdl_screen = (SdlScreen *)env->screen;
	picture_ptr->data[0] =(uint8_t *)sdl_screen->screen->pixels;
	picture_ptr->data[1] = NULL;
	picture_ptr->data[2] = NULL;
	picture_ptr->linesize[0] = sdl_screen->w * 4;

//    }
//        avpicture_fill( (AVPicture *)picture, (uint8_t *)picture_ptr, PIX_FMT_YUV420P, codec->width, codec->width );
    img_convert((AVPicture *)picture,PIX_FMT_YUV420P,(AVPicture *)picture_ptr,PIX_FMT_RGBA32,
	    sdl_screen->w,
	    sdl_screen->h);
    return picture_ptr;
}
void Encode::open() { 
    /* open the output file, if needed */
    if (!(aof->flags & AVFMT_NOFILE)) {
        if (url_fopen(&afc->pb, filename, URL_WRONLY) < 0) {
            fprintf(stderr, "Could not open '%s'\n", filename);
            exit(1);
        }
    }
    /* write the stream header, if any */
    av_write_header(afc);
}


void Encode::convert_to_YUV420P() { 
//    create_dummy_image(tmp_picture,codec);
    img_convert((AVPicture *)picture, codec->pix_fmt, (AVPicture *)tmp_picture, PIX_FMT_YUV420P,
	    codec->width,
	    codec->height);
}
void Encode::set_encoding_parameter() { 
    AVCodecContext *acc=&video_stream->codec;

    /** Set video codec from the format context */
    acc->codec_id=aof->video_codec;

    acc->codec_type = CODEC_TYPE_VIDEO;

    /* sample parameters */
    acc->bit_rate = 400000;

    /* set resolution accordingly to the global Viewport*/
    acc->width = env->screen->w;
    acc->height = env->screen->h;

    /* frames per second */
    acc->frame_rate = env->fps_speed;  
    acc->frame_rate_base = 1;

    acc->gop_size = 12; /* emit one intra frame every twelve frames at most */

    if (acc->codec_id == CODEC_ID_MPEG1VIDEO){
        /* needed to avoid using macroblocks in which some coeffs overflow 
           this doesnt happen with normal video, it just happens here as the 
           motion of the chroma plane doesnt match the luma plane */
        acc->mb_decision=2;
    }
    // some formats want stream headers to be seperate
    if(!strcmp(afc->oformat->name, "mp4") || !strcmp(afc->oformat->name, "mov") || !strcmp(afc->oformat->name, "3gp"))
        acc->flags |= CODEC_FLAG_GLOBAL_HEADER;
}
#endif
