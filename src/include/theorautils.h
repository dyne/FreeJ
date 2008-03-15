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

#include <stdint.h>
#include "theora/theora.h"
#include "vorbis/codec.h"
#include "vorbis/vorbisenc.h"
#include "ogg/ogg.h"

#include <ringbuffer.h>

// #define OGGMUX_DEBUG

#define SKELETON_VERSION_MAJOR 3
#define SKELETON_VERSION_MINOR 0
#define FISHEAD_IDENTIFIER "fishead\0"
#define FISBONE_IDENTIFIER "fisbone\0"
#define FISBONE_SIZE 52
#define FISBONE_MESSAGE_HEADER_OFFSET 44

typedef struct
{
    /* the file the mixed ogg stream is written to */
	ringbuffer_t *ringbuffer;
	int bytes_encoded;

    int audio_only;
    int video_only;
    int with_skeleton;
    int frontend;
    /* vorbis settings */
    int sample_rate;
    int channels;
    double vorbis_quality;
    int vorbis_bitrate;

    vorbis_info vi;       /* struct that stores all the static vorbis bitstream settings */
    vorbis_comment vc;    /* struct that stores all the user comments */

    /* theora settings */
    theora_info ti;
    theora_comment tc;

    /* state info */    
    theora_state td;
    vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
    vorbis_block vb;     /* local working space for packet->PCM decode */

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
    time_t start_time;

    //to do some manual page flusing
    int v_pkg;
    int a_pkg;
#ifdef OGGMUX_DEBUG
    int a_page;
    int v_page;
#endif
}
oggmux_info;

extern void init_info(oggmux_info *info);
extern void oggmux_init (oggmux_info *info);
extern void oggmux_add_video (oggmux_info *info, yuv_buffer *yuv, int e_o_s);
extern void oggmux_add_audio (oggmux_info *info, int16_t * readbuffer, int bytesread, int samplesread,int e_o_s);
extern void oggmux_add_audio_float (oggmux_info *info, float * buffer, int samples, int e_o_s);
extern void oggmux_flush (oggmux_info *info, int e_o_s);
extern void oggmux_close (oggmux_info *info);
