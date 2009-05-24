/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
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
 */

#ifndef __CV_SCREEN_H__
#define __CV_SCREEN_H__

#include <context.h>
#include <screen.h>
#ifdef __cocoa
#include <QuartzCore/QuartzCore.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/NSArray.h>
#import "CFreej.h"
#import "FrameRate.h"

@class CFreej;
class  CVScreen;

@interface CVScreenView : NSOpenGLView {
	IBOutlet CFreej		*freej;
	IBOutlet NSTextField *showFps;
	NSString			*fpsString;
    CVDisplayLinkRef	displayLink; // the displayLink that runs the show
    CGDirectDisplayID	viewDisplayID;
	CVPixelBufferRef	pixelBuffer;
    //void                *pixelBuffer;
	CVScreen			*fjScreen;
	NSRecursiveLock		*lock;
	CIContext			*ciContext;
	NSOpenGLContext		*currentContext;
	CIImage				*outFrame;
    CIImage             *exportedFrame;
    NSBitmapImageRep    *exportedFrameBuffer;  
    CIImage             *lastFrame;
	NSTimer				*renderTimer;
	bool				fullScreen;
	CFDictionaryRef		savedMode;
	bool				needsReshape;
	FrameRate			*rateCalc;
    NSMutableArray      *textures;
    CGContextRef        exportCGContextRef;
    CIContext           *exportContext;
    void                *exportBuffer;
}
@property (readonly) bool fullScreen;
- (void)awakeFromNib;
- (id)init;
- (void)prepareOpenGL;
- (void *)getSurface;
- (void)drawLayer:(Layer *)layer;
- (void)setSizeWidth:(int)w Height:(int)h;
- (IBAction)toggleFullScreen:(id)sender;
- (bool)isOpaque;
- (double)rate;
- (CVReturn)outputFrame:(uint64_t)timestamp;
- (void)reset;
@end
#else
class CVScreenView;
#endif // __cocoa
class CVScreen : public ViewPort {
 private:
	CVScreenView *view;
 public:
  CVScreen();
  ~CVScreen();

  bool init(int w, int h);
  void set_view(CVScreenView *view);
  CVScreenView *get_view(void);
  void *get_surface();
  void *coords(int x, int y);
  void blit(Layer *);
  void CVScreen::show();
  fourcc get_pixel_format() { return ARGB32; };
};

#endif
