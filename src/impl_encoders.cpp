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

#include <encoder.h>
#include <jutils.h>

#include <impl_encoders.h>

Encoder *get_encoder(char *output_filename) {
	Encoder *e = NULL;
	/*
	if (strncasecmp( strrchr(filename,'.'), ".ogg",4)) {
		return (new OggTheoraEncoder());
		else
			return (new FFmpegEncoder());
	*/
#ifdef WITH_AVCODEC // TODO let select ffmpeg encoder
	e = new OggTheoraEncoder(output_filename);
#endif
	return e;
}
