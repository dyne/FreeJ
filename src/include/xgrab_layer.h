/*  FreeJ
 *  (c) Copyright 2007 C. Rudorff aka MrGoil <goil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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
 * "$Id: image_layer.h 926 2007-10-05 21:21:44Z jaromil $"
 *
 */
#ifndef __FREEJ_XGRAB_H__
#define __FREEJ_XGRAB_H__

#include <config.h>
#ifdef WITH_XGRAB

#include <SDL.h>
#include <layer.h>

//#include <screen.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
//#include <X11/extensions/Xvlib.h>

class XGrabLayer: public Layer {
	protected:
		//ViewPort *screen;
		//SDL_Surface *surf;
	private:
		//void run(); ///< Main Layer thread loop
		//bool cafudda();
		void resize();
		bool autosize, mapped, unobscured;
		ScreenGeometry crop;
		struct crop {
			uint16_t x;
			uint16_t y;
			int16_t w; // =<0 = from right
			int16_t h; // =<0 = from bottom
		};
		// X stuff
		Display *display;
		int screen_num;
		Window win;
		XWindowAttributes wa;
		//XSetWindowAttributes win_sattr;
		XImage *ximage;
		//XImage *ximage_new;
		// OLD
		//int screen_num;
		//GC gc;
		//unsigned int border_width;
		//char *pixbuffer;
		//XvPortID xv_port;

		// SDL
		//SDL_Surface *sdlimage;
		//SDL_Rect sdl_rect_src;

	public:
		XGrabLayer();
		~XGrabLayer();

		bool open();
		bool open(uint32_t win_id);
		bool open(const char *);
		bool init(Context *freej);
		bool init(Context *freej, int w, int h);
		//void *feed();
		void close();

		void *feed();
};
#endif // WITH_XGRAB


#if 0
       SDL_Surface *SDL_GetVideoSurface(void);

       typedef struct SDL_Surface {
               Uint32 flags;                           /* Read-only */
               SDL_PixelFormat *format;                /* Read-only */
               int w, h;                               /* Read-only */
               Uint16 pitch;                           /* Read-only */
               void *pixels;                           /* Read-write */

               /* clipping information */
               SDL_Rect clip_rect;                     /* Read-only */

               /* Reference count -- used when freeing surface */
               int refcount;                           /* Read-mostly */

            /* This structure also contains private fields not shown here */
       } SDL_Surface;


#endif

#endif
