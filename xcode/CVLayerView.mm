//
//  CVLayerView.mm
//  freej
//
//  Created by xant on 8/30/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVLayerView.h"
#import "CIAlphaFade.h"
#import "CVFilterPanel.h"
#import <QTKit/QTMovie.h>

@implementation CVLayerView : NSOpenGLView

- (void)awakeFromNib
{
    [self init];
}

- (id)init
{
    lock = [[NSRecursiveLock alloc] init];
    [lock retain];
    //[self setNeedsDisplay:NO];
    //layer = NULL;
    //layerController = NULL;
    return self;
}

- (void)dealloc
{
    if (ciContext)
        [ciContext release];

    [layerController release];
    [super dealloc];
}

- (void)prepareOpenGL
{
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    
    scaleFilter = [[CIFilter filterWithName:@"CIAffineTransform"] retain];
    //CIFilter *scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
    [scaleFilter setDefaults];    // set the filter to its default values

    
    // Create CGColorSpaceRef 
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    
    // Create CIContext 
    
    ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[[self openGLContext] retain] CGLContextObj]
                                      pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
                                          options:[NSDictionary dictionaryWithObjectsAndKeys:
                                                   (id)colorSpace,kCIContextOutputColorSpace,
                                                   (id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];

	CGColorSpaceRelease(colorSpace);
    /*
     // build up list of displays from OpenGL's pixel format
     for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
     {
     [openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
     totalDisplayMask |= displayMask;
     }
     */
    // Setup the timecode overlay
    /*
     NSDictionary *fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
     [NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
     nil];
     */
    //timeCodeOverlay = [[TimeCodeOverlay alloc] initWithAttributes:fontAttributes targetSize:NSMakeSize(720.0,480.0 / 4.0)];    // text overlay will go in the bottom quarter of the display
    [lock lock];
    NSLog(@"%@\n",[layerController class]);
    [layerController initWithOpenGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj] 
                               pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj] Context:freej];
    [lock unlock];
    GLint params[] = { 1 };
    CGLSetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, params );
    //[self drawRect:NSZeroRect];
    needsReshape = YES;
    [self setNeedsDisplay:YES];
    [pool release];
}

- (void)drawRect:(NSRect)theRect
{
    NSRect bounds = [self bounds];
    NSRect frame = [self frame];
    CGRect imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
                                  NSWidth(bounds), NSHeight(bounds));
    GLint zeroOpacity = 0;
    if (needsReshape) {
        
        if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
            return;
        [[self openGLContext] makeCurrentContext];
        [[self openGLContext] setValues:&zeroOpacity forParameter:NSOpenGLCPSurfaceOpacity];
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(NSMinX(bounds), NSWidth(bounds), NSMinY(bounds), NSHeight(bounds), -1.0, 1.0);
        
        glDisable(GL_DITHER);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_FOG);
        //glDisable(GL_TEXTURE_2D);
        glDisable(GL_DEPTH_TEST);
        glPixelZoom(1.0,1.0);
        
        // clean the OpenGL context 
        glClearColor(0.0, 0.0, 0.0, 0.0);         
        glClear(GL_COLOR_BUFFER_BIT);
        [[self openGLContext] flushBuffer];

        needsReshape = NO;
    }
    if ([self needsDisplay]) {
        if (posterImage) {
            CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
            [[self openGLContext] makeCurrentContext];
            [ciContext drawImage: posterImage
                         atPoint: imageRect.origin
                        fromRect: imageRect];
            [[self openGLContext] flushBuffer];
            CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
        } else {
            CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
            [[self openGLContext] makeCurrentContext];
            [ciContext drawImage: [CIImage imageWithColor:[CIColor colorWithRed:0.0 green:0.0 blue:0.0]]
                         atPoint: imageRect.origin
                        fromRect: imageRect];
            [[self openGLContext] flushBuffer];
            CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);            
        }
        [self setNeedsDisplay:NO];
    }
}

- (void)reshape
{
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return;
    [super reshape];
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void) update
{
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return;
    [super update];
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (CVReturn)renderFrame
{
    NSLog(@"renderFrame MUST be overridden");
    return kCVReturnError;
}

/* TODO - document me */
- (void)task
{
}

- (IBAction)toggleFilters:(id)sender
{
    [layerController toggleFilters];
}

- (IBAction)toggleVisibility:(id)sender
{
    [layerController toggleVisibility];
}

- (IBAction)togglePreview:(id)sender
{
    [layerController togglePreview];
}


- (IBAction)setFilterParameter:(id)sender
{
    [layerController setFilterParameter:sender];
}

- (IBAction)setBlendMode:(id)sender
{
    [layerController setBlendMode:sender];

}


- (void)renderPreview
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    if ([layerController doPreview] && previewTarget) { 
        // scale the frame to fit the preview
        if (![previewTarget isHiddenOrHasHiddenAncestor])
            [previewTarget renderFrame:[layerController getTexture]];
        
    }
    [pool release];
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
    return [layerController doPreview];
}

- (void)startPreview
{  
    [layerController startPreview];
}


- (void)setPreviewTarget:(CVPreview *)targetView
{
    [lock lock];
    previewTarget = targetView;
    [lock unlock];
    
}

- (CVPreview *)getPreviewTarget
{
    return previewTarget;
}

- (void)stopPreview
{
    [layerController stopPreview];
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
    return [layerController isVisible];
}

- (void)activate
{
    [layerController activate];
}

- (void)deactivate
{
    [layerController deactivate];
}

- (void)rotateBy:(float)deg
{
    [layerController rotateBy:deg];
}

- (void)translateXby:(float)x Yby:(float)y
{
    [layerController translateXby:x Yby:y];
}

- (void)clear {
    NSRect        frame = [self frame];

    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]))
    return;
    posterImage = nil;

    [[self openGLContext] makeCurrentContext];    
    // clean the OpenGL context
    glClearColor(0.0, 0.0, 0.0, 0.0);         
    glClear(GL_COLOR_BUFFER_BIT);
    glFinish();

    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void)setPosterImage:(NSImage *)image
{
    NSRect bounds = [self bounds];
    NSRect frame = [self frame];
    NSData  * tiffData = [image TIFFRepresentation];
    NSBitmapImageRep * bitmap;
    bitmap = [NSBitmapImageRep imageRepWithData:tiffData];
    // scale the frame to fit the preview
    NSAffineTransform *scaleTransform = [NSAffineTransform transform];
    float scaleFactor = frame.size.height/[image size].height;
    [scaleTransform scaleBy:scaleFactor];
    
    [scaleFilter setValue:scaleTransform forKey:@"inputTransform"];
    [scaleFilter setValue:[[CIImage alloc] initWithBitmapImageRep:bitmap] forKey:@"inputImage"];
    
    posterImage = [[scaleFilter valueForKey:@"outputImage"] retain];
    [self setNeedsDisplay:YES];
}

- (CVFilterPanel *)filterPanel
{
    return filterPanel;
}

- (NSString *)filterName {
    if (layerController)
        return [layerController filterName];
    return nil;
}

- (NSDictionary *)filterParams {
    if (layerController)
        return [layerController filterParams];
    return nil;

}

@end
