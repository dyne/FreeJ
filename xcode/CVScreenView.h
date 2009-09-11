//
//  CVScreenView.h
//  freej
//
//  Created by xant on 8/30/09.
//  Copyright 2009 dyne..org. All rights reserved.
//

#ifndef __CV_SCREENVIEW_H__
#define __CV_SCREENVIEW_H__

#import <CVScreen.h>
#import <CVTexture.h>

class CVScreen;
@class CFreej;

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
- (void)addLayer:(Layer *)lay;
- (void)remLayer:(Layer *)lay;
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView;
- (NSWindow *)getWindow;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)toggleExport:(id)sender;
- (IBAction)startExport:(id)sender;
- (IBAction)stopExport:(id)sender;
- (IBAction)setExportFile:(id)sender;
@end

#endif
