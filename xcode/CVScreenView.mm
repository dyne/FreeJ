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

#import <CVScreenView.h>

#include <CVLayer.h>

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
    CVReturn ret = [(CVScreenView*)displayLinkContext outputFrame:inNow->videoTime];
    return ret;
}


@implementation CVScreenView : NSOpenGLView
/*
 - (void)windowChangedSize:(NSNotification*)inNotification
 {
 //    NSRect frame = [self frame];
 //    [self setSizeWidth:frame.size.width Height:frame.size.height];    
 }
 */

- (void)awakeFromNib
{
    [self init];
}

- (void)start
{
    if (initialized)
        return;
    initialized = YES;
    Context *ctx = [freej getContext];
    CVScreen *screen = (CVScreen *)ctx->screen;
    screen->set_view(self);
    CVDisplayLinkStart(displayLink);
    if (!rateCalc) {
        CVTimeStamp now;
        memset(&now, 0, sizeof(now));
        CVDisplayLinkGetCurrentTime(displayLink, &now);
        rateCalc = [[FrameRate alloc] initWithTimeScale:now.videoTimeScale];
        [rateCalc retain];
    }
    [layerList setDataSource:(id)self];
    [layerList registerForDraggedTypes:[NSArray arrayWithObject:@"CVLayer"]];
}

- (id)init
{
    needsReshape = YES;
    outFrame = NULL;
    lastFrame = NULL;
    exportedFrame = NULL;
    streamerStatus = NO;
    lock = [[NSRecursiveLock alloc] init];
    [lock retain];
    [self setNeedsDisplay:NO];
    [freej start];
    Context *ctx = (Context *)[freej getContext];
    fjScreen = (CVScreen *)ctx->screen;

    CVReturn err = CVPixelBufferCreate (
                                        NULL,
                                        fjScreen->geo.w,
                                        fjScreen->geo.h,
                                        k32ARGBPixelFormat,
                                        NULL,
                                        &pixelBuffer
                                        );
    if (err) {
        // TODO - Error messages
        return nil;
    }
    CVPixelBufferLockBaseAddress(pixelBuffer, NULL);
    exportBuffer = CVPixelBufferGetBaseAddress(pixelBuffer);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    exportCGContextRef = CGBitmapContextCreate (NULL,
                                                ctx->screen->geo.w,
                                                ctx->screen->geo.h,
                                                8,      // bits per component
                                                ctx->screen->geo.w*4,
                                                colorSpace,
                                                kCGImageAlphaPremultipliedLast);
    
    if (exportCGContextRef == NULL)
        NSLog(@"Context not created!");
    exportContext = [[CIContext contextWithCGContext:exportCGContextRef 
                                             options:[NSDictionary dictionaryWithObject: (NSString*) kCGColorSpaceGenericRGB 
                                                                                 forKey:  kCIContextOutputColorSpace]] retain];
    CGColorSpaceRelease( colorSpace );
    exporterQT = [[QTExporter alloc] initWithScreen:self];
    exporterFF = [[FFExporter alloc] initWithScreen:self];

    exporter = (id<Exporter>)exporterQT;
    streamer = [[QTStreamer alloc] initWithScreen:self];

#ifdef __x86_64
    [exportEngine removeItemWithTitle:@"QuickTime"]; // XXX - fix quicktime export for 64bit
    exporter = (id<Exporter>)exporterFF;
#endif

    streamerKeys = [[NSMutableArray arrayWithObjects:@"Title", @"Tags", @"Author", @"Description", @"Server", @"Port", @"Password", @"Framerate", @"Bitrate", @"Quality", @"Announcements", nil] retain];
    NSMutableArray *objects = [NSMutableArray arrayWithObjects:@"MyTitle", @"Remix,Video", @"me", @"playing with Freej-OSX", @ICECASTSERVER, @ICECASTPORT, @ICECASTPASSWORD, @"15", @"128000", @"24", @"1", nil];
    streamerDict = [[NSMutableDictionary dictionaryWithObjects:objects forKeys:streamerKeys] retain];

    [streamerDict setValue:[NSString stringWithFormat:@"MyTitle-%02d",rand()%99] forKey:[streamerKeys objectAtIndex:0]];

    NSArray *columnArray = [streamerSettings tableColumns];
    [[columnArray objectAtIndex:0] setIdentifier:[NSNumber numberWithInt:0]];
    [[columnArray objectAtIndex:1] setIdentifier:[NSNumber numberWithInt:1]];

    //[streamerSettings setDataSource:self];
    [streamerSettings reloadData];

    return self;
}

