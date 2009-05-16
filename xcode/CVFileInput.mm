//
//  CVFileInput.mm
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CIAlphaFade.h"
#import "CVFileInput.h"
#include <math.h>

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

        return [(CVFileInput*)displayLinkContext _renderTime:inOutputTime];
}

@implementation CVFileInput : NSOpenGLView

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

- (void)prepareOpenGL
{
    CVReturn                ret;
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
    CGOpenGLDisplayMask    totalDisplayMask = 0;
    int            virtualScreen;
    GLint        displayMask;
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
    CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
        
    // Setup the timecode overlay
    /*
    NSDictionary *fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
        [NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
        nil];
    */
    //timeCodeOverlay = [[TimeCodeOverlay alloc] initWithAttributes:fontAttributes targetSize:NSMakeSize(720.0,480.0 / 4.0)];    // text overlay will go in the bottom quarter of the display
    [pool release];
}

- (void)awakeFromNib
{
    [self init];
}

- (id)init
{
    [self setNeedsDisplay:NO];
    needsReshape = YES;
    //freejFrame = NULL;
    doFilters = true;
    currentFrame = NULL;
    lastFrame = NULL;
    renderedImage = NULL;
    filterPanel = NULL;
    if (CVOpenGLBufferCreate (NULL, 400, 300, NULL, &pixelBuffer) != noErr) {
        // TODO - error messages
        pixelBuffer = NULL;
    }
    doPreview = YES;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
    paramNames = [[NSMutableArray arrayWithCapacity:4] retain];
    lock = [[NSRecursiveLock alloc] init];
    [lock retain];
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (qtMovie)
        [qtMovie release];
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
    if(qtVisualContext)
        QTVisualContextRelease(qtVisualContext);
    [ciContext release];
    if (pixelBuffer)
        CVOpenGLBufferRelease(pixelBuffer);
    if (paramNames)
            [paramNames release];
    [lock release];
    [super dealloc];
}

- (void)update
{

    [lock lock];
    [super update];
    [lock unlock];
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

- (void)unloadMovie
{
    NSRect        frame = [self frame];
    [lock lock];
    //delete layer;
    [qtMovie stop];
    [qtMovie release];
    qtMovie = NULL;
    //SetMovieVisualContext([qtMovie quickTimeMovie], NULL);
    [renderedImage release];
    renderedImage = NULL;
    //[previewImage release];
    previewImage = NULL;
    QTVisualContextRelease(qtVisualContext);
    qtVisualContext = NULL;
    CVOpenGLTextureRelease(currentFrame);

    needsReshape = YES;
    //[QTMovie exitQTKitOnThread];
    [[self openGLContext] makeCurrentContext];    
    // clean the OpenGL context
    glClearColor(0.0, 0.0, 0.0, 0.0);         
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();
    [lock unlock];
}

- (void)setQTMovie:(QTMovie*)inMovie
{    
    OSStatus                err;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Context *ctx = (Context *)[freej getContext];

    //[lock lock];
    // if we own already a movie let's relase it before trying to open the new one
    if (qtMovie) {
        [self unloadMovie];
    }
    // no movie has been supplied... perhaps we are going to exit
    if (!inMovie) {
        //[lock unlock];
        return;
    }
    qtMovie = inMovie;
    [qtMovie retain]; // we are going to need this for a while
    if (!qtVisualContext)
    {
        /* Create QT Visual context */

        // Pixel Buffer attributes
        CFMutableDictionaryRef pixelBufferOptions = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);

        // the pixel format we want (freej require BGRA pixel format
        SetNumberValue(pixelBufferOptions, kCVPixelBufferPixelFormatTypeKey, k32ARGBPixelFormat);

        // size
        SetNumberValue(pixelBufferOptions, kCVPixelBufferWidthKey, ctx->screen->w);
        SetNumberValue(pixelBufferOptions, kCVPixelBufferHeightKey, ctx->screen->h);

        // alignment
        SetNumberValue(pixelBufferOptions, kCVPixelBufferBytesPerRowAlignmentKey, 1);
        // QT Visual Context attributes
        CFMutableDictionaryRef visualContextOptions = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                                         &kCFTypeDictionaryKeyCallBacks,
                                                         &kCFTypeDictionaryValueCallBacks);
        // set the pixel buffer attributes for the visual context
        CFDictionarySetValue(visualContextOptions,
                             kQTVisualContextPixelBufferAttributesKey,
                             pixelBufferOptions);

        err = QTOpenGLTextureContextCreate(kCFAllocatorDefault, (CGLContextObj)[[self openGLContext] CGLContextObj],
            (CGLPixelFormatObj)[[NSOpenGLView defaultPixelFormat] CGLPixelFormatObj], visualContextOptions, &qtVisualContext);
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        
        CGColorSpaceRelease(colorSpace);
        
    }
    if(qtMovie) { // ok the movie is here ... let's start the underlying QTMovie object
        OSStatus error;
        
        error = SetMovieVisualContext([qtMovie quickTimeMovie], qtVisualContext);
        [qtMovie gotoBeginning];
        [qtMovie setMuted:YES]; // still no audio?
        [qtMovie setIdling:NO];

        movieDuration = [[[qtMovie movieAttributes] objectForKey:QTMovieDurationAttribute] QTTimeValue];
        // register the layer within the freej context
        if (!layer) {
            layer = new CVLayer((NSObject *)self);
            layer->init(ctx);
            
            layer->buffer = (void *)pixelBuffer; // give freej a fake buffer ... that's not going to be used anyway
            //layer->start();
        }
        [self toggleVisibility:nil];
    }

    //[lock unlock];
    [pool release];
}

