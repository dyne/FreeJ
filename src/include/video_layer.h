/*  FreeJ
 *  (c) Copyright 2001 Silvano Galliani aka kysucix <silvano.galliani@poste.it>
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
 */

#ifndef __avcodec_h__
#define __avcodec_h__

#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>

#include <layer.h>
#define INBUF_SIZE 4096

class VideoLayer: public Layer {
    private:
	AVCodec *avcodec;
	AVInputFormat *fmt;
	AVFormatContext *avformat_context;
	AVStream *avformat_stream;
	AVPicture *av_picture;
	AVPacket pkt;
	AVCodecContext *enc;
	AVCodec *codec;
	AVFrame av_frame;
	int frame_number;
	int picture_number;
	int packet_len;
	double packet_pts;
	unsigned char *ptr;
	double video_last_P_pts;
	double video_clock;
	double video_current_pts;
	double video_current_pts_time;
	bool paused;

	uint8_t *av_buf;

	bool grab_dv;
	int play_speed;
	int play_speed_control;
	int frame_rate;

	int size, lenght,video_index;
	FILE *fp;

    public:
	VideoLayer();
	~VideoLayer();

	bool init(Context *screen=NULL);
	bool open(char *file);
	void *feed();
	void close();

	bool forward();
	bool backward();
	/*
	   void rewind(framepos_t step=1);
	   void pos(framepos_t p);
	   void pause();
	   */
	//  void speedup();
	//  void slowdown();

	bool keypress(SDL_keysym *keysym);
	bool dump_stream_info(AVFormatContext *s);
	bool forward(int sec);
	bool seek(int increment);
	double get_master_clock();
	void pause();

};

#endif
