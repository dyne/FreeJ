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

#import <CVLayerController.h>
#import <CVLayerView.h>
#import <CIAlphaFade.h>
#import <CVFilterPanel.h>
#import <QTKit/QTMovie.h>
#import <CVFilterInstance.h>


/* Utility to set a SInt32 value in a CFDictionary
 */
static OSStatus SetNumberValue(CFMutableDictionaryRef inDict,
                               CFStringRef inKey,
                               SInt32 inValue)
{
    CFNumberRef number;
    
    number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inValue);
    if (NULL == number) return coreFoundationUnknownErr;
    
    CFDictionarySetValue(inDict, inKey, number);
    
    CFRelease(number);
    
    return noErr;
}

@implementation CVLayerController : NSObject

- (void)awakeFromNib
{
    //[self init];
}

- (id)init
{
    return [self initWithContext:nil];
}

- (id)initWithContext:(CFreej *)ctx
{
    CGLPixelFormatObj pFormat;
    GLint npix;
    const int attrs[2] = { kCGLPFADoubleBuffer, NULL};
    CGLError err = CGLChoosePixelFormat (
        (CGLPixelFormatAttribute *)attrs,
        &pFormat,
        &npix
    );
    freej = ctx;
    err = CGLCreateContext(pFormat , NULL, &glContext);
    lock = [[NSRecursiveLock alloc] init];
    [layerView setNeedsDisplay:NO];
    layer = NULL;
    doFilters = true;
    currentFrame = NULL;
    cvTexture = NULL;
    posterImage = NULL;
    currentPreviewTexture = NULL;
    doPreview = YES;
    imageParams = [[NSMutableDictionary dictionary] retain];
    
    return self;
}


/*
- (void)setContext:(CFreej *)ctx
{
    freej = ctx;
}
*/

- (void)dealloc
{
    [colorCorrectionFilter release];
    [compositeFilter release];
    [alphaFilter release];
    [exposureAdjustFilter release];
    [rotateFilter release];
    [scaleFilter release];
    [translateFilter release];
    ///[timeCodeOverlay release];
    CVOpenGLTextureRelease(currentFrame);
    if (imageParams)
        [imageParams release];
    [lock release];
    [super dealloc];
}


