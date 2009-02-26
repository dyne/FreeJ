//
//  CVideoFile.mm
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CIAlphaFade.h"
#import "CVideoFile.h"

#import <QTKit/QTMovie.h>

#define _ARGB2BGRA(__buf, __size) \
	{\
		long *__bgra = (long *)__buf;\
		for (int __i = 0; __i < __size; __i++)\
			__bgra[__i] = ntohl(__bgra[__i]);\
	}
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
	//if (inNow->hostTime
    return [(CVideoFileInput*)displayLinkContext _renderTime:inOutputTime];
}

@implementation CVideoFileInput : NSOpenGLView

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

- (void)prepareOpenGL
{
	CVReturn			    ret;
		
	lock = [[NSRecursiveLock alloc] init];
	
	// Create CGColorSpaceRef 
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
	// Create CIContext 
	ciContext = [[CIContext contextWithCGLContext:(CGLContextObj)[[self openGLContext] CGLContextObj]
			    pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
			    options:[NSDictionary dictionaryWithObjectsAndKeys:
				(id)colorSpace,kCIContextOutputColorSpace,
				(id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
	CGColorSpaceRelease(colorSpace);
	
	// Create CIFilters used for both preview and main frame
	colorCorrectionFilter = [[CIFilter filterWithName:@"CIColorControls"] retain];	    // Color filter	
	[colorCorrectionFilter setDefaults];						    // set the filter to its default values
	effectFilter = [[CIFilter filterWithName:@"CIZoomBlur"] retain];		    // Effect filter	
	[effectFilter setDefaults];							    // set the filter to its default values
	[effectFilter setValue:[NSNumber numberWithFloat:0.0] forKey:@"inputAmount"]; // don't apply effects at startup
	compositeFilter = [[CIFilter filterWithName:@"CISourceOverCompositing"] retain];    // Composite filter
	[CIAlphaFade class];	
	alphaFilter = [[CIFilter filterWithName:@"CIAlphaFade"] retain]; // AlphaFade filter
	[alphaFilter setDefaults]; // XXX - setDefaults doesn't work properly
	[alphaFilter setValue:[NSNumber numberWithFloat:0.5] forKey:@"outputOpacity"]; // set default value
	  	    		
	// Create display link 
	CGOpenGLDisplayMask	totalDisplayMask = 0;
	int			virtualScreen;
	GLint		displayMask;
	NSOpenGLPixelFormat	*openGLPixelFormat = [self pixelFormat];
	viewDisplayID = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  // we start with our view on the main display
	// build up list of displays from OpenGL's pixel format
	for (virtualScreen = 0; virtualScreen < [openGLPixelFormat  numberOfVirtualScreens]; virtualScreen++)
	{
		[openGLPixelFormat getValues:&displayMask forAttribute:NSOpenGLPFAScreenMask forVirtualScreen:virtualScreen];
		totalDisplayMask |= displayMask;
	}
	//ret = CVDisplayLinkCreateWithOpenGLDisplayMask(totalDisplayMask, &displayLink);
	ret = CVDisplayLinkCreateWithCGDisplay(viewDisplayID, &displayLink);
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedScreen:) name:NSWindowDidMoveNotification object:nil];
	// Set up display link callbacks 
	CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
		
	// Setup the timecode overlay
	/*
	NSDictionary *fontAttributes = [[NSDictionary alloc] initWithObjectsAndKeys:[NSFont labelFontOfSize:24.0f], NSFontAttributeName,
		[NSColor colorWithCalibratedRed:1.0f green:0.2f blue:0.2f alpha:0.60f], NSForegroundColorAttributeName,
		nil];
	*/
	//timeCodeOverlay = [[TimeCodeOverlay alloc] initWithAttributes:fontAttributes targetSize:NSMakeSize(720.0,480.0 / 4.0)];	// text overlay will go in the bottom quarter of the display
	

}

- (void)awakeFromNib
{
	[self init];
}

- (id)init
{
	[lock release];
	needsReshape = YES;
	//freejFrame = NULL;
	doFilters = true;
	currentFrame = NULL;
	lastFrame = NULL;
	renderedImage = NULL;
	filterPanel = NULL;
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
    ///[timeCodeOverlay release];
    CVOpenGLTextureRelease(currentFrame);
	CVOpenGLTextureRelease(previewFrame);
    if(qtVisualContext)
		QTVisualContextRelease(qtVisualContext);
	if (previewVisualContext)
		QTVisualContextRelease(previewVisualContext);
    [ciContext release];
	if (cifjContext)
		[cifjContext release];
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
    NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];
	[lock lock];
		[[self openGLContext] makeCurrentContext];
		
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
			
			
			needsReshape = NO;
		}
		
		// clean the OpenGL context - not so important here but very important when you deal with transparency
		glClearColor(0.0, 0.0, 0.0, 0.0);	     
		glClear(GL_COLOR_BUFFER_BIT);
		// make sure we have a frame to render    
		if(!currentFrame)
			[self updateCurrentFrame];
		// render the frame
		[self renderCurrentFrame];  
		// flush our output to the screen - this will render with the next beamsync
		glFlush();

	[lock unlock];
}

