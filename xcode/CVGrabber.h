/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
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

