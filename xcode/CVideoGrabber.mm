/*
 *  CVideo_layer.cpp
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne. All rights reserved.
 *
 */

#include "CVideoGrabber.h"
#include "CFreej.h"
#include <jutils.h>

#define CV_GRABBER_HEIGHT_MAX 480
#define CV_GRABBER_WIDTH_MAX 640

/* Apple sample code */
@implementation CVideoOutput : QTCaptureDecompressedVideoOutput

- (id)init
{
    if( self = [super init] ) {
        currentImageBuffer = nil;
		freejImageBuffer = nil;
        currentPts = 0;
        previousPts = 0;
		width = 640;
		height = 480;
    }
    return self;
}
- (void)dealloc
{
    @synchronized (self)
    {
        CVBufferRelease(currentImageBuffer);
        currentImageBuffer = nil;
    }
    [super dealloc];
}

- (void)outputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    // Store the latest frame
    // This must be done in a @synchronized block because this delegate method is not called on the main thread
    CVImageBufferRef imageBufferToRelease;
	[QTMovie enterQTKitOnThread];
    CVBufferRetain(videoFrame);
    @synchronized (self)
    {
        imageBufferToRelease = currentImageBuffer;
        currentImageBuffer = videoFrame;
        currentPts = (time_t)(1000000L / [sampleBuffer presentationTime].timeScale * [sampleBuffer presentationTime].timeValue);
        
        /* Try to use hosttime of the sample if available, because iSight Pts seems broken */
        NSNumber *hosttime = (NSNumber *)[sampleBuffer attributeForKey:QTSampleBufferHostTimeAttribute];
        if( hosttime ) currentPts = (time_t)AudioConvertHostTimeToNanos([hosttime unsignedLongLongValue])/1000;
		if (layer) {
			layer->buffer = (void *)currentImageBuffer;
		}
    }
	[QTMovie exitQTKitOnThread];
    CVBufferRelease(imageBufferToRelease);
}

- (void)awakeFromNib
{
	[self setSize:self];
}

 
- (void)windowWillClose:(NSNotification *)notification
{
    //[mCaptureSession stopRunning];
    //[[mCaptureDeviceInput device] close];
 
}

- (IBAction)setSize:(id)sender
{
	const char *selected = [[captureSize titleOfSelectedItem] UTF8String];
	if (sscanf(selected, "%dx%d", &width, &height) != 2)
		error("Bad capture size selected! : %s", selected);
	if (running) {
		// restart capture with newer size
		[self stopCapture:self];
		[self startCapture:self];
	}
}

- (IBAction)startCapture:(id)sender
{
	notice( "QTCapture opened" );

    NSError *o_returnedError;
	[QTMovie enterQTKitOnThread];
    device = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeVideo];
    if( !device )
    {
       // intf_UserFatal( p_demux, true, _("No Input device found"),
         //               _("Your Mac does not seem to be equipped with a suitable input device. "
           //               "Please check your connectors and drivers.") );
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
	
	Context *ctx = [freej getContext];
	int height = (ctx->screen->h < CV_GRABBER_HEIGHT_MAX)?ctx->screen->h:CV_GRABBER_HEIGHT_MAX;
	int width = (ctx->screen->w < CV_GRABBER_WIDTH_MAX)?ctx->screen->w:CV_GRABBER_WIDTH_MAX;
    /* Hack - This will lower CPU consumption for some reason */
    [self setPixelBufferAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:height], kCVPixelBufferHeightKey,
        [NSNumber numberWithInt:width], kCVPixelBufferWidthKey, 
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

	[captureView setCaptureSession:session];
    [session startRunning]; // start the capture session
    notice( "Video device ready!" );

	running = true;
	[QTMovie exitQTKitOnThread];
	layer = new CVLayer((NSObject *)self);
	layer->init(ctx);
	layer->activate();
	return;
error:
	[QTMovie exitQTKitOnThread];
	[input release];

}

- (IBAction)stopCapture:(id)sender
{
	@synchronized (self) {
		[QTMovie enterQTKitOnThread];
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

		freejImageBuffer = NULL;
		[QTMovie exitQTKitOnThread];
		if (layer) {
			Context *ctx = [freej getContext];
			ctx->rem_layer(layer);
			delete(layer);
			layer = NULL;
		}
		running = false;
	}
}

- (IBAction)toggleCapture:(id)sender
{
	if (running)
		[self stopCapture:self];
	else
		[self startCapture:self];
}

- (void *)grabFrame
{
	return (void *)[self getTexture];
}

- (CIImage *)getTexture
{
    time_t pts;
	//freejImageBuffer = NULL;
	
	if(!currentImageBuffer || currentPts == previousPts )
        return freejImageBuffer;

    @synchronized (self)
    {
        pts = previousPts = currentPts;

		if (freejImageBuffer) {
			[freejImageBuffer release];
		}

		freejImageBuffer = [CIImage imageWithCVImageBuffer:currentImageBuffer];
		[freejImageBuffer retain];
    }

    return freejImageBuffer;
}

@synthesize layer;
@end


