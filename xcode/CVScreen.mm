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
#import <QTKit/QTMovie.h>
#import "CFreej.h"
#import "CVLayer.h"
#import "CVScreen.h"

#define _BGRA2ARGB(__buf, __size) \
	{\
		long *__bgra = (long *)__buf;\
		for (int __i = 0; __i < __size; __i++)\
			__bgra[__i] = ntohl(__bgra[__i]);\
	}

static CVReturn renderCallback(CVDisplayLinkRef displayLink, 
                                                const CVTimeStamp *inNow, 
                                                const CVTimeStamp *inOutputTime, 
                                                CVOptionFlags flagsIn, 
                                                CVOptionFlags *flagsOut, 
                                                void *displayLinkContext)
{
	//static uint64_t save = 0;
	//if (inNow->videoTime >= save + inNow->videoRefreshPeriod) {
	//	save = inNow->videoTime;
		return [(CVScreenView*)displayLinkContext outputFrame];
	//}
	return kCVReturnSuccess;
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
//	NSRect frame = [self frame];
//	[self setSizeWidth:frame.size.width Height:frame.size.height];	
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
	lock = [[NSRecursiveLock alloc] init];
	[lock retain];
	//cafudding = NO;
	[freej start];
	Context *ctx = (Context *)[freej getContext];
	fjScreen = (CVScreen *)ctx->screen;
	//CVPixelBufferCreate(NULL, 640, 480, k32ARGBPixelFormat , NULL, &pixelBuffer);
	CVReturn err = CVOpenGLBufferCreate (NULL, ctx->screen->w, ctx->screen->h, NULL, &pixelBuffer);
	if (err) {
		// TODO - Error messages
		return nil;
	}
	CVPixelBufferRetain(pixelBuffer);
	return self;
}

- (void)update
{
	//[lock lock];
	[super update];
	//[lock unlock];
}

- (void)dealloc
{
	CVOpenGLTextureRelease(pixelBuffer);
	[ciContext release];
	[currentContext release];
	[outFrame release];
	[inFrame release];
	[lock release];
	[super dealloc];
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
	ret = CVDisplayLinkCreateWithCGDisplay(viewDisplayID, &displayLink);
	
	currentContext = [[self openGLContext] retain];
	
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
	//[lock lock];	
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
		
		glDisable(GL_DITHER);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_BLEND);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_FOG);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_DEPTH_TEST);
		glPixelZoom(1.0,1.0);
		
		// clean the OpenGL context 
		glClearColor(0.0, 0.0, 0.0, 0.0);	     
		glClear(GL_COLOR_BUFFER_BIT);

		needsReshape = NO;		
	}
	//[lock unlock];
}

- (void)renderFrame
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
	
	[self drawRect:NSZeroRect];
	//[lock lock];
	if (outFrame) {
		CGRect  cg = CGRectMake(NSMinX(bounds), NSMinY(bounds),
					NSWidth(bounds), NSHeight(bounds));
		[ciContext drawImage: outFrame
			atPoint: cg.origin  fromRect: cg];
	}
	// flush our output to the screen - this will render with the next beamsync
	//glFlush();
	[[self openGLContext] flushBuffer];
	//[lock unlock];
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
	Context *ctx = (Context *)[freej getContext];

	ctx->cafudda(0.0);
	[self renderFrame];
	[pool release];
	return kCVReturnSuccess;
}

- (void)drawLayer:(Layer *)layer
{
	NSAutoreleasePool *pool;
	CIImage *inputImage = NULL;
	CIFilter *blendFilter = NULL;
	pool = [[NSAutoreleasePool alloc] init];
	//[lock lock];
	if (layer->type == Layer::GL_COCOA) {
		CVLayer *cvLayer = (CVLayer *)layer;
		inputImage = cvLayer->gl_texture();
		NSString *blendMode = ((CVLayer *)layer)->blendMode;
		if (blendMode)
			blendFilter = [CIFilter filterWithName:blendMode];
		else
			blendFilter = [CIFilter filterWithName:@"CIOverlayBlendMode"]; 
	} else { // freej 'unknown' layer type
	
		CVPixelBufferRef pixelBufferOut;
		_BGRA2ARGB(layer->buffer, layer->geo.w*layer->geo.h); // XXX - expensive conversion
		CVReturn cvRet = CVPixelBufferCreateWithBytes (
		   NULL,
		   layer->geo.w,
		   layer->geo.h,
		   k32ARGBPixelFormat,
		   layer->buffer,
		   layer->geo.w*4,
		   NULL,
		   NULL,
		   NULL,
		   &pixelBufferOut
		);
		if (cvRet != noErr) {
			// TODO - Error Messages
		} 
		inputImage = [CIImage imageWithCVImageBuffer:pixelBufferOut];
		CVPixelBufferRelease(pixelBufferOut);
		blendFilter = [CIFilter filterWithName:@"CIOverlayBlendMode"];
	}
	[blendFilter setDefaults];
	if (inputImage && inputImage != inFrame) {
		if (inFrame)
			[inFrame release];
		inFrame = inputImage;
		[inputImage retain];	 
		
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
	//[lock unlock];
	[pool release];
}

- (void)setSizeWidth:(int)w Height:(int)h
{
	[lock lock];
	if (w != fjScreen->w || h != fjScreen->h) {

			CVPixelBufferRelease(pixelBuffer);
			CVReturn err = CVOpenGLBufferCreate (NULL, fjScreen->w, fjScreen->h, NULL, &pixelBuffer);
			if (err != noErr) {
				// TODO - Error Messages
			}
			CVPixelBufferRetain(pixelBuffer);
			fjScreen->w = w;
			fjScreen->h = h;
			needsReshape = YES;

	}
	[lock unlock];
}

- (IBAction)toggleFullScreen:(id)sender
{
	if ([self isInFullScreenMode]) {
		[self exitFullScreenModeWithOptions:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:0], 
			NSFullScreenModeAllScreens, nil ]];

	} else {
		[self enterFullScreenMode:[[self window] screen] 
			withOptions:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:0], 
			NSFullScreenModeAllScreens, nil ]];

	}
}

@end

CVScreen::CVScreen()
  : ViewPort() {

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