- (void)dealloc
{
    Context *ctx = (Context *)[freej getContext];

    CVPixelBufferUnlockBaseAddress(pixelBuffer, NULL);
    CVOpenGLTextureRelease(pixelBuffer);
    if (rateCalc)
        [rateCalc release];
    [ciContext release];
    [currentContext release];
    if (outFrame)
        [outFrame release];
    if (lastFrame)
        [lastFrame release];
    [lock release];
    [exportContext release];
    CGContextRelease( exportCGContextRef );
    // TODO: free streamer Keys&Array
    [streamer release]; // stop it first
    //[exporterQT release]; // ??
    [exporterFF release];
    [super dealloc];
}

- (void)prepareOpenGL
{
    CVReturn                ret;
    
    // Create display link 
    CGOpenGLDisplayMask    totalDisplayMask = 0;
    int            virtualScreen;
    GLint        displayMask;
    NSOpenGLPixelFormat    *openGLPixelFormat = [self pixelFormat];
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
    
    
    //[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowChangedSize:) name:NSWindowDidResizeNotification object:nil];
    
    GLint params[] = { 1 };
    CGLSetParameter( CGLGetCurrentContext(), kCGLCPSwapInterval, params );
    
    // Set up display link callbacks 
    CVDisplayLinkSetOutputCallback(displayLink, renderCallback, self);
    
    // start asking for frames
    [self start];
}

