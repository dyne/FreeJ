/*
 *  CVideo_layer.cpp
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne. All rights reserved.
 *
 */

#include "CVideoGrabber.h"
#include <jutils.h>

CVideoGrabber::CVideoGrabber() : Layer()
{
	vbuffer = NULL;
	bufsize = 0;
}

CVideoGrabber::~CVideoGrabber()
{
	close();
	if (vbuffer)
		free(vbuffer);
}

bool
CVideoGrabber::open(const char *dev)
{
    notice( "QTCapture opened" );

    NSError *o_returnedError;

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

    output = [[CVideoOutput alloc] init];
	

    /* Hack - This will lower CPU consumption for some reason */
    [output setPixelBufferAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
        [NSNumber numberWithInt:height], kCVPixelBufferHeightKey,
        [NSNumber numberWithInt:width], kCVPixelBufferWidthKey, 
		[NSNumber numberWithInt:kCVPixelFormatType_32BGRA],
		(id)kCVPixelBufferPixelFormatTypeKey, nil]];

    session = [[QTCaptureSession alloc] init];

    bool ret = [session addInput:input error: &o_returnedError];
    if( !ret )
    {
        error( "default video capture device could not be added to capture session (%i)", [o_returnedError code] );
        goto error;
    }

    ret = [session addOutput:output error: &o_returnedError];
    if( !ret )
    {
        error ( "output could not be added to capture session (%i)", [o_returnedError code] );
        goto error;
    }

    [session startRunning]; // start the capture session
    notice( "Video device ready!" );

    return true;
error:
	[input release];


    return false;

}

bool
CVideoGrabber::init(Context *ctx)
{
	 // * TODO - probe resolution of the default input device
	return init(freej, 640, 480);
}

bool
CVideoGrabber::init(Context *ctx, int w, int h)
{
	width = w;
	height = h;
	freej = ctx;
	_init(width,height);
	return true;
}

void *
CVideoGrabber::feed()
{
	time_t pts = 0;
	
    @synchronized (output)
    {
		pts = [output copyCurrentFrameToBuffer: &vbuffer size:&bufsize];
    }
	
	/* Nothing to display yet, just forget */
    if( !pts ) 
        return NULL;
   
    return vbuffer;
}

void
CVideoGrabber::close()
{

    /* Hack: if libvlc was killed, main interface thread was,
     * and poor QTKit needs it, so don't tell him.
     * Else we dead lock. */
	if (session) {
		[session stopRunning];
		if (input) {
			[session removeInput:input];
			[input release];
			input = NULL;
		}
		[session release];
	}
	
	if (output) {
		[output release];
		output = NULL;
	}
	
	if (device) {
		if ([device isOpen])
			[device close];
		[device release];
		device = NULL;
	}
	
}

bool
CVideoGrabber::forward()
{
	return false;
}

bool
CVideoGrabber::backward()
{
	return false;
}

bool
CVideoGrabber::backward_one_keyframe()
{
	return false;
}

bool
CVideoGrabber::set_mark_in()
{
	return false;
}

bool
CVideoGrabber::set_mark_out()
{
	return false;
}

void
CVideoGrabber::pause()
{
}

bool
CVideoGrabber::relative_seek(double increment)
{
	return false;
}


/* Apple sample code */
@implementation CVideoOutput : QTCaptureDecompressedVideoOutput
- (id)init
{
    if( self = [super init] ) {
        currentImageBuffer = nil;
        currentPts = 0;
        previousPts = 0;
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

    CVBufferRetain(videoFrame);

    @synchronized (self)
    {
        imageBufferToRelease = currentImageBuffer;
        currentImageBuffer = videoFrame;
        currentPts = (time_t)(1000000L / [sampleBuffer presentationTime].timeScale * [sampleBuffer presentationTime].timeValue);
        
        /* Try to use hosttime of the sample if available, because iSight Pts seems broken */
        NSNumber *hosttime = (NSNumber *)[sampleBuffer attributeForKey:QTSampleBufferHostTimeAttribute];
        if( hosttime ) currentPts = (time_t)AudioConvertHostTimeToNanos([hosttime unsignedLongLongValue])/1000;
    }
    CVBufferRelease(imageBufferToRelease);
}

- (time_t)copyCurrentFrameToBuffer:(void **)buffer size:(int *)bufsize
{
    CVImageBufferRef imageBuffer;
    time_t pts;
	OSType pixelFormat = 0;

    if(!currentImageBuffer || currentPts == previousPts )
        return 0;

    @synchronized (self)
    {
        imageBuffer = CVBufferRetain(currentImageBuffer);
        pts = previousPts = currentPts;

        CVPixelBufferLockBaseAddress(imageBuffer, 0);
        void * pixels = CVPixelBufferGetBaseAddress(imageBuffer);
		pixelFormat=CVPixelBufferGetPixelFormatType(imageBuffer);
		int size = CVPixelBufferGetBytesPerRow(imageBuffer) * CVPixelBufferGetHeight(imageBuffer);
		
		if (*buffer) {
			if (*bufsize < size) {
				*bufsize = size;
				*buffer = realloc(buffer, size);
			}
		} else {
			*bufsize = size;
			*buffer = malloc(size);
		}
        memcpy( *buffer, pixels, size );
        CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    }

    CVBufferRelease(imageBuffer);

    return currentPts;
}

@end


