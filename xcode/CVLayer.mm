/*
 *  CVLayer.cpp
 *  freej
 *
 *  Created by xant on 2/23/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#import <QTKit/QTMovie.h>
#include <Cocoa/Cocoa.h>;
#include "CVLayer.h"
#import "CIAlphaFade.h"
#import "CVFilterPanel.h"

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

static CVReturn renderCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
    CVReturn ret = [(CVLayerView *)displayLinkContext _renderTime:inOutputTime];
    return ret;
}

@implementation CVLayerView : NSOpenGLView

- (void)awakeFromNib
{
    [self init];
}

- (id)init
{
    lock = [[NSRecursiveLock alloc] init];
    [lock retain];
    [self setNeedsDisplay:NO];
    needsReshape = YES;
    //freejFrame = NULL;
    doFilters = true;
    currentFrame = NULL;
    lastFrame = NULL;
    posterImage = NULL;
    currentPreviewTexture = NULL;
    if (CVOpenGLBufferCreate (NULL, 400, 300, NULL, &pixelBuffer) != noErr) {
        // TODO - error messages
        pixelBuffer = NULL;
    }
    doPreview = NO;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
    paramNames = [[NSMutableArray arrayWithCapacity:4] retain];
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [colorCorrectionFilter release];
    [effectFilter release];
    [compositeFilter release];
    [alphaFilter release];
    [exposureAdjustFilter release];
    [rotateFilter release];
    [scaleFilter release];
    //[translateFilter release];
    ///[timeCodeOverlay release];
    CVOpenGLTextureRelease(currentFrame);
    if (ciContext)
        [ciContext release];
    if (pixelBuffer)
        CVOpenGLBufferRelease(pixelBuffer);
    if (paramNames)
            [paramNames release];
    [lock release];
    [super dealloc];
}

- (void)prepareOpenGL
{
    CVReturn                ret;
    CGOpenGLDisplayMask    totalDisplayMask = 0;
    int     virtualScreen;
    GLint   displayMask;
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    
    
    // Create CGColorSpaceRef 
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        
    // Create CIContext 
    ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
                pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
                options:nil] retain];
    CGColorSpaceRelease(colorSpace);
  
    // Create CIFilters used for both preview and main frame
    colorCorrectionFilter = [[CIFilter filterWithName:@"CIColorControls"] retain];        // Color filter    
    [colorCorrectionFilter setDefaults];                            // set the filter to its default values
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
                      
    // Create display link 
    NSOpenGLPixelFormat    *openGLPixelFormat = [self pixelFormat];
    viewDisplayID = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  // we start with our view on the main display
    // build up list of displays from OpenGL's pixel format
    for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
    {
        [openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
        totalDisplayMask |= displayMask;
    }
    //ret = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
    ret = CVDisplayLinkCreateWithCGDisplay(viewDisplayID, &displayLink);
    // Set up display link callbacks 
    //CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
        
    // Setup the timecode overlay
    /*
    NSDictionary *fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
        [NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
        nil];
    */
    //timeCodeOverlay = [[TimeCodeOverlay alloc] initWithAttributes:fontAttributes targetSize:NSMakeSize(720.0,480.0 / 4.0)];    // text overlay will go in the bottom quarter of the display
    [pool release];
}

- (void)update
{

}

