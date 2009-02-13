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

@interface CVideoOutput : QTCaptureDecompressedVideoOutput
{
    CVImageBufferRef currentImageBuffer;
    time_t currentPts;
    time_t previousPts;
}
- (id)init;
- (void)outputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection;
- (time_t)copyCurrentFrameToBuffer:(void **)buffer size:(int *)bufsize;
@end

class CVideoGrabber: public Layer {
	private:
		QTCaptureSession * session;
		QTCaptureDevice * device;
		CVideoOutput * output;
		int height, width;
		Context *freej;
		void *vbuffer;
		int bufsize;
	public:
		CVideoGrabber();
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