- (void)unloadMovie
{
	NSRect		frame = [self frame]; 
	[lock lock];
	if(CVDisplayLinkIsRunning(displayLink))
		[self togglePlay:nil];
	
	[[self openGLContext] makeCurrentContext];	
	// clean the OpenGL context
	glClearColor(0.0, 0.0, 0.0, 0.0);	     
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	
	SetMovieVisualContext([qtMovie quickTimeMovie], NULL);
	SetMovieVisualContext([previewMovie quickTimeMovie], NULL);
	[qtMovie release];
	[previewMovie release];
	[cifjContext release];
	cifjContext = NULL;
	qtMovie = NULL;
	QTVisualContextRelease(qtVisualContext);
	qtVisualContext = NULL;
	QTVisualContextRelease(previewVisualContext);
	previewVisualContext = NULL;
	needsReshape = YES;
	[lock unlock];
}

- (void)setQTMovie:(QTMovie*)inMovie
{	
	OSStatus			    err;
	Context *ctx = (Context *)[freej getContext];
	// if we own already a movie let's relase it before trying to open the new one
	if (qtMovie) 
		[self unloadMovie];
		
	// no movie has been supplied... perhaps we are going to exit
	if (!inMovie)
		return;
		
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
		err = QTPixelBufferContextCreate(kCFAllocatorDefault,
                                     visualContextOptions, &qtVisualContext);
			
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
		
		// Create CIContext to render full-frame images
		cifjContext = [[CIContext contextWithCGContext:(CGContextRef)qtVisualContext options:[NSDictionary dictionaryWithObjectsAndKeys:
				(id)colorSpace,kCIContextOutputColorSpace,
				(id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
		CGColorSpaceRelease(colorSpace);
		
	}
    if(qtMovie) { // ok the movie is here ... let's start the underlying QTMovie object
		OSStatus error;
		
		// create a slave-movie used to render preview images (quicktime is so much faster 
		// to render scaled images that it doesn't make sense to scale them ourselves
		previewMovie = [QTMovie movie];
		QTTime t0 = { 0, 0, 0 };
		QTTimeRange mDuration = { t0, [qtMovie duration] };
		[previewMovie initWithMovie:qtMovie timeRange:mDuration error:nil];
		
		[previewMovie retain];
		NSDictionary	    *attributes = nil;
		NSRect		frame = [self frame];
		NSSize size;
		// fix preview size to honor aspect ratio
		[[previewMovie attributeForKey:QTMovieNaturalSizeAttribute] getValue:&size];
		float width = frame.size.width;
		float height = frame.size.height;
		width = (height/size.height)*size.width; // (height/size.height) is the scaling factor

		// attrbutes to be used in the preview window
		attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:width], 
					  kQTVisualContextTargetDimensions_WidthKey, [NSNumber numberWithFloat:height], kQTVisualContextTargetDimensions_HeightKey, nil], 
					  kQTVisualContextTargetDimensionsKey, 
					  [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithFloat:width], kCVPixelBufferWidthKey, 
					  [NSNumber numberWithFloat:height], kCVPixelBufferHeightKey, nil], 
					  kQTVisualContextPixelBufferAttributesKey, nil];
		CGLPixelFormatObj pixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
		


		CGLContextObj glContext = (CGLContextObj)[[self openGLContext] CGLContextObj];

		err = QTOpenGLTextureContextCreate(NULL, 
				glContext,
				pixelFormat,
				(CFDictionaryRef)attributes, &previewVisualContext);

		
		error = SetMovieVisualContext([previewMovie quickTimeMovie], previewVisualContext);
		error = SetMovieVisualContext([qtMovie quickTimeMovie], qtVisualContext);
		SetMoviePlayHints([qtMovie quickTimeMovie],hintsHighQuality, hintsHighQuality);	
		[qtMovie gotoBeginning];
		[qtMovie setMuted:YES]; // still no audio?
		[previewMovie gotoBeginning];
		MoviesTask([previewMovie quickTimeMovie], 0);
		MoviesTask([qtMovie quickTimeMovie], 0);	//QTKit is not doing this automatically
		movieDuration = [[[qtMovie movieAttributes] objectForKey:QTMovieDurationAttribute] QTTimeValue];
		[self setNeedsDisplay:YES];
		// register the layer within the freej context
		layer = new CVLayer((NSObject *)self);
		layer->init(ctx);
		//layer->start();
    }
}

