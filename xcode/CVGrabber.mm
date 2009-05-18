/*
 *  CVideo_layer.cpp
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne. All rights reserved.
 *
 */

#import "CVGrabber.h"
#import "CFreej.h"
#import <CIAlphaFade.h>
#include <jutils.h>

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
        width = 320; // XXX - make defult size configurable
        height = 240; // XXX -^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
        lock = [[NSRecursiveLock alloc] init];
        // Create CIFilters used for both preview and main frame
        colorCorrectionFilter = [[CIFilter filterWithName:@"CIColorControls"] retain];        // Color filter    
        [colorCorrectionFilter setDefaults]; // set the filter to its default values
        exposureAdjustFilter = [[CIFilter filterWithName:@"CIExposureAdjust"] retain];
        [exposureAdjustFilter setDefaults];
        // adjust exposure
        [exposureAdjustFilter setValue:[NSNumber numberWithFloat:0.0] forKey:@"inputEV"];
        
        // rotate
        NSAffineTransform *rotateTransform = [NSAffineTransform transform];
        [rotateTransform rotateByDegrees:0.0];
        rotateFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        [rotateFilter setValue:rotateTransform forKey:@"inputTransform"];
        //translateFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        //[translateFilter setValue:translateTransform forKey:@"inputTransform"];
        scaleFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        //CIFilter *scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
        [scaleFilter setDefaults];    // set the filter to its default values
        //[scaleFilter setValue:[NSNumber numberWithFloat:scaleFactor] forKey:@"inputScale"];
        
        effectFilter = [[CIFilter filterWithName:@"CIZoomBlur"] retain];            // Effect filter    
        [effectFilter setDefaults];                                // set the filter to its default values
        [effectFilter setValue:[NSNumber numberWithFloat:0.0] forKey:@"inputAmount"]; // don't apply effects at startup
        compositeFilter = [[CIFilter filterWithName:@"CISourceOverCompositing"] retain];    // Composite filter
        [CIAlphaFade class];    
        alphaFilter = [[CIFilter filterWithName:@"CIAlphaFade"] retain]; // AlphaFade filter
        [alphaFilter setDefaults]; // XXX - setDefaults doesn't work properly
        [alphaFilter setValue:[NSNumber numberWithFloat:0.5] forKey:@"outputOpacity"]; // set default value
        paramNames = [[NSMutableArray arrayWithCapacity:4] retain];
        renderedImage = nil;
        outputImage = nil;
    }
    return self;
}
- (void)dealloc
{
    [self stopCapture:self];
    CVBufferRelease(currentFrame);
    currentFrame = nil;
    [colorCorrectionFilter release];
    [effectFilter release];
    [compositeFilter release];
    [alphaFilter release];
    [exposureAdjustFilter release];
    [rotateFilter release];
    [scaleFilter release];
    [paramNames release];
    if (renderedImage)
        [renderedImage release];
    [lock release];
    [super dealloc];
}

- (void)outputVideoFrame:(CVImageBufferRef)videoFrame withSampleBuffer:(QTSampleBuffer *)sampleBuffer fromConnection:(QTCaptureConnection *)connection
{
    // Store the latest frame
    // This must be done in a @synchronized block because this delegate method is not called on the main thread
    CVImageBufferRef imageBufferToRelease;
    CVBufferRetain(videoFrame);
    [lock lock];

    imageBufferToRelease = currentFrame;
    currentFrame = videoFrame;
    currentPts = (time_t)(1000000L / [sampleBuffer presentationTime].timeScale * [sampleBuffer presentationTime].timeValue);
    
    /* Try to use hosttime of the sample if available, because iSight Pts seems broken */
    NSNumber *hosttime = (NSNumber *)[sampleBuffer attributeForKey:QTSampleBufferHostTimeAttribute];
    if( hosttime ) currentPts = (time_t)AudioConvertHostTimeToNanos([hosttime unsignedLongLongValue])/1000;
    
    if (layer) 
        layer->buffer = (void *)currentFrame;
        
    [grabberView feedFrame:currentFrame];
    [lock unlock];
    CVBufferRelease(imageBufferToRelease);
}


- (IBAction)startCapture:(id)sender
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
    
    Context *ctx = [freej getContext];
    int h = (ctx->screen->h < CV_GRABBER_HEIGHT_MAX)?ctx->screen->h:CV_GRABBER_HEIGHT_MAX;
    int w = (ctx->screen->w < CV_GRABBER_WIDTH_MAX)?ctx->screen->w:CV_GRABBER_WIDTH_MAX;
    /* Hack - This will lower CPU consumption for some reason */
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
    [grabberView start];

    return;
error:
    //[= exitQTKitOnThread];
    [input release];

}

- (IBAction)stopCapture:(id)sender
{
    [lock lock];
    running = false;
    [grabberView stop];
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

/* CVCaptureView is intended just as a bridge between the CVGrabber class and 
 * the CVFilterPanel used to configure/apply filters. 
 * This 'glue' class is necessary only because the CVGrabber needs to be a subclass
 * QTCaptureDecompressedVideoOutput. 
 * XXX - Perhaps there is a better way to do this ...
 * something that could make it easier to apply filters on the preview window
 */ 

@implementation CVGrabberView : CVLayerView

- (id)init
{
    exportedFrame = nil;
    currentFrame = nil;
    return [super init];
}

- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp
{
    [lock lock];
    if (exportedFrame)
        CVPixelBufferRelease(exportedFrame);
    exportedFrame = CVPixelBufferRetain(currentFrame);
    [lock unlock];
    return kCVReturnSuccess;
}

- (void)feedFrame:(CVPixelBufferRef)frame
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [lock lock];
    currentFrame = frame;
    newFrame = YES;
    [self renderPreview];
    [lock unlock];
    [pool release];
}

- (void)task
{
}

- (void)start
{
    if (!layer) {
        Context *ctx = [freej getContext];
        if (ctx) {
            layer = new CVLayer((NSObject *)self);
            layer->init(ctx);
            layer->buffer = (void *)pixelBuffer;
        }
    }
   // [self toggleVisibility];
}

- (void)stop
{
    if (layer) {
        layer->deactivate();
    }

}

@end