- (void) update
{
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return;
    [super update];
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

- (void)drawRect:(NSRect)theRect
{
    NSRect        frame = [self frame];
    NSRect        bounds = [self bounds];    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return;
    
    [currentContext makeCurrentContext];
    
    if(needsReshape)    // if the view has been resized, reset the OpenGL coordinate system
    {
        GLfloat     minX, minY, maxX, maxY;
        
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
    [[self openGLContext] flushBuffer];
    //[super drawRect:theRect];
    [self setNeedsDisplay:NO];
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    [pool release];
}

- (CVPixelBufferRef)exportPixelBuffer
{
    void *surface = [self getSurface];
    if (!surface) return nil;
    CVPixelBufferRef pixelBufferOut = nil;
    CVReturn cvRet = CVPixelBufferCreateWithBytes (
						   NULL,
						   fjScreen->geo.w,
						   fjScreen->geo.h,
						   k32ARGBPixelFormat,
						   surface,
						   fjScreen->geo.w*4,
						   NULL,
						   NULL,
						   NULL,
						   &pixelBufferOut
						   );
    if (cvRet != noErr) {
        // TODO - Error Messages
        if (pixelBufferOut)
            CVPixelBufferRelease(pixelBufferOut);
        return nil;
    }
    return pixelBufferOut;
}

- (CIImage *)exportSurface
{
    CIImage *exportedSurface = nil;
    CVPixelBufferRef pixelBufferOut = [self exportPixelBuffer];
    if (pixelBufferOut) {
        exportedSurface = [CIImage imageWithCVImageBuffer:pixelBufferOut];
        CVPixelBufferRelease(pixelBufferOut);
    }
    return exportedSurface;
}

- (void *)getSurface
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [lock lock];
    Context *ctx = (Context *)[freej getContext];
    
    if (lastFrame) {
        NSRect bounds = [self bounds];
        
        CGRect rect = CGRectMake(0,0, ctx->screen->geo.w, ctx->screen->geo.h);
        [exportContext render:lastFrame 
                     toBitmap:exportBuffer
                     rowBytes:ctx->screen->geo.w*4
                       bounds:rect 
                       format:kCIFormatARGB8 
                   colorSpace:NULL];
    }
    [lock unlock];
    [pool release];
    return exportBuffer;
}

- (CVReturn)outputFrame:(uint64_t)timestamp
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    //CVTexture *textureToRelease = nil;
    NSRect        bounds = [self bounds];
    [lock lock];
    Context *ctx = (Context *)[freej getContext];
    
    ctx->cafudda(0.0);
    CVTimeStamp now;
    memset(&now, 0 , sizeof(now));
    if (CVDisplayLinkGetCurrentTime(displayLink, &now) == kCVReturnSuccess) {

        if (streamer && streamerStatus != [streamer isRunning]) {
		streamerStatus=[streamer isRunning];
		[streamerButton setTitle:@"Start"];
        // TODO reset stream_fps info
         NSString *tfps = [NSString stringWithString:@"FPS: -"];
         NSString *tpkg = [NSString stringWithString:@"tx: -"];
        [streamerFPS setStringValue:tfps];
        [streamerPkg setStringValue:tpkg];
	    }
        if (streamer && [streamer isRunning]) {
            NSString *tfps = [NSString stringWithFormat:@"FPS: %1f", [streamer currentFPS]];
            NSString *tpkg = [NSString stringWithFormat:@"tx: %d", [streamer sentPkgs]];
            [streamerFPS setStringValue:tfps];
            [streamerPkg setStringValue:tpkg];
        }

        if (rateCalc) {
            [rateCalc tick:now.videoTime];
            NSString *toRelease = nil;
            if (fpsString)
                toRelease = fpsString;
            fpsString = [[NSString alloc] initWithFormat:@"%0.1lf", [rateCalc rate]];
            [showFps setStringValue:fpsString];
            if (toRelease)
                [toRelease release];
        } 
    }
    if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
        return kCVReturnError;
    
    if (outFrame) {
        CGRect  cg = CGRectMake(NSMinX(bounds), NSMinY(bounds),
                                NSWidth(bounds), NSHeight(bounds));
        [ciContext drawImage: outFrame
                     atPoint: cg.origin  fromRect: cg];
        if (lastFrame)
            [lastFrame autorelease];
        lastFrame = outFrame;
        outFrame = NULL;
    } else {
        needsReshape = YES;
    }
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);    
    
    // XXX - we force rendering at this stage instead of delaying it setting the needsDisplay flag
    // to avoid stopping displaying new frames on the output screen while system animations are in progress
    // (for example, when opening a fileselction panel the CVScreen rendering would stop while the animation is in progress
    // because both the actions would happen in the main application thread. Forcing rendering now makes it happen in the 
    // CVScreen thread
    //[self setNeedsDisplay:YES]; // this will delay rendering to be done  the application main thread
    [self drawRect:NSZeroRect]; // this directly render the frame out in this thread
    
    [lock unlock];
    [pool release];
    return kCVReturnSuccess;
}

- (void)drawLayer:(Layer *)layer
{
    //NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CIFilter *blendFilter = nil;
    CVTexture *texture = nil;
    
    if (layer->type == Layer::GL_COCOA) {
        
        CVLayer *cvLayer = (CVLayer *)layer;
        texture = cvLayer->gl_texture();
        
        NSString *blendMode = ((CVLayer *)layer)->blendMode;
        if (blendMode)
            blendFilter = [CIFilter filterWithName:[[NSString alloc] initWithFormat:@"CI%@BlendMode", blendMode]];
        else
            blendFilter = [CIFilter filterWithName:@"CIOverlayBlendMode"]; 
    } else { // freej 'not-cocoa' layer type
        
        CVPixelBufferRef pixelBufferOut;
        //_BGRA2ARGB(layer->buffer, layer->geo.w*layer->geo.h); // XXX - expensive conversion
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
        CIImage *inputImage = [CIImage imageWithCVImageBuffer:pixelBufferOut];
        texture = [[CVTexture alloc] initWithCIImage:inputImage pixelBuffer:pixelBufferOut];
        // we can release our reference to the pixelBuffer now. 
        // The CVTexture will retain it as long as it is needed
        CVPixelBufferRelease(pixelBufferOut);
        blendFilter = [CIFilter filterWithName:@"CIOverlayBlendMode"];
    }
    [blendFilter setDefaults];
    if (texture) {
        if (!outFrame) {
            outFrame = [[texture image] retain];
        } else {
            [blendFilter setValue:outFrame forKey:@"inputBackgroundImage"];
            [blendFilter setValue:[texture image] forKey:@"inputImage"];
            CIImage *temp = [blendFilter valueForKey:@"outputImage"];
            [outFrame release];
            outFrame = [temp retain];
        }
        [texture release];
    }
}

