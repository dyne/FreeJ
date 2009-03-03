//
//  CVideoFile.h
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//
#ifndef __CV_FILEINPUT_H__
#define __CV_FILEINPUT_H__

#include <context.h>
#import <Cocoa/Cocoa.h>
#import <QuickTime/QuickTime.h>
#import <QTKit/QTKit.h>
#import "CFreej.h";
#include "CVLayer.h"
#include "CVFilterPanel.h"

@interface CVideoFileInput : NSOpenGLView {
	NSRecursiveLock		*lock;
	id					delegate;
	QTMovie				*qtMovie; 
	QTTime				movieDuration;		    // cached duration of the movie - just for convenience
    QTVisualContextRef	qtVisualContext;	    // the context the movie is playing in
    CVImageBufferRef	currentFrame;		    // the current frame from the movie
	CVImageBufferRef    lastFrame;
	// this is a "fake" buffer exposed to freej core
	// XXX - freej actaully needs a buffer pointer but it's not going to use it.
	// The blitter does...but on OSX the blitter is implemented in the CVScreen class.
	// This means we could expose any address to freej and that would work anyway.
	CVOpenGLBufferRef	pixelBuffer;			 
	BOOL				newFrame;
	CIImage				*renderedImage;
	CIImage				*previewImage;
	CVFilterPanel       *filterPanel;
    
    // display link
    CVDisplayLinkRef	displayLink;		    // the displayLink that runs the show
    CGDirectDisplayID	viewDisplayID;

    // filters for CI rendering
    CIFilter				*colorCorrectionFilter;	    // hue saturation brightness control through one CI filter
    CIFilter				*compositeFilter;	    // composites the timecode over the video
	CIFilter				*alphaFilter;
	CIFilter				*exposureAdjustFilter;
	CIFilter				*rotateFilter;
	CIFilter				*translateFilter;
	CIFilter				*effectFilter;
	CIFilter				*scaleFilter;
	NSArray					*paramNames;
	
	//CIFilter				*resizeFilter;
    CIContext				*ciContext;
	CIContext				*cifjContext;
	
	bool					needsReshape;
	//CVImageBufferRef		freejFrame;
	CVLayer					*layer;
	IBOutlet CFreej			*freej;
	IBOutlet NSSlider		*alphaBar;
	bool					doFilters;
	NSTimer					*renderTimer;
}
@property (readwrite) CVLayer *layer;
- (void)awakeFromNib;
- (id)init;
- (bool)stepBackward;
- (bool)setpForward;
- (void)updateCurrentFrame;
- (void)renderCurrentFrame;
- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;
- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp;
- (void *)grabFrame;
- (IBAction)setMovieTime:(id)sender;
- (IBAction)togglePlay:(id)sender;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)setAlpha:(id)sender;
- (IBAction)setBlendMode:(id)sender;
- (IBAction)openFile:(id)sender;
- (IBAction)toggleFilters:(id)sender;
- (CIImage *)getTexture;
- (void)mouseDown:(NSEvent *)theEvent;
@end

#endif
