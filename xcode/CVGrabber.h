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
#include "CVFilterPanel.h"

@class CFreej;
@class CVGrabber;

/*****************************************************************************
* QTKit Bridge
*****************************************************************************/

@interface CVGrabberView : CVLayerView
{
    CVPixelBufferRef exportedFrame;
    IBOutlet CVGrabber *grabber;
}
- (void)feedFrame:(CVPixelBufferRef)frame;
- (void)start;
- (void)stop;
@end

@interface CVGrabber : QTCaptureDecompressedVideoOutput
{
    NSRecursiveLock             *lock;
    CVImageBufferRef            currentFrame;
    CVImageBufferRef            lastFrame;
    CIImage                     *renderedImage;
    CIImage                     *outputImage;

    time_t                      currentPts;
    time_t                      previousPts;
    IBOutlet CVGrabberView      *grabberView;
    IBOutlet NSPopUpButton      *captureSize;
    IBOutlet CFreej             *freej;
    QTCaptureDeviceInput        * input;
    QTCaptureMovieFileOutput    *captureOutput;
    QTCaptureSession            *session;
    QTCaptureDevice             *device;
    int                         width;
    int                         height;
    bool                        running;
    CVLayer                     *layer;
    CVFilterPanel               *filterPanel;
        // filters for CI rendering
    CIFilter                    *colorCorrectionFilter;        // hue saturation brightness control through one CI filter
    CIFilter                    *compositeFilter;        // composites the timecode over the video
    CIFilter                    *alphaFilter;
    CIFilter                    *exposureAdjustFilter;
    CIFilter                    *rotateFilter;
    CIFilter                    *translateFilter;
    CIFilter                    *effectFilter;
    CIFilter                    *scaleFilter;
    NSMutableArray              *paramNames;
}

- (id)init;
- (IBAction)startCapture:(id)sender;
- (IBAction)stopCapture:(id)sender;
- (IBAction)toggleCapture:(id)sender;
@end
 
#endif