#define MYCGL
- (void)allocPixelBuffer
{
    NSLog(@"pixel buffer: %ix%i", fjScreen->geo.w, fjScreen->geo.h);
#if 0
    CVReturn err = CVOpenGLBufferCreate (NULL, fjScreen->geo.w, fjScreen->geo.h, NULL, &pixelBuffer);
#else
    CVReturn err = CVPixelBufferCreate (
					NULL,
					fjScreen->geo.w,
					fjScreen->geo.h,
					k32ARGBPixelFormat,
					NULL,
					&pixelBuffer
					);
#endif
    if (err) {
	// TODO - Error messages
    }
    CVPixelBufferRetain(pixelBuffer);
    CVPixelBufferLockBaseAddress(pixelBuffer, NULL);
    exportBuffer = CVPixelBufferGetBaseAddress(pixelBuffer);
    //CVPixelBufferUnlockBaseAddress(pixelBuffer, 0 );
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);

#ifdef MYCGL
    exportCGContextRef = CGBitmapContextCreate (NULL,
						fjScreen->geo.w,
						fjScreen->geo.h,
						8,      // bits per component
						fjScreen->geo.w*4,
						colorSpace,
						kCGImageAlphaPremultipliedLast);

    if (exportCGContextRef == NULL)
        NSLog(@"WARNING: Export-context not created!");

    exportContext = [[CIContext contextWithCGContext:exportCGContextRef 
                                             options:[NSDictionary dictionaryWithObject: (NSString*) kCGColorSpaceGenericRGB 
                                                                                 forKey:  kCIContextOutputColorSpace]] retain];
#else
    CGLPixelFormatObj pFormat;
    GLint npix;
    const int attrs[2] = { kCGLPFADoubleBuffer, NULL};
    CGLError err = CGLChoosePixelFormat (
	(CGLPixelFormatAttribute *)attrs,
	&pFormat,
	&npix
    );
    CGLCreateContext (pFormat, NULL, exportCGContextRef);

    if (exportCGContextRef == NULL)
        NSLog(@"WARNING: Export-context not created!");

  //exportContext = [[CIContext contextWithCGLContext:(CGLContextObj)[currentContext CGLContextObj]
    exportContext = [[CIContext contextWithCGContext:exportCGContextRef 
                                 //       pixelFormat:(CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj]
                                          options:[NSDictionary dictionaryWithObjectsAndKeys:
                                                   (id)colorSpace,kCIContextOutputColorSpace,
                                                   (id)colorSpace,kCIContextWorkingColorSpace,nil]] retain];
#endif

    CGColorSpaceRelease( colorSpace );

    if (lastFrame)
        [lastFrame release];
    lastFrame = NULL;
}

- (void)setSizeWidth:(int)w Height:(int)h
{
    [lock lock];
    if (w != fjScreen->geo.w || h != fjScreen->geo.h) {
        CVPixelBufferRelease(pixelBuffer);

        fjScreen->geo.w = w;
        fjScreen->geo.h = h;

	[self allocPixelBuffer];
        needsReshape = YES;
        NSRect frame = [window frame];
        frame.size.width = w;
        frame.size.height = h;
        [window setFrame:frame display:YES];
        fjScreen->resize_w = w;
        fjScreen->resize_h = h;
    }
    [lock unlock];
}