- (QTTime)currentTime
{
    return [qtMovie currentTime];
}

- (QTTime)movieDuration
{
    return movieDuration;
}

- (void)setTime:(QTTime)inTime
{
   // [qtMovie setCurrentTime:inTime];
  //  if(CVDisplayLinkIsRunning(displayLink))
        //[self togglePlay:nil];
    //[self updateCurrentFrame];
   // [self display];
}

- (IBAction)setMovieTime:(id)sender
{
    [self setTime:QTTimeFromString([sender stringValue])];
}

- (IBAction)togglePlay:(id)sender
{
    //[lock lock];
    if(CVDisplayLinkIsRunning(displayLink))
    {
        CVDisplayLinkStop(displayLink);
        [qtMovie stop];
    } else {
        [qtMovie play];
        CVDisplayLinkStart(displayLink);
    }
    //[lock unlock];
}

- (IBAction    )togglePreview:(id)sender
{
    doPreview = [previewButton intValue]?YES:NO;
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
    [lock unlock];
    [pool release];
}

- (IBAction)setBlendMode:(id)sender
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
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
    [pool release];
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
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    NSRect        frame = [self frame];
    NSRect        bounds = [self bounds];
    float        scaleFactor;
    NSRect vv = [self visibleRect];
    if(!NSIsEmptyRect([self visibleRect]))
    {
        Context *ctx = (Context *)[freej getContext];
        CIImage        *previewInputImage = [self getTexture];
        
        // scale the frame to fit the preview
        NSAffineTransform *scaleTransform = [NSAffineTransform transform];
        scaleFactor = frame.size.width/ctx->screen->w;
        [scaleTransform scaleBy:scaleFactor];
    
        [scaleFilter setValue:scaleTransform forKey:@"inputTransform"];
        [scaleFilter setValue:previewInputImage forKey:@"inputImage"];

        previewImage = [scaleFilter valueForKey:@"outputImage"];
        
        NSRect frame = [self frame];
        NSRect	bounds = [self bounds];
        
        // output the downscaled frame in the preview window
        // XXX - I'm not sure we really need locking here
        [lock lock];
        [[self openGLContext] makeCurrentContext];

       

        CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
					NSWidth(bounds), NSHeight(bounds));
        [ciContext drawImage:previewImage
                atPoint: imageRect.origin
                fromRect: imageRect];
        [[self openGLContext] flushBuffer];
        [previewInputImage release];
        [lock unlock];
        
    }
    [pool release];
}