- (QTTime)currentTime
{
    return [qtMovie currentTime];
}


//--------------------------------------------------------------------------------------------------

- (QTTime)movieDuration
{
    return movieDuration;
}

//--------------------------------------------------------------------------------------------------

- (void)setTime:(QTTime)inTime
{
    [qtMovie setCurrentTime:inTime];
    if(CVDisplayLinkIsRunning(displayLink))
		[self togglePlay:nil];
    [self updateCurrentFrame];
    [self display];
}

//--------------------------------------------------------------------------------------------------

- (IBAction)setMovieTime:(id)sender
{
    [self setTime:QTTimeFromString([sender stringValue])];
}

- (IBAction)togglePlay:(id)sender
{
    if(CVDisplayLinkIsRunning(displayLink))
    {
		CVDisplayLinkStop(displayLink);
		[qtMovie stop];
    } else {
		[qtMovie play];
		CVDisplayLinkStart(displayLink);
    }
}

- (IBAction)setFilterParameter:(id)sender
{
    [lock lock];
	switch([sender tag])
    {
	case 0:
	    [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputContrast"];
	    break;

	case 1:
	    [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputBrightness"];
	    break;

	case 2:
	    [colorCorrectionFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputSaturation"];
	    break;
	    
	case 3:
	    [effectFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"inputAmount"];
	    break;
	case 4:
		[alphaFilter setValue:[NSNumber numberWithFloat:[sender floatValue]] forKey:@"outputOpacity"];
		break;
	default:
	    break;
	    
    }
    [lock unlock];
  //  if(!CVDisplayLinkIsRunning(displayLink))
	//	[self display];
}

- (void)setFilterCenterFromMouseLocation:(NSPoint)where
{
    CIVector	*centerVector = nil;
    
    [lock lock];
    
	centerVector = [CIVector vectorWithX:where.x Y:where.y];
    [effectFilter setValue:centerVector forKey:@"inputCenter"];
    
	[lock unlock];
    if(!CVDisplayLinkIsRunning(displayLink))
		[self display];
}

- (void)renderCurrentFrame
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	[QTMovie enterQTKitOnThread];
    NSRect		frame = [self frame];
    NSRect		bounds = [self bounds];

    if(currentFrame)
    {
		CGRect	    imageRect;
		CIImage	    *previewInputImage = [CIImage imageWithCVImageBuffer:previewFrame];//, *scaledImage;
		CIImage	    *previewImage = previewInputImage;
		

		
		// update timecode overlay
		//timecodeImage = [timeCodeOverlay getImageForTime:[self currentTime]];
		if (doFilters) {
			// preview
			[colorCorrectionFilter setValue:previewInputImage forKey:@"inputImage"];
			[effectFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] forKey:@"inputImage"];
			previewImage = [effectFilter valueForKey:@"outputImage"];
			
		} 
		// and do preview
		imageRect = [previewImage extent];
		[ciContext drawImage:previewImage
				atPoint:CGPointMake((int)((frame.size.width - imageRect.size.width) * 0.5), (int)((frame.size.height - imageRect.size.height) * 0.5))
				fromRect:imageRect];
    }
    // housekeeping on the visual context
	QTVisualContextTask(previewVisualContext);
	QTVisualContextTask(qtVisualContext);
	if (layer) {
		//layer->lock();
		layer->buffer = (void *)currentFrame;
		//layer->unlock();
	}
	[QTMovie exitQTKitOnThread];
	[pool release];
}



//--------------------------------------------------------------------------------------------------

- (BOOL)getFrameForTime:(const CVTimeStamp *)timeStamp
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    OSStatus err = noErr;
	BOOL rv = NO;
	
	[lock lock];
	//CFDataRef movieTimeData;
	[QTMovie enterQTKitOnThread];
    // See if a new frame is available
    if(QTVisualContextIsNewImageAvailable(qtVisualContext,timeStamp))
    {	    
		CVOpenGLTextureRelease(previewFrame);
		CVPixelBufferRelease(currentFrame);
		QTVisualContextCopyImageForTime(qtVisualContext,
			NULL,
			timeStamp,
			&currentFrame);
		
		//CVPixelBufferLockBaseAddress(currentFrame, 0);
		/* XXX this conversion wastes cpu time */
		//_ARGB2BGRA(CVPixelBufferGetBaseAddress(currentFrame), layer->geo.w*layer->geo.h);
		//CVPixelBufferUnlockBaseAddress(currentFrame, 0);
		// set preview moview currenttime to match master movie
		[previewMovie setCurrentTime:[qtMovie currentTime]];
		// and grab a frame to be used in the preview 
		// (quicktime will take care of resize and pixel format
		// since we are using a different visual context
		QTVisualContextCopyImageForTime(previewVisualContext,
			NULL,
			timeStamp,
			&previewFrame);
	    // In general this shouldn't happen, but just in case...
	    if(err != noErr && !currentFrame)
	    {
		    error("QTVisualContextCopyImageForTime: %ld\n",err);
		    rv = NO;
	    }
		
		//[delegate performSelectorOnMainThread:@selector(movieTimeChanged:) withObject:self waitUntilDone:NO];
	    rv = YES;
		newFrame = YES; // announce that we have a new frame
    }
	[lock unlock];
	[QTMovie exitQTKitOnThread];
	[pool release];
    return rv;
}