- (void)drawRect:(NSRect)theRect
{

    NSRect        frame = [self frame];
    NSRect        bounds = [self bounds];
    //[[freej getLock] lock];
    [lock lock];
    if(needsReshape)    // if the view has been resized, reset the OpenGL coordinate system
    {
        GLfloat     minX, minY, maxX, maxY;
        
        minX = NSMinX(bounds);
        minY = NSMinY(bounds);
        maxX = NSMaxX(bounds);
        maxY = NSMaxY(bounds);
        
        [[self openGLContext] makeCurrentContext];

        [self update]; 

        if(NSIsEmptyRect([self visibleRect])) 
        {
            glViewport(0, 0, 1, 1);
        } else {
            glViewport(0, 0,  frame.size.width ,frame.size.height);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(minX, maxX, minY, maxY, -1.0, 1.0);
        
        glClearColor(0.0, 0.0, 0.0, 0.0);         
        glClear(GL_COLOR_BUFFER_BIT);
        
        [[self openGLContext] flushBuffer];
        needsReshape = NO;
    }
    
    // clean the OpenGL context - not so important here but very important when you deal with transparenc

    [self setNeedsDisplay:NO];
    //[[freej getLock] unlock];
    [lock unlock];

}


- (CVReturn)_renderTime:(const CVTimeStamp *)time
{
    NSLog(@"_renderTime MUST be overridden");
    return kCVReturnError;
}

- (IBAction)toggleFilters:(id)sender
{
    doFilters = doFilters?false:true;
}

- (IBAction)toggleVisibility:(id)sender
{
    if (layer)
        if (layer->active)
            layer->deactivate();
        else
            layer->activate();
}

- (IBAction)togglePreview:(id)sender
{
    doPreview = doPreview?NO:YES;
}

- (IBAction)setAlpha:(id)sender
{
    if (layer) {
        //layer->set_blit("alpha");
        //layer->set_([sender floatValue]);
    }
}

- (void) setLayer:(CVLayer *)lay
{
    if (lay) {
        layer = lay;
        layer->fps.set(25);
        // set alpha
        [self setAlpha:alphaBar];
    } 
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

    // to prevent its run() method to try rendering
    // a frame while we change filter parameters
    if (layer) 
        layer->lock();
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
        for (int i = 0; i < 4; i++) {
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
    if (layer)
        layer->unlock();
    //[lock unlock];
    [pool release];
}

- (IBAction)setBlendMode:(id)sender
{
    if (layer) {
        switch([sender tag])
        {
            case 0:
                NSString *blendMode = [[NSString alloc] initWithFormat:@"CI%@BlendMode", [[sender selectedItem] title]];
                layer->blendMode = blendMode;
                break;
            default:
                break;
        }
    }
}

- (void)setFilterCenterFromMouseLocation:(NSPoint)where
{
    CIVector    *centerVector = nil;
    
    //[lock lock];
    
    centerVector = [CIVector vectorWithX:where.x Y:where.y];
    [effectFilter setValue:centerVector forKey:@"inputCenter"];
    
    //[lock unlock];
    if(!CVDisplayLinkIsRunning(displayLink))
        [self display];
}

- (void)renderPreview
{
    Context *ctx = (Context *)[freej getContext];
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if (doPreview && previewTarget) { 
        // scale the frame to fit the preview
        [previewTarget renderFrame:[self getTexture]];

    }
    [pool release];
}


- (CVTexture *)getTexture
{
    CIImage     *inputImage = nil;
    CVTexture   *texture = nil;
    //NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CIImage     *renderedImage = nil;

    [lock lock];
    if (newFrame) {        
        inputImage = [CIImage imageWithCVImageBuffer:currentFrame];
        if (doFilters) {    
            [colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
            [exposureAdjustFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
            [effectFilter setValue:[exposureAdjustFilter valueForKey:@"outputImage"] 
                          forKey:@"inputImage"];
            [alphaFilter  setValue:[effectFilter valueForKey:@"outputImage"]
                          forKey:@"inputImage"];
            [rotateFilter setValue:[alphaFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
            //[translateFilter setValue:[rotateFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
            renderedImage = [rotateFilter valueForKey:@"outputImage"];
        } else {
            renderedImage = inputImage;
        }
        texture = [[CVTexture alloc] initWithCIImage:renderedImage pixelBuffer:currentFrame];
        if (lastFrame)
            [lastFrame release];
        lastFrame = texture;
        [self task]; // notify we have a new frame and the qtvisualcontext can be tasked
        newFrame = NO;

    } 
    
    texture = [lastFrame retain];
    [lock unlock];

    return texture;
}

- (void)mouseDown:(NSEvent *)theEvent {
    [filterPanel setLayer:self];
    [filterPanel show];
    [super mouseDown:theEvent];
}

- (bool)isOpaque
{
    return YES;
}

- (bool)needPreview
{
    return doPreview;
}

- (void)startPreview
{  
    doPreview = YES;
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

- (void)windowChangedScreen:(NSNotification*)inNotification
{
    NSWindow *window = [inNotification object]; 
    CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];
    if(displayID && (viewDisplayID != displayID))
    {
        CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, 
            (CGLContextObj)[[self openGLContext] CGLContextObj], 
            (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]);
        viewDisplayID = displayID;
    }
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
        return layer->active?YES:NO;
    return NO;
}

- (void)activate
{
    if (layer)
        layer->activate();
}

- (void)deactivate
{
    if (layer)
        layer->deactivate();
}

@synthesize layer;

@end

CVLayer::CVLayer(NSObject *vin) : Layer()
{
    input = vin;
    bufsize = 0;
    blendMode = NULL;
    type = Layer::GL_COCOA;
    data = (void *)this;
    [input setLayer:this];
    set_name([[(NSView *)input toolTip] UTF8String]);
    start();
}

CVLayer::~CVLayer()
{
    stop();
    lock();
    unlock();
    close();
    deactivate();
    [input togglePlay:nil];
    [input setLayer:nil];
}

/*
void 
CVLayer::run()
{
    feed();
    [input renderPreview];
}
*/

void
CVLayer::activate()
{
    freej->add_layer(this);
    active = true;
    notice("Activating %s", name);
    //blitter.set_blit("RGB");
    //blitter.current_blit->lay
}

void
CVLayer::deactivate()
{
    freej->rem_layer(this);
    active = false;
}

bool
CVLayer::open(const char *path)
{
    //[input openFile: path];
    return false;
}

bool
CVLayer::init(Context *ctx)
{
     // * TODO - probe resolution of the default input device
    return init(ctx, ctx->screen->w, ctx->screen->h);
}

bool
CVLayer::init(Context *ctx, int w, int h)
{
    width = w;
    height = h;
    freej = ctx;
    _init(width,height);
    return true;
}

void *
CVLayer::feed()
{
    if (active || [input needPreview]) {
        [input _renderTime:nil];
    }
    return (void *)buffer;
}

void
CVLayer::close()
{
    //[input release];
}

bool
CVLayer::forward()
{
    [input stepForward];
    return true;
}

bool
CVLayer::backward()
{
    [input stepBackward];
    return true;
}

bool
CVLayer::backward_one_keyframe()
{
    return backward();
}

bool
CVLayer::set_mark_in()
{
    return false;
}

bool
CVLayer::set_mark_out()
{
    return false;
}

void
CVLayer::pause()
{
    [input stop];
}

bool
CVLayer::relative_seek(double increment)
{
    return false;
}

// accessor to get a texture from the CVLayerView cocoa class
CVTexture * 
CVLayer::gl_texture()
{
    return [input getTexture];
}