- (BOOL)getFrameForTime:(const CVTimeStamp *)timeStamp
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BOOL rv = NO;
    [lock lock];
    if(QTVisualContextIsNewImageAvailable(qtVisualContext, NULL))
    {        
        if (currentFrame) {
            CVOpenGLTextureRelease(currentFrame);
            currentFrame = NULL; 
        }
        QTVisualContextCopyImageForTime(qtVisualContext,
        NULL,
        NULL,
        &currentFrame);
/*      
        // rendering (aka: applying filters) is now done in getTexture()
        [lock lock];
        if (renderedImage) {
            [renderedImage release];
            renderedImage = NULL;
        } 
        [lock unlock];
*/
        rv = YES;
        
    } else {
        MoviesTask([qtMovie quickTimeMovie], 1000);
    }
    [lock unlock];
    layer->fps.calc();
    layer->fps.delay();
    [pool release];
    return rv;
}

- (void)updateCurrentFrame
{
    [self getFrameForTime:nil];    
}

- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp
{
    CVReturn rv = kCVReturnError;
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];

    if(qtMovie && [self getFrameForTime:timeStamp])
    {
        if (doPreview) {
          //  if (timeStamp->videoTime > lastRenderedTime + timeStamp->videoTimeScale/25) {
                lastRenderedTime = timeStamp->videoTime;
                [self renderPreview];
           // }
        }
        // make sure we have a frame to render    
        // render the frame
        rv = kCVReturnSuccess;
    } else {
        rv = kCVReturnError;
    }
    MoviesTask([qtMovie quickTimeMovie], 0);
    QTVisualContextTask(qtVisualContext);
    [pool release];
    return rv;
}

- (bool)stepBackward
{
    [qtMovie stepBackward];
    return true;
}

- (bool)setpForward
{
    [qtMovie stepForward];
    return true;
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
    layer = lay;
    layer->fps.set(25);
    if (lay) {
        // set alpha
        [self setAlpha:alphaBar];
    } else {
        [self setQTMovie:nil];
    }
}

- (IBAction)openFile:(id)sender
{
     func("doOpen");    
     NSOpenPanel *tvarNSOpenPanelObj    = [NSOpenPanel openPanel];
     NSInteger tvarNSInteger    = [tvarNSOpenPanelObj runModalForTypes:nil];
     if(tvarNSInteger == NSOKButton){
         func("openScript we have an OK button");    
     } else if(tvarNSInteger == NSCancelButton) {
         func("openScript we have a Cancel button");
         return;
     } else {
         error("doOpen tvarInt not equal 1 or zero = %3d",tvarNSInteger);
         return;
     } // end if     

     NSString * tvarDirectory = [tvarNSOpenPanelObj directory];
     func("openScript directory = %@",tvarDirectory);

     NSString * tvarFilename = [tvarNSOpenPanelObj filename];
     func("openScript filename = %@",tvarFilename);
 
    if (tvarFilename) {
        if(CVDisplayLinkIsRunning(displayLink)) 
            [self togglePlay:nil];
    
        QTMovie *movie = [[QTMovie alloc] initWithFile:tvarFilename error:nil];
        [self setQTMovie:movie];
        [movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
        //[movie gotoBeginning];
        [self togglePlay:nil];

    }
}

- (IBAction)toggleFilters:(id)sender
{
    doFilters = doFilters?false:true;
}

- (IBAction)toggleVisibility:(id)sender
{
    if (layer)
        if ([showButton state] == NSOnState) {
            if (!layer->active)
                layer->activate();
        } else if (layer->active) {
            layer->deactivate();
        }
}

- (CIImage *)getTexture
{
    CIImage     *inputImage = NULL;

    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    [lock lock];
    if (currentFrame != lastFrame) {
        lastFrame = currentFrame;
        if (renderedImage)
            [renderedImage release];
        inputImage = [CIImage imageWithCVImageBuffer:currentFrame];
        
        if (inputImage) {
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
                                
                renderedImage = [renderedImage retain];
            } else {
                renderedImage = [inputImage retain];
            }
        }
    } 
    [lock unlock];
    [pool release];
    return [renderedImage retain];
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (!filterPanel) {
        filterPanel = [[CVFilterPanel alloc] initWithName:[self toolTip]];
        [filterPanel setLayer:self];
    }
    [filterPanel show];
    [super mouseDown:theEvent];
}

- (bool)isOpaque
{
    return YES;
}

@synthesize layer;
@end
