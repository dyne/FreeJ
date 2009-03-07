//
//  CVGenerator.m
//  freej
//
//  Created by xant on 3/7/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

// XXX - WIP

#import "CVGenerator.h"


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
	CVReturn			    ret;
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	
	// Create CGColorSpaceRef 
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
	// Create CIContext 
	ciContext = [[CIContext contextWithCGLContext:[[self openGLContext] CGLContextObj]
			    pixelFormat:[[self pixelFormat] CGLPixelFormatObj]
			    options:nil] retain];
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