//--------------------------------------------------------------------------------------------------

- (void)updateCurrentFrame
{
    [self getFrameForTime:nil];    
}

//--------------------------------------------------------------------------------------------------

- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp
{
    CVReturn rv = kCVReturnError;
    NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
    if([self getFrameForTime:timeStamp])
    {
		[self drawRect:NSZeroRect]; // refresh the whole view
		rv = kCVReturnSuccess;
    } else {
		rv = kCVReturnError;
    }
    [pool release];
    return rv;
}

//--------------------------------------------------------------------------------------------------

- (void *)grabFrame
{
	return (void *)[self getTexture];
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
     NSOpenPanel *tvarNSOpenPanelObj	= [NSOpenPanel openPanel];
     NSInteger tvarNSInteger	= [tvarNSOpenPanelObj runModalForTypes:nil];
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
 
	QTMovie *movie = [QTMovie movieWithFile:tvarFilename error:nil];
	[self setQTMovie:movie];
	[movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
	[movie gotoBeginning];
	[self togglePlay:nil];

	layer->activate();
}

- (IBAction)toggleFilters:(id)sender
{
	doFilters = doFilters?false:true;
}

- (CIImage *)getTexture
{
	NSAutoreleasePool *pool;
	pool = [[NSAutoreleasePool alloc] init];
	CIImage     *inputImage = NULL;
	[lock lock];
	if (newFrame) {
		if (lastFrame) {
			CVPixelBufferRelease(lastFrame);
			[renderedImage release];
		}
		lastFrame = currentFrame;
		CVPixelBufferRetain(lastFrame);
		inputImage = [CIImage imageWithCVImageBuffer:currentFrame];
		newFrame = NO;
	}
	[lock unlock];
	if (inputImage) {
		if (doFilters) {
			[colorCorrectionFilter setValue:inputImage forKey:@"inputImage"];
			[effectFilter setValue:[colorCorrectionFilter valueForKey:@"outputImage"] 
						  forKey:@"inputImage"];
			[alphaFilter  setValue:[effectFilter valueForKey:@"outputImage"]
						  forKey:@"inputImage"];
			renderedImage = [alphaFilter valueForKey:@"outputImage"];
			//CVPixelBufferLockBaseAddress(freejFrame, 0);
							
			renderedImage = [renderedImage retain];
		} else {
			renderedImage = [inputImage retain];
		}
	}
	
	[pool release];
	return renderedImage;
}

- (void)mouseDown:(NSEvent *)theEvent {
    // determine if I handle theEvent
	if (!filterPanel) {
		filterPanel = [[CVFilterPanel alloc] init];
	}
	[filterPanel setLayer:self];
	[filterPanel show];
	[super mouseDown:theEvent];
}

@synthesize layer;
@end
