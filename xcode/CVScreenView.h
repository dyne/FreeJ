/*  FreeJ
 *  (c) Copyright 2009 Andrea Guzzo <xant@dyne.org>
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


#ifndef __CV_SCREENVIEW_H__
#define __CV_SCREENVIEW_H__


#import <CVScreen.h>
#import <CVTexture.h>
#import <Carbon/Carbon.h>

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
    QTStreamer          *streamer;
    bool                streamerStatus;
    bool                initialized;
	IBOutlet CFreej		*freej;
	IBOutlet NSTextField *showFps;
    IBOutlet NSTableView *layerList;
    IBOutlet NSWindow   *window;
    IBOutlet NSTableView *streamerSettings;
    IBOutlet NSButton    *streamerButton;
    IBOutlet NSTextField *streamerFPS;
    IBOutlet NSTextField *streamerPkg;
    NSMutableArray *streamerKeys;
    NSMutableDictionary *streamerDict;
}
@property (readonly) bool fullScreen;
- (void)awakeFromNib;
- (id)init;
- (void)prepareOpenGL;
- (void *)getSurface;
- (CIImage *)exportSurface;
- (CVPixelBufferRef)exportPixelBuffer;
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
- (CVDisplayLinkRef)getDisplayLink;
- (IBAction)toggleFullScreen:(id)sender;
- (IBAction)toggleExport:(id)sender;
- (IBAction)startExport:(id)sender;
- (IBAction)stopExport:(id)sender;
- (IBAction)setExportFile:(id)sender;
- (IBAction)toggleStreamer:(id)sender;
@end

typedef struct _fmmetadata {
  int timeout;
  char *streamurl1, *streamdel1; 
  char *streamurl2, *streamdel2; 
} FlowMixerMetaData;

#endif
