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

#include <string.h>
#include <context.h>

#include <encoder.h>

Encoder::Encoder(char *output_filename) {
	started = false;
	env = NULL;
	set_output_name(output_filename);
}
Encoder::~Encoder() {
}
bool Encoder::_init(Context *_env) {
	if(_env == NULL)
		return false;
	env=_env;
	return true;
}
bool Encoder::set_output_name(char *output_filename) {
	int filename_number=1;

	if(!output_filename)
	{
		error("FFmpegEncoder:init::Invalid filename");
	}

	// save filename in the object
	filename = strdup(output_filename);

	/*
	 * Test if file exists, and append a number.
	 */
	char nuova[512];
	FILE *fp;

	while ( (fp = fopen(filename,"r"))!=NULL) {// file already exists!
		// take point extension pointer ;)
		char *point = strrchr(output_filename,'.');

		int lenght_without_extension = (point - output_filename);

		// copy the string before the  point
		strncpy (nuova,output_filename, lenght_without_extension);

		// insert -n
		sprintf (nuova + lenght_without_extension, "-%d%s", filename_number,output_filename + lenght_without_extension);
		jfree(filename);
		filename = strdup(nuova);

		fclose(fp);
		// increment number inside filename
		filename_number++;
	}

	notice ("Encoder:_init::filename %s saved",filename);
	return true;
}

bool Encoder::set_sdl_surface(SDL_Surface *surface) {
	if(surface == NULL)
		return false;
	this->surface = surface;
	return true;
}
char *Encoder::get_filename() {
	return filename;
}
