/*
 *  CVideo_layer.cpp
 *  freej
 *
 *  Created by xant on 2/9/09.
 *  Copyright 2009 dyne. All rights reserved.
 *
 */

#include "CVGrabber.h"
#include "CFreej.h"
#import "CIAlphaFade.h"
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
    [lock unlock];
    CVBufferRelease(imageBufferToRelease);
}

- (void)awakeFromNib
{
    //[self setSize:self];
}

 
- (void)windowWillClose:(NSNotification *)notification
{
    //[mCaptureSession stopRunning];
    //[[mCaptureDeviceInput device] close];
 
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

    [captureView setCaptureSession:session];
    [session startRunning]; // start the capture session
    notice( "Video device ready!" );

    running = true;
    if (!layer) {
        layer = new CVLayer((NSObject *)self);
        layer->init(ctx);
    }
    layer->activate();
    return;
error:
    //[= exitQTKitOnThread];
    [input release];

}

- (IBAction)stopCapture:(id)sender
{
    [lock lock];
    if (layer) {
        layer->deactivate();
    }
    running = false;
    
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

- (CIImage *)getTexture
{
    time_t pts;
    
    CIImage     *inputImage = NULL;

    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    [lock lock];
    if (currentFrame != lastFrame) {
    //if (currentPts == previousPts) { 
        lastFrame = currentFrame;
        if (renderedImage)
            [renderedImage release];
        inputImage = [CIImage imageWithCVImageBuffer:currentFrame];
        
        if (inputImage) {
            //if (doFilters) {
                [colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
                [exposureAdjustFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
                [effectFilter setValue:[exposureAdjustFilter valueForKey:@"outputImage"] 
                              forKey:@"inputImage"];
                [alphaFilter  setValue:[effectFilter valueForKey:@"outputImage"]
                              forKey:@"inputImage"];
                [rotateFilter setValue:[alphaFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
                //[translateFilter setValue:[rotateFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
                renderedImage = [rotateFilter valueForKey:@"outputImage"];
                                
                renderedImage = [renderedImage retain];
            //} else {
            //    renderedImage = [inputImage retain];
            //}
        }
    } 
    [lock unlock];
    [pool release];
    return [renderedImage retain];
}

- (IBAction)setFilterParameter:(id)sender
{
    NSAutoreleasePool *pool;
    float deg = 0;
    float x = 0;
    float y = 0;
    NSAffineTransform    *rotateTransform;
    NSAffineTransform    *translateTransform;
    NSString *paramName = NULL;
    pool = [[NSAutoreleasePool alloc] init];
    [lock lock];
    switch([sender tag])
    {
    case 0:  // opacity (AlphaFade)
        [alphaFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"outputOpacity"];
        break;
    case 1: //brightness (ColorCorrection)
        [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputBrightness"];
        break;
    case 2: // saturation (ColorCorrection)
        [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSaturation"];
        break;
    case 3: // contrast (ColorCorrection)
        [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputContrast"];
        break;
    case 4: // exposure (ExposureAdjust)
        [exposureAdjustFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputEV"];
        break;
    case 5: // rotate and translate
        rotateTransform = [NSAffineTransform transform];
        [rotateTransform rotateByDegrees:[sender floatValue]];
         deg = ([sender floatValue]*M_PI)/180.0;
        if (deg) {
             x = ((layer->geo.w)-((layer->geo.w)*cos(deg)-(layer->geo.h)*sin(deg)))/2;
             y = ((layer->geo.h)-((layer->geo.w)*sin(deg)+(layer->geo.h)*cos(deg)))/2;
        }
        translateTransform = [NSAffineTransform transform];
        [translateTransform translateXBy:x yBy:y];
        [rotateTransform appendTransform:translateTransform];
        [rotateTransform concat];
        [translateTransform concat];
        [rotateFilter setValue:rotateTransform forKey:@"inputTransform"];
        
        break;
    case 6:
        NSString *filterName = [[NSString alloc] initWithFormat:@"CI%@", [[sender selectedItem] title]];
        NSLog(filterName);
        [effectFilter release];
        effectFilter = [[CIFilter filterWithName:filterName] retain];    
        FilterParams *pdescr = [filterPanel getFilterParamsDescriptorAtIndex:[sender indexOfSelectedItem]];
        [effectFilter setDefaults];
        NSView *cView = (NSView *)sender;
        for (int i = 0; i < 3; i++) {
            NSTextField *label = (NSTextField *)[cView nextKeyView];
            NSSlider *slider = (NSSlider *)[label nextKeyView];

            if (i < pdescr->nParams) {
                [label setHidden:NO];
                NSString *pLabel = [[[NSString alloc] initWithCString:pdescr->params[i].label] retain];
                [label setTitleWithMnemonic:pLabel];
                [slider setHidden:NO];
                [slider setMinValue:pdescr->params[i].min];
                [slider setMaxValue:pdescr->params[i].max];
                [slider setDoubleValue:pdescr->params[i].min];
                if ([paramNames count] > i) {
                    NSString *old = [paramNames objectAtIndex:i];
                    [paramNames replaceObjectAtIndex:i withObject:pLabel];
                    [old release];    
                } else {
                    [paramNames insertObject:pLabel atIndex:i];
                }
                if ([pLabel isEqual:@"CenterY"])
                    [slider setMaxValue:layer->geo.h];
                else if ([pLabel isEqual:@"CenterX"])
                    [slider setMaxValue:layer->geo.w];
                else
                    [effectFilter setValue:[NSNumber numberWithFloat:pdescr->params[i].min] forKey:pLabel];
            } else {
                [label setHidden:YES];
                [slider setHidden:YES];
            }
            cView = slider;
        }
        break;
    case 7:
        paramName = [paramNames objectAtIndex:0];
        if ([paramName isEqual:@"CenterX"]) {
            NSSlider *y = (NSSlider *)[[sender nextKeyView] nextKeyView];
            [effectFilter setValue:[CIVector vectorWithX:[sender floatValue] Y:[y floatValue]]
                forKey:@"inputCenter"];
        } else { 
            [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
        }
        break;
    case 8:
        paramName = [paramNames objectAtIndex:1];
        if ([paramName isEqual:@"CenterY"]) {
            NSSlider *x = (NSSlider *)[[sender previousKeyView] previousKeyView];
            [effectFilter setValue:[CIVector vectorWithX:[x floatValue] Y:[sender floatValue]]
                forKey:@"inputCenter"];
        } else { 
            [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
        }
        break;
    case 9:
        [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[paramNames objectAtIndex:2]];
        break;
    case 10:
        [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[paramNames objectAtIndex:3]];
        break;
    default:
        break;
    }
    [lock unlock];
    [pool release];
}

- (IBAction)setBlendMode:(id)sender
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if (layer) {
        [lock lock];
        switch([sender tag])
        {
            case 0:
                NSString *blendMode = [[NSString alloc] initWithFormat:@"CI%@BlendMode", [[sender selectedItem] title]];
                layer->blendMode = blendMode;
                break;
            default:
                break;
        }
        [lock unlock];
    }
    [pool release];
}

- (NSString *)toolTip
{
    return [captureView toolTip];
}



//    Delegate of NSCaptureView
- (CIImage *)view:(QTCaptureView *)view willDisplayImage :(CIImage *)inputImage 
{
    //NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if (outputImage)
        [outputImage release];
    [lock lock];

    [colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
    
    [exposureAdjustFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
    [effectFilter setValue:[exposureAdjustFilter valueForKey:@"outputImage"] 
                  forKey:@"inputImage"];
    // we don't need the alpha filter in the preview
    [rotateFilter setValue:[effectFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
    //[translateFilter setValue:[rotateFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
    outputImage = [rotateFilter valueForKey:@"outputImage"];
    [lock unlock];

    //[pool release];
    return [outputImage retain];
}


@synthesize layer;
@synthesize filterPanel;
@end

/* CVCaptureView is intended just as a bridge between the CVGrabber class and 
 * the CVFilterPanel used to configure/apply filters. 
 * This 'glue' class is necessary only because the CVGrabber needs to be a subclass
 * QTCaptureDecompressedVideoOutput. 
 * XXX - Perhaps there is a better way to do this ...
 * something that could make it easier to apply filters on the preview window
 */ 

@implementation CVCaptureView : QTCaptureView
- (id)init
{
    if( self = [super init] ) {
        return self;
    }
    return nil;
}

- (void)dealloc
{
    if (filterPanel)
        [filterPanel dealloc];
    [super dealloc];
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (!filterPanel) {
        filterPanel =  [[CVFilterPanel alloc] initWithName:[self toolTip]];
        [grabber setFilterPanel:filterPanel];
        [filterPanel setLayer:(id)grabber];
    }

    [filterPanel show];
    [super mouseDown:theEvent];
}


@end
