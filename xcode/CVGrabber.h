/*
 *  CVideo_layer.h
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */
#ifndef __CVGRABBERCONTROLLER_H__
#define __CVGRABBERCONTROLLER_H__

#include "CVLayer.h"
#import "CVLayerController.h"

#include <context.h>
#import <QTKit/QTKit.h>
#import <CoreAudio/CoreAudio.h>

#include "CVFilterPanel.h"

@class CFreej;
@class CVGrabber;

@interface CVGrabberController : CVLayerController
{
    CVPixelBufferRef exportedFrame;
}

- (void)feedFrame:(CVPixelBufferRef)frame;
@end


/*
 * QTKit Bridge
 */

@interface CVGrabber : QTCaptureDecompressedVideoOutput
{
    NSRecursiveLock              *lock;
    CVImageBufferRef             currentFrame;
    CVImageBufferRef             lastFrame;
    CIImage                      *renderedImage;
    CIImage                      *outputImage;

    time_t                       currentPts;
    time_t                       previousPts;
    IBOutlet CVGrabberController *grabberController;
    IBOutlet NSPopUpButton       *captureSize;
    IBOutlet CFreej              *freej;
    QTCaptureDeviceInput         * input;
    QTCaptureMovieFileOutput     *captureOutput;
    QTCaptureSession             *session;
    QTCaptureDevice              *device;
    int                          width;
    int                          height;
    bool                         running;
    CVLayer                      *layer;
}

- (id)init;
- (IBAction)startCapture:(id)sender;
- (IBAction)stopCapture:(id)sender;
- (IBAction)toggleCapture:(id)sender;
@end
 
#endif

