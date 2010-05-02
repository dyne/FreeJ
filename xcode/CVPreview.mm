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

#import <CVPreview.h>
#include <QuartzCore/QuartzCore.h>
#include <OpenGL/OpenGL.h>

@implementation CVPreview

- (id)init {
    [self setNeedsDisplay:NO];

    // Initialization code here.
    lock = [[NSRecursiveLock alloc] init];
    needsReshape = YES;
    texture = nil;
    [lock retain];
    return self;
}

- (void)awakeFromNib
{
    [self init];
}

- (void)dealloc
{
    [lock release];
    [super dealloc];
}

- (void)drawRect:(NSRect)theRect
{
    NSRect frame = [self frame];
    NSRect bounds = [self bounds];

    @synchronized(self) {
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
            return;
    [[self openGLContext] makeCurrentContext];

    if(needsReshape)    // if the view has been resized, reset the OpenGL coordinate system
    {
        GLfloat minX, minY, maxX, maxY;

        minX = NSMinX(bounds);
        minY = NSMinY(bounds);
        maxX = NSMaxX(bounds);
        maxY = NSMaxY(bounds);

        //[self update]; 

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

        needsReshape = NO;
    }
    // flush our output to the screen - this will render with the next beamsync
    //glFlush();
    [[self openGLContext] flushBuffer];
    //[super drawRect:theRect];
    [self setNeedsDisplay:NO];    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    }
}

- (void)update
{
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
            return;
    [super update];
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void)prepareOpenGL
{
    NSAutoreleasePool *pool;
    pool = [[NSAutoreleasePool alloc] init];
    // Create CGColorSpaceRef 
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	// Create CIContext 

#if MAC_OS_X_VERSION_10_6
	// Create CIContext
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
                                      pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
									   colorSpace:colorSpace
										  options:nil] retain];
#else
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
									  pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
										  options:[NSDictionary dictionaryWithObjectsAndKeys:
												   (id)colorSpace,kCIContextOutputColorSpace,
												   (id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
#endif
    CGColorSpaceRelease(colorSpace);
    [pool release];
}

- (void)renderFrame:(CVTexture *)image
{
    NSRect        frame = [self frame];
    NSRect        bounds = [self bounds];
    float         scaleFactor;
    //CVTexture   *textureToRelease = nil;
    Context       *ctx = (Context *)[freej getContext];
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
   // if (texture)
   //     textureToRelease = texture;
    texture = image;
    @synchronized(self) {
    NSAffineTransform *scaleTransform = [NSAffineTransform transform];

    scaleFactor = frame.size.height/ctx->screen->geo.h;
    [scaleTransform scaleBy:scaleFactor];
    CIFilter    *scaleFilter = [CIFilter filterWithName:@"CIAffineTransform"];
    [scaleFilter setDefaults];    // set the filter to its default values

    [scaleFilter setValue:scaleTransform forKey:@"inputTransform"];
    [scaleFilter setValue:[texture image] forKey:@"inputImage"];

    CIImage *previewImage = [scaleFilter valueForKey:@"outputImage"];
    // output the downscaled frame in the preview window
    // XXX - I'm not sure we really need locking here

    CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
                NSWidth(bounds), NSHeight(bounds));
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return;

    CGPoint origin;

    origin.x = (frame.size.width-(ctx->screen->geo.w * scaleFactor))/2;
    origin.y = (frame.size.height-(ctx->screen->geo.h * scaleFactor))/2;
    [ciContext drawImage:previewImage
            atPoint: origin
            fromRect: imageRect];
    //[self drawRect:NSZeroRect]; 
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);    

    [self setNeedsDisplay:YES];
    //if (textureToRelease)
    //    [textureToRelease release];
    }
    [texture release];

    [pool release];
}

- (void)clear
{
    needsReshape = YES;
    [self drawRect:NSZeroRect];
}

@end
