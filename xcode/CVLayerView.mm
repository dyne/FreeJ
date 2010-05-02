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

#import <CVLayerView.h>
#import <CIAlphaFade.h>
#import <CVFilterPanel.h>
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
    posterImage = nil;
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
#if MAC_OS_X_VERSION_10_6
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
                                      pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
									   colorSpace:colorSpace
										  options:nil] retain];
#else
	// DEPRECATED in 10.6
    ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[[self openGLContext] retain] CGLContextObj]
                                      pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
                                          options:[NSDictionary dictionaryWithObjectsAndKeys:
                                                   (id)colorSpace,kCIContextOutputColorSpace,
                                                   (id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
#endif
	CGColorSpaceRelease(colorSpace);
    [lock lock];
    [layerController setContext:freej];
    [lock unlock];
    GLint params[] = { 1 };
    CGLSetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, params );
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
            CGPoint origin;
            origin.x = (frame.size.width-[posterImage extent].size.width)/2;
            origin.y = (frame.size.height-[posterImage extent].size.height)/2;
            [[self openGLContext] makeCurrentContext];
            [ciContext drawImage: posterImage
                         atPoint: origin
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
    // XXX - sender is assumed to be a button (or checkbox)
    if (layerController)
        if ([sender state] == NSOnState)
            [layerController activate];
        else
            [layerController deactivate];
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
    if (layerController)
        [layerController renderPreview];
}


- (void)mouseDown:(NSEvent *)theEvent {
    [filterPanel setLayer:layerController];
    [filterPanel show];
    [super mouseDown:theEvent];
}

- (BOOL)isOpaque
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

- (NSString *)blendMode 
{
    return [layerController blendMode];
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
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];

    if (!image) {
        if (posterImage) [posterImage release];
            posterImage = nil;
        [self setNeedsDisplay:YES];
        [pool release];
        return;
    }
    NSRect bounds = [self bounds];
    NSRect frame = [self frame];
    NSData  * tiffData = [image TIFFRepresentation];
    NSBitmapImageRep * bitmap;
    bitmap = [NSBitmapImageRep imageRepWithData:tiffData];
    // scale the frame to fit the preview
    NSAffineTransform *scaleTransform = [NSAffineTransform transform];
    float xScaleFactor = frame.size.width/[image size].width;
    float yScaleFactor = frame.size.height/[image size].height;
    [scaleTransform scaleBy:(xScaleFactor < yScaleFactor)?xScaleFactor:yScaleFactor];
    
    [scaleFilter setValue:scaleTransform forKey:@"inputTransform"];
    CIImage *inputImage = [[CIImage alloc] initWithBitmapImageRep:bitmap];
    [scaleFilter setValue:inputImage forKey:@"inputImage"];
    posterImage = [[scaleFilter valueForKey:@"outputImage"] retain];
    [inputImage release];
    [self setNeedsDisplay:YES];
    [pool release];
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
