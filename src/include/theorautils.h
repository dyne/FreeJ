/* -*- tab-width:4;c-file-style:"cc-mode"; -*- */
/*
 * theorautils.h -- Ogg Theora/Ogg Vorbis Abstraction and Muxing
 * Copyright (C) 2003-2005 <j@v2v.cc>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef _F2T_THEORAUTILS_H_
#define _F2T_THEORAUTILS_H_

#include <config.h>

#include <stdint.h>
#include <theora/theora.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#ifdef HAVE_KATE
#include <kate/kate.h>
#endif
#include <ogg/ogg.h>

#include <ringbuffer.h>
#include <time.h>

//#define OGGMUX_DEBUG 1



#define SKELETON_VERSION_MAJOR 3
#define SKELETON_VERSION_MINOR 0
#define FISHEAD_IDENTIFIER "fishead\0"
#define FISBONE_IDENTIFIER "fisbone\0"
#define FISBONE_SIZE 52
#define FISBONE_MESSAGE_HEADER_OFFSET 44

typedef struct
{
#ifdef HAVE_KATE
    kate_state k;
    kate_info ki;
    kate_comment kc;
#endif
    ogg_stream_state ko;    /* take physical pages, weld into a logical
                             * stream of packets */
    int katepage_valid;
    unsigned char *katepage;
    int katepage_len;
    int katepage_buffer_length;
    double katetime;
}
oggmux_kate_stream;

typedef void TIMER;

typedef struct
{
    /* the file the mixed ogg stream is written to */
	ringbuffer_t *ringbuffer;
	int bytes_encoded;
    FILE *outfile;

    int audio_only;
    int video_only;
    int with_skeleton;
    FILE *frontend;
    /* vorbis settings */
    int sample_rate;
    int channels;
    double vorbis_quality;
    int vorbis_bitrate;
    long aveVorBitRate;	  // average audio bit rate seted by oggmux_init function
	char status[512]; // status string line

    vorbis_info vi;       /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment vc;    /* struct that stores all the user comments */

    /* theora settings */
    theora_info ti;
    theora_comment tc;
    int speed_level;

    /* state info */
    theora_state td;
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block vb;     /* local working space for packet->PCM decode */

    int with_kate;

    /* used for muxing */
    ogg_stream_state to;    /* take physical pages, weld into a logical
                             * stream of packets */
    ogg_stream_state vo;    /* take physical pages, weld into a logical
                             * stream of packets */
    ogg_stream_state so;    /* take physical pages, weld into a logical
                             * stream of packets, used for skeleton stream */

    int audiopage_valid;
    int videopage_valid;
    unsigned char *audiopage;
    unsigned char *videopage;
    int videopage_len;
    int audiopage_len;
    int videopage_buffer_length;
    int audiopage_buffer_length;

    /* some stats */
    double audiotime;
    double videotime;
    double duration;

    int vkbps;
    int akbps;
    ogg_int64_t audio_bytesout;
    ogg_int64_t video_bytesout;
    ogg_int64_t kate_bytesout;
    time_t start_time;

    //to do some manual page flusing
    int v_pkg;
    int a_pkg;
    int k_pkg;
#ifdef OGGMUX_DEBUG
    int a_page;
    int v_page;
    int k_page;
#endif

    int n_kate_streams;
    oggmux_kate_stream *kate_streams;
}
oggmux_info;

void init_info(oggmux_info *info);
extern void oggmux_setup_kate_streams(oggmux_info *info, int n_kate_streams);
extern void oggmux_init (oggmux_info *info);
extern void oggmux_add_video (oggmux_info *info, yuv_buffer *yuv, int e_o_s);
extern void oggmux_add_audio (oggmux_info *info, float * readbuffer, int bytesread, int samplesread,int e_o_s);
extern void oggmux_add_kate_text (oggmux_info *info, int idx, double t0, double t1, const char *text, size_t len);
extern void oggmux_add_kate_end_packet (oggmux_info *info, int idx, double t);
extern void oggmux_flush (oggmux_info *info, int e_o_s);
extern void oggmux_close (oggmux_info *info);


#endif