- (IBAction)toggleFullScreen:(id)sender
{    
    CGDirectDisplayID currentDisplayID = (CGDirectDisplayID)[[[[[self window] screen] deviceDescription] objectForKey:@"NSScreenNumber"] intValue];  
    CGDisplayErr err;
    if (fullScreen) {
		//CGDisplaySwitchToMode(currentDisplayId savedMode);
		err = CGDisplaySetDisplayMode(currentDisplayID, savedMode, NULL);
		if ( err != CGDisplayNoErr ) {
			// TODO -e rror messages
		}
        SetSystemUIMode(kUIModeNormal, kUIOptionAutoShowMenuBar);
        [self retain];
        NSWindow *fullScreenWindow = [self window];
        [self removeFromSuperview];
        [myWindow setContentView:self];
        [myWindow release];
        [self release];
        [fullScreenWindow release];
        fullScreen = NO;
        needsReshape = YES;
    } else {
		//CFDictionaryRef newMode = CGDisplayBestModeForParameters(currentDisplayID, 32, fjScreen->geo.w, fjScreen->geo.h, 0);
		CGDisplayModeRef newMode;
		bool exactMatch;
		// Loop through all display modes to determine the closest match.
		// CGDisplayBestModeForParameters is deprecated on 10.6 so we will emulate it's behavior
		// Try to find a mode with the requested depth and equal or greater dimensions first.
		// If no match is found, try to find a mode with greater depth and same or greater dimensions.
		// If still no match is found, just use the current mode.
		CFArrayRef allModes = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
		for(int i = 0; i < CFArrayGetCount(allModes); i++)	{
			CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
			CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
			if(CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) != kCFCompareEqualTo)
				continue;
			
			if((CGDisplayModeGetWidth(mode) >= fjScreen->geo.w) && (CGDisplayModeGetHeight(mode) >= fjScreen->geo.h))
			{
				newMode = mode;
				exactMatch = true;
				break;
			}
		}
		
		// No depth match was found
		if(!exactMatch)
		{
			for(int i = 0; i < CFArrayGetCount(allModes); i++)
			{
				CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
				CFStringRef pixEnc = CGDisplayModeCopyPixelEncoding(mode);
				if(CFStringCompare(pixEnc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) != kCFCompareEqualTo)
					continue;
				
				if((CGDisplayModeGetWidth(mode) >= fjScreen->geo.w) && (CGDisplayModeGetHeight(mode) >= fjScreen->geo.h))
				{
					newMode = mode;
					break;
				}
			}
		}
        //CFDictionaryRef newMode = CGDisplayBestModeForParameters(currentDisplayID, 32, fjScreen->geo.w, fjScreen->geo.h, 0);
        NSAssert(newMode, @"Couldn't find display mode");
        myWindow = [[self window] retain];
/*
		savedMode = CGDisplayCurrentMode(currentDisplayID);
        SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
		
        CGDisplaySwitchToMode(currentDisplayID, newMode);
*/		
        savedMode = CGDisplayCopyDisplayMode(currentDisplayID);
        SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);

        //CGDisplaySwitchToMode(currentDisplayID, newMode);
		err = CGDisplaySetDisplayMode(currentDisplayID, newMode, NULL);
		if ( err != CGDisplayNoErr ) {
			// TODO -e rror messages
		}
        NSScreen *screen = [[self window] screen];
        NSWindow *newWindow = [[NSWindow alloc] initWithContentRect:[screen frame]
                                                          styleMask:NSBorderlessWindowMask
                                                            backing:NSBackingStoreBuffered
                                                              defer:NO
                                                             screen:screen];
        [self retain];
        [self removeFromSuperview];
        [newWindow setContentView:self];
        [newWindow setFrameOrigin:[screen frame].origin];
        [self release];
        [newWindow makeKeyAndOrderFront:sender];
        [NSCursor setHiddenUntilMouseMoves:YES];
        fullScreen = YES;
    }
    [self drawRect:NSZeroRect];
}

- (void)setExporter
{
    /*
    if ([exportButtonFF state])
        exporter = (id<Exporter>)exporterFF;
    else if ([exportButtonOG state])
        exporter = (id<Exporter>)exporterOG;
    else
        exporter = (id<Exporter>)exporterQT;
    */
}

