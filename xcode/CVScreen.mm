/*  FreeJ
 *  (c) Copyright 2009 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <config.h>

#include <stdlib.h>

#include <jutils.h>
#define __cocoa
#import <QuartzCore/CIKernel.h>
#import "CFreej.h"
#import "CVLayer.h"
#import "CVScreen.h"

static CVReturn renderCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
	static uint64_t save = 0;
	if (inNow->videoTime >= save + inNow->videoRefreshPeriod) {
		save = inNow->videoTime;
		return [(CVScreenView*)displayLinkContext outputFrame];
	}
	return kCVReturnError;
}


@implementation CVScreenView : NSOpenGLView

- (void)windowChangedScreen:(NSNotification*)inNotification
{
    NSWindow *window = [inNotification object]; 
    CGDirectDisplayID displayID = (CGDirectDisplayID)[[[[window screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];

    if(displayID && (viewDisplayID != displayID))
    {
		CVDisplayLinkSetCurrentCGDisplay(displayLink, displayID);
		viewDisplayID = displayID;
    }
}

- (void)windowChangedSize:(NSNotification*)inNotification
{
	NSRect frame = [self frame];
	[self setSizeWidth:frame.size.width Height:frame.size.height];	
}

- (void)awakeFromNib
{
	[self init];
}

- (void)start
{

	Context *ctx = [freej getContext];
	CVScreen *screen = (CVScreen *)ctx->screen;
	screen->set_view(self);
	CVDisplayLinkStart(displayLink);
	
}

- (id)init
{
	needsReshape = YES;
	//cafudding = NO;
	[freej start];
	Context *ctx = (Context *)[freej getContext];
	fjScreen = (CVScreen *)ctx->screen;
	//CVPixelBufferCreate(NULL, 640, 480, k32ARGBPixelFormat , NULL, &pixelBuffer);
	CVReturn err = CVOpenGLBufferCreate (NULL, 400, 300, NULL, &pixelBuffer);
	if (err) {
		// TODO - Error messages
		return nil;
	}
	CVPixelBufferRetain(pixelBuffer);
	return self;
}

- (void)update
{
	@synchronized (self) {
		[super update];
	}
}

- (void)dealloc
{
	CVOpenGLTextureRelease(pixelBuffer);
	[ciContext release];
	[fullScreenContext release];
	[standardContext release];
	[outFrame release];
	[inFrame release];
}
- (void)prepareOpenGL
{
	CVReturn			    ret;
		
	lock = [[NSRecursiveLock alloc] init];
	  	    		
	// Create display link 
	CGOpenGLDisplayMask	totalDisplayMask = 0;
	int			virtualScreen;
	GLint		displayMask;
	NSOpenGLPixelFormat	*openGLPixelFormat = [self pixelFormat];
	// we start with our view on the main display
	// build up list of displays from OpenGL's pixel format
	viewDisplayID = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  
	for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
	{
		[openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	//ret = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	ret = CVDisplayLinkCreateWithCGDisplay(viewDisplayID, &displayLink);
	

	func("Setting up full-screen context.");
	
	 NSOpenGLPixelFormatAttribute fullScreenAttributes[] =
         {
             NSOpenGLPFADoubleBuffer,
             NSOpenGLPFAFullScreen,
             NSOpenGLPFAAccelerated,
             NSOpenGLPFAColorSize, [[[NSUserDefaults 
standardUserDefaults] objectForKey:@"fullScreenColorBitDepth"] intValue],
             NSOpenGLPFAAlphaSize, 8,
             NSOpenGLPFADepthSize, 32,
             nil
         };

	// Initialize an NSOpenGLContext for full-screen.
	NSOpenGLPixelFormat *fullScreenPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:fullScreenAttributes];
	fullScreenContext = [[NSOpenGLContext alloc] initWithFormat:fullScreenPixelFormat shareContext:nil];
	//[fullScreenPixelFormat release];
	// Make current.
	//[fullScreenContext makeCurrentContext];
	standardContext = [[self openGLContext] retain];
	currentContext = standardContext;
	
	// Create CGColorSpaceRef 
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
	// Create CIContext 
	
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[currentContext CGLContextObj]
			    pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
			    options:[NSDictionary dictionaryWithObjectsAndKeys:
				(id)colorSpace,kCIContextOutputColorSpace,
				(id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
	CGColorSpaceRelease(colorSpace);

	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedSize:) name:NSWindowDidResizeNotification object:nil];
	// Set up display link callbacks 
	CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
	
	// start asking for frames
	[self start];
}

- (void)drawRect:(NSRect)theRect
{
    NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
	[currentContext makeCurrentContext];	
	if(needsReshape)	// if the view has been resized, reset the OpenGL coordinate system
	{
		GLfloat 	minX, minY, maxX, maxY;

		minX = NSMinX(bounds);
		minY = NSMinY(bounds);
		maxX = NSMaxX(bounds);
		maxY = NSMaxY(bounds);

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
		needsReshape = NO;
		
		// clean the OpenGL context - not so important here but very important when you deal with transparency
		// make sure we have a frame to render    
		// flush our output to the screen - this will render with the next beamsync
	}
}

- (void)renderFrame
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
	
	CGRect	    imageRect;
	//CIImage	    *inputImage = [CIImage imageWithCVImageBuffer:pixelBuffer];//, *scaledImage;
	//@synchronized (self) {
		[self drawRect:NSZeroRect];
		// and do preview
		if (outFrame) {
			CGRect  cg = CGRectMake(NSMinX(bounds), NSMinY(bounds),
						NSWidth(bounds), NSHeight(bounds));
			//imageRect = [outFrame extent];
			[ciContext drawImage: outFrame
				atPoint: cg.origin  fromRect: cg];
		}
		glFlush();
	//}
	[pool release];
}

- (void *)getSurface
{		
	return (void *)CVPixelBufferGetBaseAddress(pixelBuffer);			
}

- (CVReturn)outputFrame
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	if (outFrame) {
		[outFrame release];
		outFrame = NULL;
	}
	//CVPixelBufferLockBaseAddress(pixelBuffer, 0);
	Context *ctx = (Context *)[freej getContext];
	//cafudding = YES;
	ctx->cafudda(0.0);
	[self renderFrame];
	//cafudding = NO;
	//CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

	[pool release];
	return kCVReturnSuccess;
}

- (void)drawLayer:(Layer *)layer
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	//@synchronized (self) {
	if (layer->type = Layer::GL_COCOA) {
		CVLayer *cvLayer = (CVLayer *)layer;
		CIImage *inputImage = cvLayer->gl_texture();
		if (inputImage && inputImage != inFrame) {
			CIFilter *blendFilter = NULL;
			if (inFrame)
				[inFrame release];
			inFrame = inputImage;
			[inputImage retain];
			//if (cvLayer->blendFilter)
			//	blendFilter = cvLayer->blendFilter;
			//else 
			blendFilter = [CIFilter filterWithName:@"CIOverlayBlendMode"]; 
			if (!outFrame) {
				outFrame = inputImage;
			} else {
				[blendFilter setValue:outFrame forKey:@"inputBackgroundImage"];
				[blendFilter setValue:inputImage forKey:@"inputImage"];
				CIImage *tmp = [blendFilter valueForKey:@"outputImage"];
				[outFrame release];
				outFrame = tmp;
			}
			[outFrame retain];
		}
	}
	//}
	[pool release];
}

- (void)setSizeWidth:(int)w Height:(int)h
{
	if (w != fjScreen->w || h != fjScreen->h) {
		@synchronized (self)
		{
			CVPixelBufferRelease(pixelBuffer);
			CVReturn err = CVOpenGLBufferCreate (NULL, fjScreen->w, fjScreen->h, NULL, &pixelBuffer);
			CVPixelBufferRetain(pixelBuffer);
			fjScreen->w = w;
			fjScreen->h = h;
			needsReshape = YES;
		}
	}
}

/*
- (IBAction)fullScreen
{
	if (currentContext != fullScreenContext) {
		@synchronized (self) {
			currentContext = fullScreenContext;
			//[self setOpenGLContext:currentContext];
			//[currentContext setView:self];
			[ciContext release];
			// Create CGColorSpaceRef 
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
			
		// Create CIContext 
		
		ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[currentContext CGLContextObj]
					pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
					options:[NSDictionary dictionaryWithObjectsAndKeys:
					(id)colorSpace,kCIContextOutputColorSpace,
					(id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
		CGColorSpaceRelease(colorSpace);
		}
	}
}
*/

/*
- (bool)isCafudding
{
	return cafudding;
}
*/
@end

CVScreen::CVScreen()
  : ViewPort() {

  //buffer = NULL;
  bpp = 32;
  view = NULL;
}

CVScreen::~CVScreen() {
  func("%s",__PRETTY_FUNCTION__);

}


bool CVScreen::init(int w, int h) {

  this->w = w;
  this->h = h;
  bpp = 32;
  size = w*h*(bpp>>3);
  pitch = w*(bpp>>3);

  // test
  //buffer = malloc(size);

  return true;
}


void *CVScreen::coords(int x, int y) {
//   func("method coords(%i,%i) invoked", x, y);
// if you are trying to get a cropped part of the layer
// use the .pitch parameter for a pre-calculated stride
// that is: number of bytes for one full line
  return
    ( x + (w*y) +
      (uint32_t*)get_surface() );
}

void *CVScreen::get_surface() {
  if (view)
	return [view getSurface];
  return NULL;
}

void CVScreen::set_view(CVScreenView *v)
{
	view = v;
}

void CVScreen::blit(Layer *lay)
{
	if (view)
		[view drawLayer:lay];
}
