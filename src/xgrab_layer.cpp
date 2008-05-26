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
	autosize = true;
	//gc = NULL;
	
	set_name("XGR");
	jsclass = &js_xgrab_class;
	int r = XInitThreads();
	func("XinitThread: %i", r);
}

XGrabLayer::~XGrabLayer() {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	close();
}

int bad_window_handler(Display *disp, XErrorEvent *err) {
	error("No such window (0x%lx)", err->resourceid);
	return 0; //the returned value is ignored
}

bool XGrabLayer::open(const char *file) {
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

//int bad_window_handler(Display *, XErrorEvent *);

bool XGrabLayer::open(uint32_t win_id_new) {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	if (opened)
		return 0;

	char errmsg[MAX_ERR_MSG];
	XErrorHandler old_h = XSetErrorHandler(bad_window_handler);
// check win id ok
//set_filename(display_name); <- win title ...

	if ( (display=XOpenDisplay(NULL)) == NULL ) {
		snprintf(errmsg, MAX_ERR_MSG,
			"Can't connect to X server");
		goto fail;
	}
	screen_num = DefaultScreen(display);

	//screen_num = DefaultScreen(display);
/*
 * root win:
 * w=RootWindow(dpy, screen);
 *
 * find parent win with WM deco:
	  if (window && !frame) {
	      Window root;
	      int dummyi;
	      unsigned int dummy;

	      if (XGetGeometry (dpy, window, &root, &dummyi, &dummyi,
				&dummy, &dummy, &dummy, &dummy) &&
		  window != root)
	        window = XmuClientWindow (dpy, window);
	  }
*/
	if (!XGetWindowAttributes(display, win_id_new, &wa)) {
		snprintf(errmsg, MAX_ERR_MSG,
			"Can't get win attributes");
		goto fail;
	}
	mapped = wa.map_state;
//wa.class = InputOutput, InputOnly
	func("xwin depth:%u ", wa.depth);
{
	int res = XSelectInput(display, win_id_new,
		StructureNotifyMask| 	// ConfigureNotify,DestroyNotify,(un)MapNotify
		VisibilityChangeMask|	// VisibilityNotify
		PointerMotionMask|		// MotionNotify
		ExposureMask			// (No)Expose, GraphicsExpose
	);
	func("xsel input: %i", res);
}
	XSync (display, False);
	//XSetErrorHandler(old_h);

	lock();
	win = win_id_new;
	resize();
	unlock();

	opened = true;
	active = true;
	return true;

	fail:
		error(errmsg); // TODO: JS exception
		close();
		return false;
}
void XGrabLayer::resize() {
	Window junkwin;
	int rx, ry, xright, ybelow;
	int dw = DisplayWidth (display, screen_num);
	int dh = DisplayHeight(display, screen_num);

	(void) XTranslateCoordinates (display, win, wa.root,
		-wa.border_width,
		-wa.border_width,
		&rx, &ry, &junkwin);

	xright = (dw - rx - wa.border_width * 2 - wa.width);
	ybelow = (dh - ry - wa.border_width * 2 - wa.height);

	uint32_t wn, hn; // new width, height
	wn = wa.width - (rx<0 ? -rx : 0) - (xright<0 ? -xright :0) - crop.x;
//if (crop.w > 0)
	hn = wa.height- (ry<0 ? -ry : 0) - (ybelow<0 ? -ybelow :0) - crop.y;
	crop.x = (rx<0 ? -rx : 0);
	crop.y = (ry<0 ? -ry : 0);

	//lock();
	geo.w = (wn > 0 ? wn : 0);
	geo.h = (hn > 0 ? hn : 0);
	geo.bpp = 32;
	geo.size = geo.w*geo.h*(geo.bpp/8);
	geo.pitch = geo.w*(geo.bpp/8);
	//unlock();
}
#if 0
resize:
ConfigureNotify event, serial 19, synthetic NO, window 0x5a00003,
    event 0x5a00003, window 0x5a0002a, (0,27), width 636, height 32,
    border_width 0, above 0x5a00021, override NO

VisibilityNotify event, serial 19, synthetic NO, window 0x5a00003,
    state VisibilityUnobscured

UnmapNotify event, serial 19, synthetic NO, window 0x5a00003,
    event 0x5a00003, window 0x5a00003, from_configure NO

DestroyNotify event, serial 26, synthetic NO, window 0x5c00007,
    event 0x5c00007, window 0x5c0000b

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
	return init(freej, 0, 0);
	//return init(freej, freej->screen->w, freej->screen->h);
}
bool XGrabLayer::init(Context *freej, int w, int h) {
	func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	env = freej;
	autosize = false;
	_init(w, h);
	crop.x=0;crop.y=0;crop.w=w;crop.h=h;
	return true;
}
//XImage *XGetImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format);
//XImage *XGetSubImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format, XImage *dest_image, int dest_x, dest_y);

void *XGrabLayer::feed() {
	//func("%u:%s:%s (%p)",__LINE__,__FILE__,__FUNCTION__, this);
	//return surf->pixels;
	//
	if (!win) // deactivate? / void pthread_exit(void *retval); // who join()?
		return NULL;

	XEvent event;
	while(XCheckMaskEvent(display, ~0x0, &event)) {
		switch (event.type) {
			case VisibilityNotify:
				func("vn");
				break;
			case DestroyNotify:
				func("dn");
				break;
			case MapNotify:
				func("mn");
				mapped = true;
				break;
			case UnmapNotify:
				func("un");
				mapped = false;
				break;
			case ConfigureNotify:
				func("cn");
				break;
			case PropertyNotify:
				func("pn");
				break;
			default:
				func("unh event: %i w:0x%x", event.type, ((XAnyEvent*)&event)->window);
		}
	}
	void* ret = NULL;
	XLockDisplay(display);
	if (!XGetWindowAttributes(display, win, &wa)) {
		error("%s", "Can't get win attributes");
		goto exit;
	}
	if (wa.map_state != IsViewable) { // IsUnmapped, IsUnviewable, IsViewable
		func("unmapped");
		ret = (ximage ? ximage->data : NULL);
		goto exit;
	}
//Bool XCheckMaskEvent(Display *display, long event_mask, XEvent
//*event_return);
//
//Bool XCheckTypedEvent(Display *display, int event_type, XEvent
//*event_return);
	resize();
	if (ximage)
		XDestroyImage(ximage);
	if (win) {
		//ximage = XGetImage(display, win, 0, 0, geo.w, geo.h, AllPlanes, ZPixmap);
		ximage = XGetImage(display, win, crop.x, crop.y, geo.w, geo.h, AllPlanes, ZPixmap);
		if (ximage)
			ret = ximage->data;
	}

exit:
	XUnlockDisplay(display);
	return ret;
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
//	if (win) {
//	}
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

