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
    lastFrame = NULL;
    posterImage = NULL;
    currentPreviewTexture = NULL;
    doPreview = YES;
    filterParams = [[NSMutableDictionary dictionary] retain];
    
    return self;
}

- (void)setContext:(CFreej *)ctx
{
    freej = ctx;
}

- (void)dealloc
{
    [colorCorrectionFilter release];
    [effectFilter release];
    [compositeFilter release];
    [alphaFilter release];
    [exposureAdjustFilter release];
    [rotateFilter release];
    [scaleFilter release];
    [translateFilter release];
    ///[timeCodeOverlay release];
    CVOpenGLTextureRelease(currentFrame);
    if (filterParams)
        [filterParams release];
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
    [lock lock];
    if (currentFrame)
        CVPixelBufferRelease(currentFrame);
    currentFrame = CVPixelBufferRetain(frame);
    newFrame = YES;
    [lock unlock];
    [self renderPreview];
}

- (CVReturn)renderFrame
{
    NSLog(@"renderFrame MUST be overridden");
    return kCVReturnError;
}

- (IBAction)toggleFilters:(id)sender
{
    doFilters = doFilters?false:true;
}

- (IBAction)toggleVisibility:(id)sender
{
    if (layer)
        if (layer->is_active())
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

- (NSString *)filterName
{
    NSString *filter = nil;
    [lock lock];
    if (effectFilter)
        filter = [effectFilter name];
    [lock unlock];
    return filter;
}

- (NSDictionary *)filterParams
{
    return filterParams;
}

- (IBAction)setFilterParameter:(id)sender
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
        fjLayer = layer->fj_layer();

    // TODO - optimize the logic in this routine ... it's becoming huge!!
    // to prevent its run() method to try rendering
    // a frame while we change filter parameters
    [lock lock];
    switch([sender tag])
    {
        case 0:  // opacity (AlphaFade)
            [alphaFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"outputOpacity"];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 1: //brightness (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputBrightness"];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 2: // saturation (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSaturation"];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 3: // contrast (ColorCorrection)
            [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputContrast"];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 4: // exposure (ExposureAdjust)
            [exposureAdjustFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputEV"];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
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
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 6: // traslate X
            if (fjLayer) 
                fjLayer->geo.x = [sender floatValue];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 7: // traslate Y
            if (fjLayer)
                fjLayer->geo.y = [sender floatValue];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
            break;
        case 100:
		{
            NSString *filterName = [NSString stringWithFormat:@"CI%@", [[sender selectedItem] title]];
            //NSLog(filterName);
            [effectFilter release];
            effectFilter = [[CIFilter filterWithName:filterName] retain];
            [effectFilter setDefaults];
            [effectFilter setName:[[sender selectedItem] title]]; 
            FilterParams *pdescr = [[layerView filterPanel] getFilterParamsDescriptorAtIndex:[sender indexOfSelectedItem]];
            NSView *cView = (NSView *)sender;
            for (int i = 0; i < 4; i++) {
                NSTextField *label = (NSTextField *)[cView nextKeyView];
                NSSlider *slider = (NSSlider *)[label nextKeyView];
                
                if (i < pdescr->nParams) {
                    [label setHidden:NO];
                    [label setTitleWithMnemonic:[NSString stringWithUTF8String:pdescr->params[i].label]];
                    
                    // first update sliders' min and max values
                    [slider setToolTip:[label stringValue]];
                    [slider setHidden:NO];
                    [slider setMinValue:pdescr->params[i].min];
                    [slider setMaxValue:pdescr->params[i].max];
                    NSNumber *value = [filterParams valueForKey:[label stringValue]];
                    if (value) 
                        [slider setDoubleValue:[value floatValue]];
                    else
                        [slider setDoubleValue:pdescr->params[i].min];
                    // than update the current value for this specific layer (saved in filterParams)
                    [filterParams setValue:[NSNumber numberWithFloat:[slider doubleValue]]  forKey:[label stringValue]];
                    
                    // handle the case it refers to a "center" coordinate
                    if (strcmp(pdescr->params[i].label, "CenterY") == 0) {
                        if (fjLayer)
                            [slider setMaxValue:fjLayer->geo.h];
                        NSSlider *x = (NSSlider *)[[slider previousKeyView] previousKeyView];
                        [effectFilter setValue:[CIVector vectorWithX:[x floatValue] Y:[slider floatValue]]
                                        forKey:@"inputCenter"];
                    } else if (strcmp(pdescr->params[i].label, "CenterX") == 0) {
                        if (fjLayer)
                            [slider setMaxValue:fjLayer->geo.w];
                        NSSlider *y = (NSSlider *)[[slider nextKeyView] nextKeyView];
                        [effectFilter setValue:[CIVector vectorWithX:[slider floatValue] Y:[y floatValue]]
                                        forKey:@"inputCenter"];
                    } else {
                        [effectFilter setValue:[NSNumber numberWithFloat:[slider doubleValue]] forKey:[label stringValue]];
                    }
                } else {
                    // hide unused sliders
                    [label setHidden:YES];
                    [slider setHidden:YES];
                }
                cView = slider;
            }
            break;
		}
        case 101:
            paramName = [sender toolTip];
            if ([paramName isEqual:@"CenterX"]) {
                NSSlider *y = (NSSlider *)[[sender nextKeyView] nextKeyView];
                [effectFilter setValue:[CIVector vectorWithX:[sender floatValue] Y:[y floatValue]]
                                forKey:@"inputCenter"];
            } else { 
                [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            }
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            break;
        case 102:
            paramName = [sender toolTip];
            if ([paramName isEqual:@"CenterY"]) {
                NSSlider *x = (NSSlider *)[[sender previousKeyView] previousKeyView];
                [effectFilter setValue:[CIVector vectorWithX:[x floatValue] Y:[sender floatValue]]
                                forKey:@"inputCenter"];
            } else { 
                [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            }
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            break;
        case 103:
            paramName = [sender toolTip];
            [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            break;
        case 104:
            paramName = [sender toolTip];
            [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:paramName];
            [filterParams setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:[sender toolTip]];
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
    [effectFilter setValue:centerVector forKey:@"inputCenter"];
    
    //[lock unlock];
}

- (void)renderPreview
{
    CVPreview *previewTarget = [layerView getPreviewTarget];
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if ([self doPreview] && previewTarget) { 
        // scale the frame to fit the preview
        if (![previewTarget isHiddenOrHasHiddenAncestor])
            [previewTarget renderFrame:[self getTexture]];
        
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
    if (!effectFilter) {
        effectFilter = [[CIFilter filterWithName:@"CIZoomBlur"] retain];            // Effect filter  
        [effectFilter setDefaults];                                // set the filter to its default values
        [effectFilter setName:@"ZoomBlur"];
        [effectFilter setValue:[NSNumber numberWithFloat:0.0] forKey:@"inputAmount"]; // don't apply effects at startup
    }
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
    CIImage     *inputImage = nil;
    CVTexture   *texture = nil;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CIImage     *renderedImage = nil;
    Layer       *fjLayer = NULL;
    
    if (layer)
        fjLayer = layer->fj_layer();
    
    if (!filtersInitialized)
        [self initFilters];
    [lock lock];
    if (newFrame) {       
        inputImage = [CIImage imageWithCVImageBuffer:currentFrame];
        newFrame = NO;
        if (doFilters) {    
            [colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
            [exposureAdjustFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
            [effectFilter setValue:[exposureAdjustFilter valueForKey:@"outputImage"] 
                            forKey:@"inputImage"];
            [alphaFilter  setValue:[effectFilter valueForKey:@"outputImage"]
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
        } else {
            renderedImage = inputImage;
        }
        texture = [[CVTexture alloc] initWithCIImage:renderedImage pixelBuffer:currentFrame];
        
        if (lastFrame)
            [lastFrame release];
        lastFrame = [texture retain];
    } else { 
        texture = [lastFrame retain];
    }
    [lock unlock];
    [pool release];
    return texture;
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
        cvLayer->init();
        cvLayer->activate();
        if (freej) {
            // TODO Geometry should expose a proper API
            Context *ctx = [freej getContext];
            cvLayer->geo.w = ctx->screen->geo.w;
            cvLayer->geo.h = ctx->screen->geo.h;
        }
        layer = cvLayer;
    }
}

- (void)stop
{
    if (layer) {
        layer->deactivate();
    }
    
}

- (void)setPreviewTarget:(CVPreview *)targetView
{
    [lock lock];
    if (layerView)
        [layerView setPreviewTarget:targetView];
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
        return layer->is_visible();
    return NO;
}

- (void)activate
{    if (layer) {
        layer->activate();
        if (freej) {
            Layer *fjLayer = layer->fj_layer();
            if (!fjLayer->screen) {
                Context *ctx = [freej getContext];
                ctx->screen->add_layer(layer->fj_layer());
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
    if (layer) {
        Layer *fjLayer = layer->fj_layer();
        if (fjLayer) {
            fjLayer->geo.x = x;
            fjLayer->geo.y = y;
        }
    }
}

- (void)toggleFilters
{
    doFilters = doFilters?false:true;
}

- (void)toggleVisibility
{
    if (layer)
        if (layer->is_active())
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
    if (layerView)
        return (char *)[[layerView toolTip] UTF8String];
    return (char*)"CVLayer";
}

@synthesize layer;
@synthesize currentFrame;

@end