- (IBAction)startExport:(id)sender
{
    Context *ctx = [freej getContext];
    [self setExporter];
    if (exporter) 
        if ([exporter startExport:25 width:ctx->screen->geo.w height:ctx->screen->geo.h]) {
            NSLog(@"started file exporter: %ix%i @%ifps", ctx->screen->geo.w, ctx->screen->geo.h, 25);
            [sender setTitle:@"Stop"];
            /*
                [exportButtonQT setEnabled:false];
                [exportButtonFF setEnabled:false];
                [exportButtonOG setEnabled:false];
                [exportFilename setEnabled:false];
             */
        }
}

- (IBAction)stopExport:(id)sender
{
    [self setExporter];
    if (exporter)
        [exporter stopExport];
    [sender setTitle:@"Start"];
    [exportButton setEnabled:YES];
    /*
    [exportButtonFF setEnabled:YES];
    [exportButtonOG setEnabled:NO]; // XXX Ogg exporter: not yet implemented
#ifdef __x86_64
    [exportButtonQT setEnabled:NO];
#else
    [exportButtonQT setEnabled:YES];
#endif
     */
}

- (void)setExportOutputFile:(NSString *)filename
{
#if 0
    [exporterQT setOutputFile:filename];
    [exporterFF setOutputFile:filename];
    //[exporterOG setOutputFile:filename];
#else
    [exporter setOutputFile:filename];
#endif
}



- (IBAction)toggleExport:(id)sender
{
    [self setExporter];
    [self setExportOutputFile:[exportFilename stringValue]];
    if (exporter) {
        if ([exporter isRunning])
            [self stopExport:sender];
        else
            [self startExport:sender];
    }
}

- (void)setExportFileDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{
    if(returnCode == NSOKButton){
        func("openFilePanel: OK");    
    } else if(returnCode == NSCancelButton) {
        func("openFilePanel: Cancel");
        return;
    } else {
        error("openFilePanel: Error %3d",returnCode);
        return;
    } // end if     
    NSString * tvarDirectory = [panel directory];
    func("openFile directory = %@",tvarDirectory);
    
    NSString * tvarFilename = [[panel filename]retain];
    func("openFile filename = %@",tvarFilename);
    
    if (tvarFilename) {
        [exportFilename setStringValue:tvarFilename];
    }
    [tvarFilename release];
}

- (IBAction)setExportFile:(id)sender
{
    NSSavePanel *fileSelectionPanel    = [NSSavePanel savePanel];
    
    [fileSelectionPanel 
     beginSheetForDirectory:nil 
     file:nil
     modalForWindow:[sender window]
     modalDelegate:self 
     didEndSelector:@selector(setExportFileDidEnd: returnCode: contextInfo:) 
     contextInfo:sender];        
}

- (IBAction)toggleStreamer:(id)sender
{
    if (streamer) {
        if ([streamer isRunning]) {
	    [streamer stopStream];
            [sender setTitle:@"Start"];
	    streamerStatus=[streamer isRunning];
        NSString *tfps = [NSString stringWithString:@"FPS: -"];
        NSString *tpkg = [NSString stringWithString:@"tx: -"];
        [streamerFPS setStringValue:tfps];
        [streamerPkg setStringValue:tpkg];
	} else {
	    [streamer setParams: streamerDict];
	    if ([streamer startStream])
		    [sender setTitle:@"Stop"];
	    streamerStatus=[streamer isRunning];
	}
    }
}

- (void)openStreamPresetDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    /*
    if (returnCode == NSOKButton) {
		[streamerDict release];
		streamerDict = [[((ICPresetPanel*)icPresetPanel) getDict] mutableCopy];
        // TODO check if all 'streamerKeys' are present in streamerDict.
    	[streamerSettings reloadData];
    }
    */
    [sheet orderOut:self];
    [[NSApplication sharedApplication] endSheet:icPresetPanel];
}

- (IBAction)presetsStreamer:(id)sender
{
    /*
    [(ICPresetPanel*)icPresetPanel myReset:streamerDict]; 
    [[NSApplication sharedApplication] beginSheet:icPresetPanel 
		modalForWindow:[sender window]
		modalDelegate:self 
		didEndSelector:@selector(openStreamPresetDidEnd:returnCode:contextInfo:) 
		contextInfo:self];
    */
}

- (bool)isOpaque
{
    return YES;
}

