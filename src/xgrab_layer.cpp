/*  FreeJ
 *  (c) Copyright 2007 C. Rudorff <goil@dyne.org>
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
 * "$Id: xgrab_layer.cpp 922 2007-09-05 19:17:31Z mrgoil $"
 *
 * some parts shameless stolen from avidemux GUI_xvDraw.cpp
 */

// #include <context.h>
#include <jutils.h>
#include <xgrab_layer.h>
#include <config.h>

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <sdl_screen.h>
#include <SDL.h>
#include "SDL_rotozoom.h"

XGrabLayer::XGrabLayer()
	:Layer() {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);

	//surf = NULL;
	opened = false;
	ximage = NULL;
	win = 0;
	//gc = NULL;
	
	set_name("XGR");
	jsclass = &js_xgrab_class;
}

XGrabLayer::~XGrabLayer() {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	close();
}

bool XGrabLayer::open(char *file) {
	error("%s: not supported", __PRETTY_FUNCTION__);
	return false;
}
bool XGrabLayer::open() {
	if (opened)
		return 0;
	if (!win) {
		error("%s: no win_id set", __PRETTY_FUNCTION__); 
		return false;
	}
	return open(win);
}

bool XGrabLayer::open(uint32_t win_id_new) {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	if (opened)
		return 0;

	char errmsg[MAX_ERR_MSG];
// check win id ok
//set_filename(display_name); <- win title ...

	if ( (display=XOpenDisplay(NULL)) == NULL ) {
		snprintf(errmsg, MAX_ERR_MSG, 
			"Can't connect to X server");
		goto fail;
	}
	//screen_num = DefaultScreen(display);
	

	XWindowAttributes wa;
	if (!XGetWindowAttributes(display, win_id_new, &wa)) {
		snprintf(errmsg, MAX_ERR_MSG, 
			"Can't get win attributes");
		goto fail;

	}
	lock();
	geo.w = wa.width;
	geo.h = wa.height;
	geo.bpp = 32;
	geo.size = geo.w*geo.h*(geo.bpp/8);
	geo.pitch = geo.w*(geo.bpp/8);
	unlock();

	func("xwin depth:%u ", wa.depth);
	//CWBackingStore
	//wa.backing_store // NotUseful, WhenMapped, Always
	//CWSaveUnder
	//wa.save_under // bool
	//
	//CWEventMask wa.event_mask ...
	win_sattr.backing_store = Always;
	win_sattr.save_under = True;
	if (!XChangeWindowAttributes(display, win_id_new, 
		CWBackingStore|CWSaveUnder, &win_sattr)) {
		snprintf(errmsg, MAX_ERR_MSG, 
			"Can't set win attributes");
		goto fail;
	}
    XSync (display, False);
	// save old values
	win_sattr.backing_store = wa.backing_store;
	win_sattr.save_under = wa.save_under;

	win = win_id_new;
	opened = true;
	active = true;
	return true;

	fail:
		error(errmsg); // TODO: JS exception
		close();
		return false;
}

#if 0
	/* Create GC for drawing */
	XGCValues gcv;
	gcv.function = GXcopy;
	gcv.graphics_exposures = False;
	gcv.fill_style=FillSolid;
	//gc = XCreateGC(display, win, GCFunction|GCGraphicsExposures|GCForeground|GCBackground|GCFillStyle, &gcv);
	gc = XCreateGC(display, win, GCFunction|GCGraphicsExposures|GCFillStyle, &gcv);
	//gc = XCreateGC(display, win, 0, &gcv);
	if (gc == NULL) {
		sprintf(errmsg, "XCreateGC failed");
		goto fail;
	}
	//XFillRectangle(display, win, gc, 80, 120, 100, 50);
	//XSetForeground(display, gc, 0);
	//XDrawRectangle(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height);

	/* Display window */
	XMapWindow(display, win);

	/* create pixel buffer */
	// XGetWindowAttributes (display, win, &xwa);
	/*
	sdlimage = SDL_CreateRGBSurface
	  (SDL_SWSURFACE, geo.w, geo.h, geo.bpp,
	  red_bitmask,green_bitmask,blue_bitmask,alpha_bitmask);
	pixbuffer = (char*)sdlimage->pixels;

	sdl_rect_src.x = geo.x;
	sdl_rect_src.y = geo.y;
	sdl_rect_src.w = geo.w;
	sdl_rect_src.h = geo.h;
	*/
	{ // use main pixbuffer
		SDL_Surface* src = env->screen->screen;
		pixbuffer = (char*)src->pixels;
		xvimage = XvCreateImage(display, xv_port, xv_format, pixbuffer,
				  src->w, src->h);
	}
	func("Xv pix id: 0x%lx", xvimage);

	//memset(xvimage->data, 0x30405060, xvimage->data_size);

	/* events */
	XSelectInput(display, win, ExposureMask | KeyPressMask |
		ButtonPressMask | StructureNotifyMask);

	notice("VP opened %s :: w[%u] h[%u] (%u bytes)",
	 display_name, geo.w, geo.h, geo.size);
	//func("VP X: depth:%i r,g,b mask: %x,%x,%x", 
	//xwa.depth, ximage->red_mask, ximage->green_mask, ximage->blue_mask);

	opened = true;
	return true;

	fail:
		error(errmsg); // TODO: JS exception
		close();
		return false;
}
#endif

