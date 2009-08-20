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
#import "QTExporter.h"

@class CFreej;
class  CVScreen;

@interface CVScreenView : NSOpenGLView {
	NSRecursiveLock		*lock;
	NSString			*fpsString;
    NSWindow            *myWindow;
    CVDisplayLinkRef	displayLink; // the displayLink that runs the show
    CGDirectDisplayID	viewDisplayID;
	CVPixelBufferRef	pixelBuffer;
    //void                *pixelBuffer;
	CVScreen			*fjScreen;
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
    CGContextRef        exportCGContextRef;
    CIContext           *exportContext;
    void                *exportBuffer;
    QTExporter          *exporter;
	IBOutlet CFreej		*freej;
	IBOutlet NSTextField *showFps;
    IBOutlet NSTableView *layerList;
    IBOutlet NSWindow   *window;
}
@property (readonly) bool fullScreen;
- (void)awakeFromNib;
- (id)init;
- (void)prepareOpenGL;
- (void *)getSurface;
- (CIImage *)exportSurface;
- (void)drawLayer:(Layer *)layer;
- (void)setSizeWidth:(int)w Height:(int)h;
- (bool)isOpaque;
- (double)rate;
- (CVReturn)outputFrame:(uint64_t)timestamp;
- (void)reset;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex;
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)toggleExport:(id)sender;
- (IBAction)startExport:(id)sender;
- (IBAction)stopExport:(id)sender;
- (IBAction)setExportFile:(id)sender;
@end
#else
class CVScreenView;
#endif // __cocoa

/*
 * C++ glue class exposed to the freej context
 */

class CVScreen : public ViewPort {
    private:
        CVScreenView *view;
        bool init(int w, int h);
    public:
        CVScreen(int w, int h);
        ~CVScreen();
        void set_view(CVScreenView *view);
        CVScreenView *get_view(void);
        void *get_surface();
        void *coords(int x, int y);
        void blit(Layer *);
        inline void setup_blits(Layer *lay) { };
        void show();
        bool add_layer(Layer *lay);
        void rem_layer(Layer *lay);
        fourcc get_pixel_format() { return ARGB32; };
        void CVScreen::resize(int w, int h);
};

#endif