- (double)rate
{
    if (rateCalc) 
        return [rateCalc rate];
    return 0;
}

- (void)reset
{
    needsReshape = YES;
    [self drawRect:NSZeroRect];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if (aTableView == layerList) {
      return fjScreen->layers.length;
    } 
    if (aTableView == streamerSettings) {
      return [streamerDict count];
    }
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if (aTableView == layerList) { // list of active layers
        Layer *lay = fjScreen->layers.pick(rowIndex+1);
        if (lay)
            return [NSString stringWithUTF8String:lay->name];
    }
    if (aTableView == streamerSettings) { // table with streamer settings. TODO - get it out of here
      //if ([[pTableColumn identifier] isEqualToString:@"Col_ID3"]) // XX
        if ([aTableColumn identifier] == [NSNumber numberWithInt:0])
            return [NSString stringWithString:[streamerKeys objectAtIndex:rowIndex]];
        else
            return [NSString stringWithString:[streamerDict objectForKey:[streamerKeys objectAtIndex:rowIndex]]];
    }
    return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(int)rowIndex
{
    if (aTableView == streamerSettings && [aTableColumn identifier] == [NSNumber numberWithInt:1])
        [streamerDict setValue:anObject forKey:[streamerKeys objectAtIndex:rowIndex]];
}

- (NSArray *)tableView:(NSTableView *)aTableView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedRowsWithIndexes:(NSIndexSet *)indexSet
{
    if (aTableView != layerList)
        return nil;
    return [NSArray arrayWithObjects:@"CVLayer", @"CVLayerIndex", nil];
}

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
    if (aTableView != layerList) return NO;

    NSUInteger row = [rowIndexes firstIndex];
    Layer *lay = fjScreen->layers.pick(row+1);
    if (lay) {
        [pboard addTypes:[NSArray arrayWithObjects:@"CVLayer", @"CVLayerIndex", nil] owner:(id)self];
        [pboard setData:[NSData dataWithBytes:(void *)&lay length:sizeof(Layer *)] forType:@"CVLayer"];
        [pboard setData:[NSData dataWithBytes:(void *)&row length:sizeof(NSUInteger)] forType:@"CVLayerIndex"];
        return YES;
    }
    return NO;
}


- (NSDragOperation)tableView:(NSTableView *)aTableView validateDrop:(id < NSDraggingInfo >)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)operation
{
    NSDragOperation dragOp = NSDragOperationCopy;
    if ([info draggingSource] == layerList) {
        dragOp =  NSDragOperationMove;
    }
    [aTableView setDropRow:row dropOperation:NSTableViewDropAbove];
    
    return dragOp;
}

- (IBAction)updateExporterSettings:(id)sender
{
    NSString *selectedEngine = [[exportEngine selectedItem] title];
    if ([selectedEngine localizedCaseInsensitiveCompare:@"quicktime"] == NSOrderedSame)
        exporter = (id<Exporter>)exporterQT;
    else if ([selectedEngine localizedCaseInsensitiveCompare:@"ffmpeg"] == NSOrderedSame)
        exporter = (id<Exporter>)exporterFF;
    
    
}


- (BOOL)tableView:(NSTableView *)aTableView acceptDrop:(id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation
{
    if (aTableView != layerList)
        return NO;

    Layer *lay = NULL;
    NSUInteger srcRow;
    
    [[[info draggingPasteboard] dataForType:@"CVLayerIndex"] getBytes:&srcRow length:sizeof(NSUInteger)];  
    [[[info draggingPasteboard] dataForType:@"CVLayer"] getBytes:&lay length:sizeof(Layer *)];
    if (lay) {
        lay->move((srcRow > row)?(row+1):row);
        [layerList reloadData];
        return YES;
    }
    return NO;
}

- (void)addLayer:(Layer *)lay
{
    [layerList reloadData];
}

- (void)remLayer:(Layer *)lay
{
    [layerList reloadData];
}

- (NSWindow *)getWindow
{
    return window;
}

- (CVDisplayLinkRef)getDisplayLink
{
    return displayLink;
}

@synthesize fullScreen;

@end
