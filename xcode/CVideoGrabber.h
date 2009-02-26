/*
 *  CVideo_layer.h
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 __MyCompanydyne.orgName__. All rights reserved.
 *
 */
#ifndef __CVideo_layer_H__
#define __CVideo_layer_H__

#define __cocoa
#include <context.h>
#import <QTKit/QTKit.h>
#import <CoreAudio/CoreAudio.h>

#include "CVLayer.h"

@class CFreej;

/*****************************************************************************
* QTKit Bridge
*****************************************************************************/

class CVideoGrabber;

@interface CVideoOutput : QTCaptureDecompressedVideoOutput
{
	NSRecursiveLock		*lock;
    CVImageBufferRef currentImageBuffer;
	CIImage *freejImageBuffer;
    time_t currentPts;
    time_t previousPts;
	IBOutlet QTCaptureView *captureView;
	IBOutlet NSPopUpButton *captureSize;
	IBOutlet CFreej *freej;
	QTCaptureDeviceInput * input;
	QTCaptureMovieFileOutput *captureOutput;
	QTCaptureSession * session;
	QTCaptureDevice * device;
	int width;
	int height;
	bool running;
	CVLayer *layer;
}
@property CVLayer *layer;
- (id)init;
- (void)awakeFromNib;
- (void *)grabFrame;
- (IBAction)startCapture:(id)sender;
- (IBAction)stopCapture:(id)sender;
- (IBAction)toggleCapture:(id)sender;
- (IBAction)setSize:(id)sender;
- (CVImageBufferRef)getTexture;
@end

class CVideoGrabber: public Layer {
	private:
		CVideoOutput * output;
	public:

};

#endif

