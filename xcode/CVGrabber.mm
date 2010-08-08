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
        currentPts = 0;
        previousPts = 0;
        width = 352;
        height = 288;
    }
    return self;
}

- (void)dealloc
{
    [self stopCapture];

    [super dealloc];
}


- (void)startCapture:(CVGrabberController *)controller
{
    notice( "QTCapture opened" );
    bool ret = false;
    
    NSError *o_returnedError;
    width = [controller width];
    height = [controller height];
    /* Hack - using max resolution seems to lower cpu consuption for some reason */
    int h = (height < CV_GRABBER_HEIGHT_MAX)
          ? height
          : CV_GRABBER_HEIGHT_MAX;
    int w = (width < CV_GRABBER_WIDTH_MAX)
          ? width
          : CV_GRABBER_WIDTH_MAX;
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

    ret = [session addInput:input error: &o_returnedError];
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

    [self setDelegate:controller];
    return;
error:
    //[= exitQTKitOnThread];
    [input release];

}

- (void)stopCapture
{
    [lock lock];
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


@end

/* This 'glue' class is necessary only because the CVGrabber needs to be a subclass
 * QTCaptureDecompressedVideoOutput. 
 */ 

@implementation CVGrabberController : CVLayerController

- (id)init
{
	grabber = [[CVGrabber alloc] init];
	return [super init];
}

- (void)dealloc
{
	if (grabber)
		[grabber release];
	[super dealloc];
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput 
  didOutputVideoFrame:(CVImageBufferRef)videoFrame 
	 withSampleBuffer:(QTSampleBuffer *)sampleBuffer 
	   fromConnection:(QTCaptureConnection *)connection
{
	[self feedFrame:videoFrame];
}

- (void)start
{
	[super start];
	[grabber startCapture:self];
}

- (void)stop
{
	[super stop];
	[grabber stopCapture];
}

@end

