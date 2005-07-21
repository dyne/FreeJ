/* -*- c-basic-offset: 8; -*- */
/* theora.c: Ogg Theora data handlers for libshout
 * $Id: theora.c 7194 2004-07-20 21:46:55Z brendan $
 *
 *  Copyright (C) 2004 the Icecast team <team@icecast.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <theora/theora.h>

#include "shout_private.h"
#include "shout_ogg.h"

/* -- local data structures -- */
typedef struct {
	theora_info ti;
	theora_comment tc;
	uint32_t granule_shift;
	double prev_time;
} theora_data_t;

/* -- local prototypes -- */
static int read_theora_page(ogg_codec_t *codec, ogg_page *page);
static void free_theora_data(void *codec_data);
static int theora_ilog(unsigned int v);

/* -- theora functions -- */
int _shout_open_theora(ogg_codec_t *codec, ogg_page *page)
{
	ogg_packet packet;

	theora_data_t *theora_data = calloc(1, sizeof(theora_data_t));
	if (! theora_data)
		return SHOUTERR_MALLOC;

	theora_info_init(&theora_data->ti);
	theora_comment_init(&theora_data->tc);

	ogg_stream_packetout(&codec->os, &packet);
	
	if (theora_decode_header(&theora_data->ti, &theora_data->tc, &packet) < 0) {
		free_theora_data(theora_data);

		return SHOUTERR_UNSUPPORTED;
	}

	codec->codec_data = theora_data;
	codec->read_page = read_theora_page;
	codec->free_data = free_theora_data;
	codec->headers = 1;

	return SHOUTERR_SUCCESS;
}

static int read_theora_page(ogg_codec_t *codec, ogg_page *page)
{
	theora_data_t *theora_data = codec->codec_data;
	ogg_packet packet;
	double per_frame, duration, new_time;
	ogg_int64_t granulepos, iframe, pframe;
	uint64_t frames;

	if (ogg_page_granulepos(page) == 0)
	{
		while (ogg_stream_packetout(&codec->os, &packet) > 0) {
			if (theora_decode_header(&theora_data->ti, &theora_data->tc, &packet) < 0)
				return SHOUTERR_INSANE;
			codec->headers++;
		}
		if (codec->headers == 3) {
			theora_data->prev_time = 0;
			theora_data->granule_shift = theora_ilog(theora_data->ti.keyframe_frequency_force - 1);
		}

		return SHOUTERR_SUCCESS;
	}

	per_frame = (double)theora_data->ti.fps_denominator / theora_data->ti.fps_numerator * 1000000;
	granulepos = ogg_page_granulepos(page);

	if (granulepos > 0) {
		iframe = granulepos >> theora_data->granule_shift;
		pframe = granulepos - (iframe << theora_data->granule_shift);
		frames = iframe + pframe;
		new_time = (frames  * per_frame);

		duration = new_time - theora_data->prev_time;
		theora_data->prev_time = new_time;

		codec->senttime += (uint64_t)(duration + 0.5);
	}

	return SHOUTERR_SUCCESS;
}

static void free_theora_data(void *codec_data)
{
	theora_data_t *theora_data = (theora_data_t *)codec_data;

	theora_info_clear(&theora_data->ti);
	theora_comment_clear(&theora_data->tc);
	free(theora_data);
}

static int theora_ilog(unsigned int v)
{
	int ret = 0;

	while (v) {
		ret++;
		v >>= 1;
	}

	return ret;
}
