/*
 *  CVideo_layer.h
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __CVideo_layer_H__
#define __CVideo_layer_H__

#include <context.h>
#import <QTKit/QTKit.h>
#import <CoreAudio/CoreAudio.h>

/*****************************************************************************
* QTKit Bridge
*****************************************************************************/
@class CFreej;

@interface CVideoOutput : QTCaptureDecompressedVideoOutput
{
    CVImageBufferRef currentImageBuffer;
    time_t currentPts;
    time_t previousPts;
	IBOutlet QTCaptureView *captureView;
	IBOutlet NSPopUpButton *captureSize;
	IBOutlet CFreej *controller;
	QTCaptureDeviceInput * input;
	QTCaptureMovieFileOutput *captureOutput;
	QTCaptureSession * session;
	QTCaptureDevice * device;
	int width;
	int height;
	bool running;
}

- (id)init;
- (void)awakeFromNib;
- (time_t)copyCurrentFrameToBuffer:(void **)buffer size:(int *)bufsize;
- (IBAction)startCapture:(id)sender;
- (IBAction)stopCapture:(id)sender;
- (IBAction)toggleCapture:(id)sender;
- (IBAction)setSize:(id)sender;
@end

class CVideoGrabber: public Layer {
	private:
		int height, width;
		Context *freej;
		void *vbuffer;
		int bufsize;
		CVideoOutput * output;
	public:
		CVideoGrabber(CVideoOutput *vout);
		~CVideoGrabber();
		bool open(const char *dev);
		bool init(Context *freej);
		bool CVideoGrabber::init(Context *ctx, int w, int h);
		void *feed();
		void close();
		
		bool forward();
		bool backward();
		bool backward_one_keyframe();
		
		bool relative_seek(double increment);
		
		bool set_mark_in();
		bool set_mark_out();
		
		void pause();
		
};

#endif