bool XGrabLayer::init(Context *freej) {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	return init(freej, freej->screen->w, freej->screen->h);
}
bool XGrabLayer::init(Context *freej, int w, int h) {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	env = freej;
	_init(w, h);
	return true;
}
void *XGrabLayer::feed() {
	//func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	//return surf->pixels;
//XImage *XGetImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format);
	if (ximage)
		XDestroyImage(ximage);
	if (win) {
		ximage = XGetImage(display, win, 0, 0, geo.w, geo.h, AllPlanes, ZPixmap);
		return ximage->data;
	}
	return NULL;

//XImage *XGetSubImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format, XImage *dest_image, int dest_x, dest_y);

	//(running ? NULL : (void *)0x1 );
}

//SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);

bool XGrabLayer::keypress(int key) {
	bool res = true;
	return res;
}

void XGrabLayer::close() {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	opened = false;
	active = false;
	stop();
	buffer = NULL;

	if (ximage) {
		XDestroyImage(ximage); // also frees *data
		ximage = NULL;
		//XFree(xvimage); // does not free pixbuffer
	}
//	if (gc) {
//		XFreeGC(display, gc);
//		gc = NULL;
//	}
	if (win) {
		func("resetting win_attr");
		if (!XChangeWindowAttributes(display, win, CWBackingStore|CWSaveUnder, &win_sattr)) {
			error("resetting win_attributes failed");
		}
		XSync(display, false);
	}
}

DECLARE_CLASS_GC("XGrabLayer",js_xgrab_class,js_xgrab_constructor,js_layer_gc);
JS_CONSTRUCTOR("XGrabLayer",js_xgrab_constructor,XGrabLayer);

JSFunctionSpec js_xgrab_methods[] = {
	ENTRY_METHODS  ,
	{"open",      js_xgrab_open,  1},
	{"close",     js_xgrab_close, 1},
	{0}
};
#if 0
//JS_CONSTRUCTOR("ViewPort",js_xgrab_constructor,XGrabLayer);
JS(js_xgrab_constructor) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

	XGrabLayer *xg = new XGrabLayer();

	// initialize with javascript context
	if(! xg->init(cx, obj) ) {
		error("failed initializing xgrab");
		delete xg; return JS_FALSE;
	}
	if (argc == 1) {
		JS_ARG_NUMBER(winid, 0);
		if(!JS_NewNumberValue(cx, xg->open((uint32_t)winid), rval)) {
			error("failed initializing xgrab controller");
			delete xg; return JS_FALSE;
		}
	}
	// assign instance into javascript object
	if( ! JS_SetPrivate(cx, obj, (void*)xg) ) {
		error("failed assigning xgrab controller to javascript");
		delete xg; return JS_FALSE;
	}
	//*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
#endif
JS(js_xgrab_open) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	GET_LAYER(XGrabLayer);

	if (argc == 0) {
		return JS_NewNumberValue(cx, lay->open(), rval);
	}

	if (argc == 1) {
		JS_ARG_NUMBER(winid, 0);
		return JS_NewNumberValue(cx, lay->open((uint32_t)winid), rval);
	}
	JS_ERROR("Wrong number of arguments");
  
}
JS(js_xgrab_close) {
	GET_LAYER(XGrabLayer);
	lay->close();	
	return JS_TRUE;
}

