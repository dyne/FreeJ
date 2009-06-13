/*
 *  CVideo_layer.cpp
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */

#import "CVGrabber.h"
#import "CFreej.h"
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
        width = 512; // XXX - make defult size configurable
        height = 384; // XXX -^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
    
    [grabberView feedFrame:currentFrame];

    if (imageBufferToRelease)
        CVBufferRelease(imageBufferToRelease);

}


- (IBAction)startCapture:(id)sender
{
    notice( "QTCapture opened" );

    NSError *o_returnedError;
    Context *ctx = [freej getContext];
    /* Hack - using ma resolution seems to lower cpu consuption for some reason */
    int h = (ctx->screen->h < CV_GRABBER_HEIGHT_MAX)?ctx->screen->h:CV_GRABBER_HEIGHT_MAX;
    int w = (ctx->screen->w < CV_GRABBER_WIDTH_MAX)?ctx->screen->w:CV_GRABBER_WIDTH_MAX;
 
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
    [self setDelegate:grabberView];
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

/* This 'glue' class is necessary only because the CVGrabber needs to be a subclass
 * QTCaptureDecompressedVideoOutput. 
 */ 

@implementation CVGrabberView : CVLayerView

- (id)init
{
    static char *suffix = "/Contents/Resources/webcam.png";
    char iconFile[1024];
    
    exportedFrame = nil;
    currentFrame = nil;
    
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(suffix)-1);
    strcat(iconFile, suffix);
    icon = [CIImage imageWithContentsOfURL:
        [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]];
    [icon retain];
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
        layer->buffer = currentFrame;
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

- (void)drawRect:(NSRect)theRect
{
    GLint zeroOpacity = 0;
    if (needsReshape) {
        NSRect bounds = [self bounds];
        NSRect frame = [self frame];
        CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
            NSWidth(bounds), NSHeight(bounds));

        if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
            return;
        [[self openGLContext] makeCurrentContext];

        [[self openGLContext] setValues:&zeroOpacity forParameter:NSOpenGLCPSurfaceOpacity];
        [super drawRect:theRect];
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        [ciContext drawImage:icon
            atPoint: imageRect.origin
            fromRect: imageRect];
        [[self openGLContext] flushBuffer];
        needsReshape = NO;
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }
    [self setNeedsDisplay:NO];
}

- (bool)isOpaque
{
    return NO;
}

@end
