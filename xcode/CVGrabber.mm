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

#import <CVGrabber.h>
#import <CFreej.h>
#import <CIAlphaFade.h>

#include <jutils.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#define CV_GRABBER_WIDTH_MAX 640
#define CV_GRABBER_HEIGHT_MAX 480

/* Apple sample code */
@implementation CVGrabber : QTCaptureDecompressedVideoOutput

- (id)init
{
    if( self = [super init] ) {
        currentFrame = nil;
        currentPts = 0;
        previousPts = 0;
        width = 352;
        height = 288;
        lock = [[NSRecursiveLock alloc] init];
    }
    return self;
}

- (void)dealloc
{
    [self stopCapture:self];
    CVBufferRelease(currentFrame);

    [lock release];
    [super dealloc];
}

- (void)outputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    // Store the latest frame
    // This must be done in a @synchronized block because this delegate method is not called on the main thread
    CVImageBufferRef imageBufferToRelease;
    CVBufferRetain(videoFrame);

    imageBufferToRelease = currentFrame;
    currentFrame = videoFrame;
    currentPts = (time_t)(1000000L / [sampleBuffer presentationTime].timeScale * [sampleBuffer presentationTime].timeValue);
    
    /* Try to use hosttime of the sample if available, because iSight Pts seems broken */
    //NSNumber *hosttime = (NSNumber *)[sampleBuffer attributeForKey:QTSampleBufferHostTimeAttribute];
    //if( hosttime ) currentPts = (time_t)AudioConvertHostTimeToNanos([hosttime unsignedLongLongValue])/1000;
    
    [grabberController feedFrame:currentFrame];

    if (imageBufferToRelease)
        CVBufferRelease(imageBufferToRelease);

}


- (IBAction)startCapture:(id)sender
{
    notice( "QTCapture opened" );

    NSError *o_returnedError;
    Context *ctx = [freej getContext];
    width = ctx->screen->geo.w;
    height = ctx->screen->geo.h;
    /* Hack - using ma resolution seems to lower cpu consuption for some reason */
    int h = (height < CV_GRABBER_HEIGHT_MAX)?height:CV_GRABBER_HEIGHT_MAX;
    int w = (width < CV_GRABBER_WIDTH_MAX)?width:CV_GRABBER_WIDTH_MAX;
 
    device = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeVideo];
    if( !device )
    {
        error ( "Can't find any Video device" );
        goto error;
    }
    [device retain];

    if( ![device open: &o_returnedError] )
    {
        error( "Unable to open the capture device (%i)", [o_returnedError code] );
        goto error;
    }

    if( [device isInUseByAnotherApplication] == YES )
    {
        error( "default capture device is exclusively in use by another application" );
        goto error;
    }

    input = [[QTCaptureDeviceInput alloc] initWithDevice: device];
    if( !input )
    {
        error( "can't create a valid capture input facility" );
        goto error;
    }
    
    [self setPixelBufferAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:h], kCVPixelBufferHeightKey,
        [NSNumber numberWithInt:w], kCVPixelBufferWidthKey, 
        [NSNumber numberWithInt:kCVPixelFormatType_32ARGB],
        (id)kCVPixelBufferPixelFormatTypeKey, nil]];

    session = [[QTCaptureSession alloc] init];

    bool ret = [session addInput:input error: &o_returnedError];
    if( !ret )
    {
        error( "default video capture device could not be added to capture session (%i)", [o_returnedError code] );
        goto error;
    }

    ret = [session addOutput:self error: &o_returnedError];
    if( !ret )
    {
        error ( "output could not be added to capture session (%i)", [o_returnedError code] );
        goto error;
    }

    [session startRunning]; // start the capture session
    notice( "Video device ready!" );

    running = true;
    [self setDelegate:grabberController];
    [grabberController start];
    return;
error:
    //[= exitQTKitOnThread];
    [input release];

}

- (IBAction)stopCapture:(id)sender
{
    [lock lock];
    running = false;
    [grabberController stop];
    if (session) {
        [session stopRunning];
        if (input) {
            [session removeInput:input];
            [input release];
            input = NULL;
        }
        [session removeOutput:self];
        [session release];
    }
    /*
    if (output) {
        [output release];
        output = NULL;
    }
    */
    if (device) {
        if ([device isOpen])
            [device close];
        [device release];
        device = NULL;
    }

    [lock unlock];
}

- (IBAction)toggleCapture:(id)sender
{
    if (running)
        [self stopCapture:self];
    else
        [self startCapture:self];
}

@end

/* This 'glue' class is necessary only because the CVGrabber needs to be a subclass
 * QTCaptureDecompressedVideoOutput. 
 */ 

@implementation CVGrabberController : CVLayerController

- (id)init
{
    
    exportedFrame = nil;
    currentFrame = nil;
    NSLog(@"MADONNA!!! \n");

    return [super init];
}

- (void)dealloc
{
    if (currentFrame)
        CVPixelBufferRelease(currentFrame);
    [super dealloc];
}

- (CVReturn)renderFrame
{
    [lock lock];
    [self renderPreview];
    if (layer)
        layer->vbuffer = currentFrame;
    [lock unlock];
    return kCVReturnSuccess;
}

- (void)feedFrame:(CVPixelBufferRef)frame
{
    CVReturn error;
    [lock lock];
    u_char *sourceAddr, *destAddr;
    if (!currentFrame) {
        error = CVPixelBufferCreate (
                    NULL,
                    CVPixelBufferGetWidth(frame),
                    CVPixelBufferGetHeight(frame),
                    CVPixelBufferGetPixelFormatType(frame),
           NULL,
           &currentFrame
        );
        if (error != kCVReturnSuccess) {
            NSLog(@"Can't create pixelbuffer to store currentFrame (err=%i)", error);
            [lock unlock];
            return;
        }
    }
    // copy the pixelbuffer coming from the capture device to avoid holding it for 
    // too much time (causing the system to hang)
    if(CVPixelBufferLockBaseAddress(frame, 0) == kCVReturnSuccess) {
        CVPixelBufferLockBaseAddress(currentFrame, 0);

        sourceAddr = (u_char *)CVPixelBufferGetBaseAddress(frame);
        destAddr = (u_char *)CVPixelBufferGetBaseAddress(currentFrame);
        memcpy(destAddr, sourceAddr, CVPixelBufferGetBytesPerRow(frame)*CVPixelBufferGetHeight(frame));
        
        CVPixelBufferUnlockBaseAddress(currentFrame, 0);
        newFrame = YES;
    } else {
        NSLog(@"CVPixelBufferLockBaseAddress() failed with error %i", error);
    }
   [lock unlock];
}

@end

