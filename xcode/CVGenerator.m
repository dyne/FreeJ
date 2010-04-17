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

// XXX - WIP

#import <CVGenerator.h>


@implementation CVGenerator

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
      

    }
    return self;
}
- (void)dealloc
{
	if (renderTimer)
		[renderTimer release];
	[lock release];
	[super dealloc];
}

- (CIImage *)getTexture
{
	[lock lock];
	if (currentImage)
		freejImage = [currentImage retain];
	[lock unlock];
	return freejImage; 
}

- (void)prepareOpenGL
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	
	// Create CGColorSpaceRef 
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
	// Create CIContext
#if MAC_OS_X_VERSION_10_6
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
                                      pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
									   colorSpace:colorSpace
										  options:nil] retain];
#else
	// deprecated in 10.6
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[currentContext CGLContextObj]
									  pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
										  options:[NSDictionary dictionaryWithObjectsAndKeys:
										           (id)colorSpace,kCIContextOutputColorSpace,
										           (id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
#endif

	CGColorSpaceRelease(colorSpace);
	renderTimer = [[NSTimer timerWithTimeInterval:0.001   //a 1ms time interval
                                target:self
                                selector:@selector(timerFired:)
                                userInfo:nil
                                repeats:YES] retain];

    [[NSRunLoop currentRunLoop] addTimer:renderTimer 
                                forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:renderTimer 
                                forMode:NSEventTrackingRunLoopMode]; //Ensure timer fires during resize
	
	generator = [[CIFilter filterWithName:@"CIStarShineGenerator"] retain];
	[generator setValue:[[CIColor colorWithString:@"0.5 0.7 0.3 1.0"] retain] forKey:@"inputColor"];
	[generator setDefaults];
	needsReshape = YES;
	
	[pool release];
}

- (void)drawRect:(NSRect)rect {
	NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
	[lock lock];
	if(needsReshape)	// if the view has been resized, reset the OpenGL coordinate system
	{
		GLfloat 	minX, minY, maxX, maxY;
		
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
		
	}

	if (currentImage)
		[currentImage release];
	currentImage = [generator valueForKey:@"outputImage"];
	[currentImage retain];
	[ciContext drawImage:currentImage
		inRect:NSRectToCGRect([self frame])
		fromRect:[currentImage extent]];
	[[self openGLContext] flushBuffer];
	[self setNeedsDisplay:NO];
	[lock unlock];
}

// Timer callback method
- (void)timerFired:(id)sender
{
    // It is good practice in a Cocoa application to allow the system to send the -drawRect:
    // message when it needs to draw, and not to invoke it directly from the timer. 
    // All we do here is tell the display it needs a refresh
    [self setNeedsDisplay:YES];
}
@end