- (void)prepareOpenGL
{
    CGOpenGLDisplayMask    totalDisplayMask = 0;
    int     virtualScreen;
    GLint   displayMask;
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    
    // Create display link 
    if (layerView) {
        NSOpenGLPixelFormat    *openGLPixelFormat = [layerView pixelFormat];
        viewDisplayID = (CGDirectDisplayID)[[[[[layerView window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  // we start with our view on the main display
        // build up list of displays from OpenGL's pixel format
        for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
        {
            [openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
            totalDisplayMask |= displayMask;
        }
    }
    // Setup the timecode overlay
    /*
     NSDictionary *fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
     [NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
     nil];
     */
    //timeCodeOverlay = [[TimeCodeOverlay alloc] initWithAttributes:fontAttributes targetSize:NSMakeSize(720.0,480.0 / 4.0)];    // text overlay will go in the bottom quarter of the display
    
    GLint params[] = { 1 };
    CGLSetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, params );
    
    [pool release];
}

- (void)feedFrame:(CVPixelBufferRef)frame
{
    CIImage     *renderedImage = nil;
    Layer       *fjLayer = NULL;
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    if (layer)
        fjLayer = layer->fjLayer();
    else
        return;
    [lock lock];
    if (!currentFrame) {        
        CVReturn err = CVPixelBufferCreate (
                                            NULL,
                                            fjLayer->geo.w,
                                            fjLayer->geo.h,
                                            k32ARGBPixelFormat,
                                            NULL,
                                            &currentFrame
                                            );
    }
    //currentFrame = CVPixelBufferRetain(frame);
    // TODO - check error code
    CIImage *inputImage = [CIImage imageWithCVImageBuffer:frame];

    CGRect bounds = CGRectMake(0, 0, fjLayer->geo.w, fjLayer->geo.h);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CVPixelBufferLockBaseAddress(currentFrame, 0);
    // ensure to start drawing on a black background
    memset(CVPixelBufferGetBaseAddress(currentFrame), 0, CVPixelBufferGetDataSize(currentFrame));
    
    CGContextRef context = CGBitmapContextCreateWithData(
                                                         CVPixelBufferGetBaseAddress(currentFrame),
                                                         fjLayer->geo.w,
                                                         fjLayer->geo.h,
                                                         8,
                                                         fjLayer->geo.w*4,
                                                         colorSpace,
                                                         kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big,
                                                         NULL,
                                                         NULL
                                                         );
    CVPixelBufferUnlockBaseAddress(currentFrame, 0);
    if (!context) {
        // todo - error messages
        return;
    }
    NSDictionary *ciContextOptions = [NSDictionary dictionaryWithObject:(NSString*)kCGColorSpaceGenericRGB 
                                                                 forKey: kCIContextOutputColorSpace];
    CIContext *ciCtx = [CIContext contextWithCGContext:context options:ciContextOptions];
    //CGContextRelease(context);
    
//    if (doFilters) {
        [colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
        [exposureAdjustFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
        [alphaFilter  setValue:[exposureAdjustFilter valueForKey:@"outputImage"]
                        forKey:@"inputImage"];
        [rotateFilter setValue:[alphaFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
        if (fjLayer && (fjLayer->geo.x || fjLayer->geo.y)) {
            NSAffineTransform   *translateTransform = [NSAffineTransform transform];
            [translateTransform translateXBy:fjLayer->geo.x yBy:fjLayer->geo.y];
            [translateFilter setValue:translateTransform forKey:@"inputTransform"];
            
            [translateFilter setValue:[rotateFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
            renderedImage = [translateFilter valueForKey:@"outputImage"];
        } else {
            renderedImage = [rotateFilter valueForKey:@"outputImage"];
        }
//    } else {
//        renderedImage = inputImage;
//    }
    
    [ciCtx drawImage:renderedImage inRect:bounds fromRect:bounds];
    newFrame = YES;
    [lock unlock];
    [pool release];
    //[self renderFrame];
}

- (CVReturn)renderFrame
{
    CIImage     *inputImage = nil;
    CIImage     *renderedImage = nil;
    Layer       *fjLayer = NULL;
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    if (layer)
        fjLayer = layer->fjLayer();
    else
        return kCVReturnError;
    
    if (!filtersInitialized)
        [self initFilters]; // initialize on first use
    
    if (newFrame) {
        [lock lock];
        if (cvTexture)
            [cvTexture release];
        cvTexture = [[CVTexture alloc] initWithCIImage:inputImage pixelBuffer:currentFrame];
        newFrame = NO;
        [lock unlock];
        //[self renderPreview];
    }
    [pool release];
    return kCVReturnSuccess;
}

- (IBAction)toggleFilters:(id)sender
{
    doFilters = doFilters?false:true;
}

- (IBAction)toggleVisibility:(id)sender
{
    if (layer)
        if (layer->isActive())
            layer->deactivate();
        else
            layer->activate();
}

- (IBAction)togglePreview:(id)sender
{
    doPreview = doPreview?NO:YES;
}

- (void) setLayer:(CVCocoaLayer *)lay
{
    if (lay) {
        layer = lay;
        //layer->fps.set(30);
    } 
}

- (NSDictionary *)imageParams
{
    return imageParams;
}

- (IBAction)setValue:(NSNumber *)value forImageParameter:(NSString *)parameter
{
    Layer *fjLayer = NULL;
    NSAutoreleasePool *pool;
    float deg = 0;
    float x = 0;
    float y = 0;
    NSAffineTransform    *rotateTransform;
    NSAffineTransform    *rototranslateTransform;
    NSString *paramName = NULL;
    pool = [[NSAutoreleasePool alloc] init];
    if (layer)
        fjLayer = layer->fjLayer();
    else
        return;
    
    // TODO - optimize the logic in this routine ... it's becoming huge!!
    // to prevent its run() method to try rendering
    // a frame while we change filter parameters
    [lock lock];
#if 0
    switch([sender tag])
    {
        case 0:  // opacity (AlphaFade)
            [alphaFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"outputOpacity"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 1: //brightness (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputBrightness"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 2: // saturation (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSaturation"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 3: // contrast (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputContrast"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 4: // exposure (ExposureAdjust)
            [exposureAdjustFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputEV"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 5: // rotate 
            rotateTransform = [NSAffineTransform transform];
            [rotateTransform rotateByDegrees:[sender floatValue]];
            deg = ([sender floatValue]*M_PI)/180.0;
            if (deg && fjLayer) {
                x = ((fjLayer->geo.w)-((fjLayer->geo.w)*cos(deg)-(fjLayer->geo.h)*sin(deg)))/2;
                y = ((fjLayer->geo.h)-((fjLayer->geo.w)*sin(deg)+(fjLayer->geo.h)*cos(deg)))/2;
            }
            rototranslateTransform = [NSAffineTransform transform];
            [rototranslateTransform translateXBy:x yBy:y];
            [rotateTransform appendTransform:rototranslateTransform];
            [rotateTransform concat];
            [rototranslateTransform concat];
            [rotateFilter setValue:rotateTransform forKey:@"inputTransform"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender label]];
            break;
        case 6: // traslate X
            if (fjLayer) 
                fjLayer->geo.x = [sender floatValue];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 7: // traslate Y
            if (fjLayer)
                fjLayer->geo.y = [sender floatValue];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        default:
            break;
    }
#endif
    [lock unlock];
    [pool release];
}

- (IBAction)setImageParameter:(id)sender
{
    Layer *fjLayer = NULL;
    NSAutoreleasePool *pool;
    float deg = 0;
    float x = 0;
    float y = 0;
    NSAffineTransform    *rotateTransform;
    NSAffineTransform    *rototranslateTransform;
    NSString *paramName = NULL;
    pool = [[NSAutoreleasePool alloc] init];
    if (layer)
        fjLayer = layer->fjLayer();
    else
        return;

    // TODO - optimize the logic in this routine ... it's becoming huge!!
    // to prevent its run() method to try rendering
    // a frame while we change filter parameters
    [lock lock];
    switch([sender tag])
    {
        case 0:  // opacity (AlphaFade)
            [alphaFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"outputOpacity"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 1: //brightness (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputBrightness"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 2: // saturation (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSaturation"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 3: // contrast (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputContrast"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 4: // exposure (ExposureAdjust)
            [exposureAdjustFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputEV"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 5: // rotate 
            rotateTransform = [NSAffineTransform transform];
            [rotateTransform rotateByDegrees:[sender floatValue]];
            deg = ([sender floatValue]*M_PI)/180.0;
            if (deg && fjLayer) {
                x = ((fjLayer->geo.w)-((fjLayer->geo.w)*cos(deg)-(fjLayer->geo.h)*sin(deg)))/2;
                y = ((fjLayer->geo.h)-((fjLayer->geo.w)*sin(deg)+(fjLayer->geo.h)*cos(deg)))/2;
            }
            rototranslateTransform = [NSAffineTransform transform];
            [rototranslateTransform translateXBy:x yBy:y];
            [rotateTransform appendTransform:rototranslateTransform];
            [rotateTransform concat];
            [rototranslateTransform concat];
            [rotateFilter setValue:rotateTransform forKey:@"inputTransform"];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 6: // traslate X
            if (fjLayer) 
                fjLayer->geo.x = [sender floatValue];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 7: // traslate Y
            if (fjLayer)
                fjLayer->geo.y = [sender floatValue];
            [imageParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        default:
            break;
    }
    [lock unlock];
    [pool release];
}

- (void)setBlendMode:(NSString *)mode
{
    if (layer) 
        layer->blendMode = mode;
}

- (void)setFilterCenterFromMouseLocation:(NSPoint)where
{
    CIVector    *centerVector = nil;
    
    //[lock lock];
    
    centerVector = [CIVector vectorWithX:where.x Y:where.y];
#if 0
    [effectFilter setValue:centerVector forKey:@"inputCenter"];
#endif
    //[lock unlock];
}

- (void)renderPreview
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if ([self doPreview] && previewTarget) { 
        // scale the frame to fit the preview
        if (![previewTarget isHiddenOrHasHiddenAncestor]) {
            Layer *fjLayer = layer->fjLayer();
            // XXX - it's too dangerous to access layer->buffer directly
            if (fjLayer && fjLayer->buffer)
                [previewTarget renderFrame:fjLayer];
        }
        
    }
    [pool release];
}

- (void)initFilters {
    // Create CIFilters used for both preview and main frame
    if (!colorCorrectionFilter) {
        colorCorrectionFilter = [[CIFilter filterWithName:@"CIColorControls"] retain];        // Color filter  
        [colorCorrectionFilter setDefaults]; // set the filter to its default values
    }
    if (!exposureAdjustFilter) {
        exposureAdjustFilter = [[CIFilter filterWithName:@"CIExposureAdjust"] retain];
        [exposureAdjustFilter setDefaults];
        // adjust exposure
        [exposureAdjustFilter setValue:[NSNumber numberWithFloat:0.0] forKey:@"inputEV"];
    }
    
    // rotate
    if (!rotateFilter) {
        NSAffineTransform *rotateTransform = [NSAffineTransform transform];
        rotateFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        [rotateTransform rotateByDegrees:0.0];
        [rotateFilter setValue:rotateTransform forKey:@"inputTransform"];
    }
    
    if (!translateFilter) {
        translateFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        NSAffineTransform   *translateTransform = [NSAffineTransform transform];
        [translateTransform translateXBy:0.0 yBy:0.0];
        [translateFilter setValue:translateTransform forKey:@"inputTransform"];
    }
    if (!scaleFilter) {
        scaleFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
        //CIFilter *scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
        [scaleFilter setDefaults];    // set the filter to its default values
    }
    //[scaleFilter setValue:[NSNumber numberWithFloat:scaleFactor] forKey:@"inputScale"];

    if (!compositeFilter) {
        compositeFilter = [[CIFilter filterWithName:@"CISourceOverCompositing"] retain];    // Composite filter
        [compositeFilter setDefaults];
    }
    //[CIAlphaFade class];
    if (!alphaFilter) {
        alphaFilter = [[CIFilter filterWithName:@"CIAlphaFade"] retain]; // AlphaFade filter
        [alphaFilter setDefaults]; // XXX - setDefaults doesn't work properly
#if MAC_OS_X_VERSION_10_6
        [alphaFilter setValue:[NSNumber numberWithFloat:1.0] forKey:@"outputOpacity"]; // set default value
#else
        [alphaFilter setValue:[NSNumber numberWithFloat:0.5] forKey:@"outputOpacity"]; // set default value
#endif
    }
    filtersInitialized = true;
}

- (CVTexture *)getTexture
{    
    CVTexture   *texture = nil;
    [lock lock];
    texture = [cvTexture retain];
    [lock unlock];

    return [texture autorelease];
}


- (bool)needPreview
{
    return doPreview;
}

- (void)startPreview
{  
    doPreview = YES;
}

- (void)start
{
    if (!layer) {
        /* TODO - avoid creating a CVLayer directly,
                  we should only know about CVCocoaLayer here */
        CVLayer *cvLayer = new CVLayer(self);
        if (freej) {
            // TODO Geometry should expose a proper API
            Context *ctx = [freej getContext];
			/*
            cvLayer->geo.w = ctx->screen->geo.w;
            cvLayer->geo.h = ctx->screen->geo.h;
			 */
            cvLayer->init(ctx->screen->geo.w, ctx->screen->geo.h, 32);
        } else {
            cvLayer->init();
        }
        cvLayer->activate();
        layer = (CVCocoaLayer *)cvLayer;
    }
}

- (void)stop
{
    if (layer) {
        layer->deactivate();
		delete layer;
		layer = NULL;
    }
    
}

- (CVPreview *)getPreviewTarget
{
    return previewTarget;
}

- (void)setPreviewTarget:(CVPreview *)targetView
{
    [lock lock];
    previewTarget = targetView;
    [lock unlock];
}

- (void)stopPreview
{
    doPreview = NO;
}

- (void)lock
{
    [lock lock];
}

- (void)unlock
{
    [lock unlock];
}

- (bool)isVisible
{
    if (layer)
        return layer->isVisible();
    return NO;
}

- (void)activate
{    if (layer) {
        // ensure activating the underlying layer ... 
        // this won't do anything if the layer is already active
        layer->activate();
        if (freej) {
            Layer *fjLayer = layer->fjLayer();
            if (!fjLayer->screen) {
                Context *ctx = [freej getContext];
                ctx->screen->add_layer(layer->fjLayer());
            }
        }
    }
}

- (NSString *)blendMode {
    if (layer)
        return layer->blendMode;
    return NULL;
}

- (void)deactivate
{
    if (layer)
        layer->deactivate();
}

- (void)rotateBy:(float)deg
{
    if (layer) {
        
    }
}

- (void)translateXby:(float)x Yby:(float)y
{
    if (layer)
		layer->setOrigin(x, y);
}

- (void)toggleFilters
{
    doFilters = doFilters?false:true;
}

- (void)toggleVisibility
{
    if (layer)
        if (layer->isActive())
            layer->deactivate();
        else
            layer->activate();
}

- (void)togglePreview
{
    doPreview = doPreview?NO:YES;
}

- (bool)doPreview
{
    return doPreview;
}

- (char *)name {
    //if (layer)
      //  return layer->fj_name();
    if (layerView)
        return (char *)[[layerView toolTip] UTF8String];
    return (char*)"CVCocoaLayer";
}

- (Linklist<FilterInstance> *)activeFilters
{
    NSMutableArray *result = nil;
    if (layer) {
        Layer *fjLayer = layer->fjLayer();
        if (fjLayer) {
           return &fjLayer->filters;
        }
    }
    return NULL;
}

- (int)width
{
	if (layer) {
		return layer->width();
	} else if (freej) {
		Context *ctx = [freej getContext];
		return ctx->screen->geo.w;
	}
	return 0;
}

- (int)height
{
	if (layer) {
		return layer->height();
	} else if (freej) {
		Context *ctx = [freej getContext];
		return ctx->screen->geo.h;
	}
	return 0;
}

@synthesize freej;
@synthesize layer;
@synthesize layerView;

@end
