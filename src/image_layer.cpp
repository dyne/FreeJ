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

#include <config.h>

#ifdef WITH_SDL_IMAGE

#include "image_layer.h"
#include "SDL.h"
#include "SDL_image.h"
#include <context.h>
#include <jutils.h>

ImageLayer::ImageLayer()
    :Layer() {
	black_image=NULL;
	subliminal=0;
	blinking=false;
	count=0;
	set_name("IMG");
    }

ImageLayer::~ImageLayer() {
    close();
}
bool ImageLayer::open(char *file) {
    image = IMG_Load(file);
    if(!image) {
	error("ImageLayer::open() problems loading %s: %s", file,IMG_GetError());
	return false;
    }
    /**
     * Convert to display pixel format if the images is not 32 bit per pixel
     */
    if(image->format->BitsPerPixel != 32) {
	image = SDL_DisplayFormat(image);
    }
    return true;
}
bool ImageLayer::init(Context *scr) {
    func("ImageLayer::init");
    _init(scr, image->w, image->h, image->format->BitsPerPixel);
    notice("ImageLayer :: w[%u] h[%u] bpp[%u] size[%u]",
	    image->w, image->h,32,geo.size);

    /** allocate memory for the black image */
    black_image = jalloc(black_image,geo.size);

    // paint it black!
    black_image = memset(black_image,0,geo.size);

    return true;
}
void *ImageLayer::feed() {
    count++;
    // threads problems !! XXX TODO 
    if(count==freej->fps_speed)
	count=0;
    if(blinking && count%2!=0) {
	return black_image;
    }
    // subliminal mode
    if(subliminal!=0 && count>= subliminal)
	return black_image;

    return image->pixels;
}

bool ImageLayer::keypress(char key) {
    bool res = true;

    switch(key) {
	case 'o':
	    subliminal++;
	    act("ImageLayer::subliminal count %i",subliminal);
	    break;
	case 'p':
	    subliminal=0;
	    act("ImageLayer::subliminal reset");
	    break;
	case 'b':
	    if(blinking)
		blinking=false;
	    else 
		blinking=true;
	    act("ImageLayer::blinking %s",(blinking)?"ON":"OFF");
	    break;

	default:
	    res = false;
	    break;
    }
    return res;
}


void ImageLayer::close() {
    func("ImageLayer::close()");
    SDL_FreeSurface(image);
}
#endif
