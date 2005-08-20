/*  FreeJ
 *  (c) Copyright 2005 Silvano Galliani aka kysucix <kysucix@dyne.org>
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
#ifndef __img_h__
#define __img_h__

#include <SDL.h>
#include <SDL_image.h>
#include <layer.h>

class ImageLayer: public Layer {
    private:
	SDL_Surface *image;
	SDL_Surface *display_image;

	void *black_image;

	/** how many times show image  when in subliminal mode */
	int subliminal;

	bool blinking;

	int count;

    public:
	ImageLayer();
	~ImageLayer();

	bool init(int width, int height);

	bool open(char *file);
	void *feed();

	void close();

	bool keypress(char key);
};

#endif
