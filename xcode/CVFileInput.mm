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
    CVReturn ret = [(CVFileInput*)displayLinkContext _renderTime:inOutputTime];
    return ret;
}

@implementation CVFileInput : CVLayerView

- (id)init
{
    isPlaying = NO;
    return [super init];
}

- (void)dealloc
{
    if (qtMovie)
        [qtMovie release];
    if(qtVisualContext)
        QTVisualContextRelease(qtVisualContext);
    [super dealloc];
}

- (void)unloadMovie
{
    NSRect        frame = [self frame];
    [lock lock];

    QTVisualContextTask(qtVisualContext);
    [qtMovie stop];
    [qtMovie release];
    qtMovie = NULL;

    if (lastFrame)
        [lastFrame release];
    lastFrame = NULL;
    if (currentFrame)
        CVPixelBufferRelease(currentFrame);
    
    posterImage = NULL;

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

    [lock lock];
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
        //[qtMovie setMuted:YES]; // still no audio?
        
        NSArray *tracks = [qtMovie tracks];
        for (NSUInteger i = 0; i < [tracks count]; i ++) {
            QTTrack *track = [tracks objectAtIndex:i];
            NSString *type = [track attributeForKey:QTTrackMediaTypeAttribute];
            if (![type isEqualToString:QTMediaTypeVideo]) {
                [track setEnabled:NO];
                DisposeMovieTrack([track quickTimeTrack]);
            }
        }

        movieDuration = [[[qtMovie movieAttributes] objectForKey:QTMovieDurationAttribute] QTTimeValue];
    
        Context *ctx = (Context *)[freej getContext];
        QTTime posterTime = [qtMovie duration];
        posterTime.timeValue /= 2;
        NSImage *poster = [qtMovie frameImageAtTime:posterTime];
        NSData  * tiffData = [poster TIFFRepresentation];
        NSBitmapImageRep * bitmap;
        bitmap = [NSBitmapImageRep imageRepWithData:tiffData];

        CIImage *posterInputImage = [[CIImage alloc] initWithBitmapImageRep:bitmap];
        // scale the frame to fit the preview
        NSAffineTransform *scaleTransform = [NSAffineTransform transform];
        NSRect bounds = [self bounds];
        NSRect frame = [self frame];
        float scaleFactor = frame.size.width/ctx->screen->w;
        [scaleTransform scaleBy:scaleFactor];
    
        [scaleFilter setValue:scaleTransform forKey:@"inputTransform"];
        [scaleFilter setValue:posterInputImage forKey:@"inputImage"];

        posterImage = [scaleFilter valueForKey:@"outputImage"];

        CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
            NSWidth(bounds), NSHeight(bounds));
        
        [lock lock];
        [ciContext drawImage:posterImage
            atPoint: imageRect.origin
            fromRect: imageRect];
        [[self openGLContext] makeCurrentContext];
        [[self openGLContext] flushBuffer];
        [lock unlock];
        [posterInputImage release];            
       
        // register the layer within the freej context
        if (!layer) {
            layer = new CVLayer(self);
            layer->init(ctx);
            // give freej a fake buffer ... that's not going to be used anyway
            layer->buffer = (void *)pixelBuffer;
        }
    }

    [lock unlock];
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
/*
    [qtMovie setCurrentTime:inTime];
    if(CVDisplayLinkIsRunning(displayLink))
        [self togglePlay:nil];
    [self updateCurrentFrame];
    [self display];
*/
}

- (IBAction)setMovieTime:(id)sender
{
    [self setTime:QTTimeFromString([sender stringValue])];
}

- (IBAction)togglePlay:(id)sender
{
    [lock lock];
    isPlaying = isPlaying?NO:YES;
    [lock unlock];
}

- (CVTexture *)getTexture
{
    return [super getTexture];
}

- (BOOL)getFrameForTime:(const CVTimeStamp *)timeStamp
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BOOL rv = NO;
    // we can care ourselves about thread safety when accessing the QTKit api
    [QTMovie enterQTKitOnThreadDisablingThreadSafetyProtection];
    QTTime now = [qtMovie currentTime];
    // TODO - check against real hosttime to skip frames instead of
    // slowing down playback
    now.timeValue+=(now.timeScale/layer->fps.fps);
    QTTime duration = [qtMovie duration];
    if (QTTimeCompare(now, duration) == NSOrderedAscending)
        [qtMovie setCurrentTime:now];
    else 
        [qtMovie gotoBeginning];
    if(qtVisualContext)
    {        
        [lock lock];    
        if (currentFrame) 
            CVOpenGLTextureRelease(currentFrame);
            
        QTVisualContextCopyImageForTime(qtVisualContext,
        NULL,
        NULL,
        &currentFrame);
      
        // rendering (aka: applying filters) is now done in getTexture()
        // implemented in CVLayerView (our parent)
        newFrame = YES;
        rv = YES;
        [lock unlock];
    } 
    MoviesTask([qtMovie quickTimeMovie], 0);
    [QTMovie exitQTKitOnThread];    
    [pool release];
    return rv;
}


- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp
{
    NSAutoreleasePool *pool = nil;
    
    CVReturn rv = kCVReturnError;

    pool =[[NSAutoreleasePool alloc] init];

    if(qtMovie && [self getFrameForTime:timeStamp])
    {
       
        // render preview if necessary
        [self renderPreview];
        rv = kCVReturnSuccess;
    } else {
        rv = kCVReturnError;
    }
    [pool release];
    return rv;
}

- (void)task
{
    QTVisualContextTask(qtVisualContext);
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

- (void)openFilePanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{
     if(returnCode == NSOKButton){
     	func("openScript we have an OK button");	
     } else if(returnCode == NSCancelButton) {
     	func("openScript we have a Cancel button");
     	return;
     } else {
     	error("doOpen tvarInt not equal 1 or zero = %3d",returnCode);
     	return;
     } // end if     
     NSString * tvarDirectory = [panel directory];
     func("openScript directory = %@",tvarDirectory);

     NSString * tvarFilename = [panel filename];
     func("openScript filename = %@",tvarFilename);
	 
    if (tvarFilename) {
        //if(CVDisplayLinkIsRunning(displayLink)) 
        //    [self togglePlay:nil];
        NSDictionary *movieAttributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], QTMovieOpenAsyncOKAttribute,tvarFilename, QTMovieFileNameAttribute,[NSNumber numberWithBool:NO] , QTMovieHasAudioAttribute, nil];
        QTMovie *movie = [[QTMovie alloc] initWithAttributes:movieAttributes error:nil];
        [movie setIdling:NO];
        [self setQTMovie:movie];
        [movie setAttribute:[NSNumber numberWithBool:YES] forKey:QTMovieLoopsAttribute];
        //[movie setAttribute:[NSNumber numberWithBool:NO] forKey:QTMovieHasAudioAttribute];
        //NSLog(@"movie %@", [movie movieAttributes]);

        //[movie gotoBeginning];
        [self togglePlay:nil];

    }

}

- (IBAction)openFile:(id)sender 
{	
     func("doOpen");	
     NSOpenPanel *tvarNSOpenPanelObj	= [NSOpenPanel openPanel];
     NSArray *types = [NSArray arrayWithObjects:
        [NSString stringWithUTF8String:"avi"],
        [NSString stringWithUTF8String:"mov"],
        [NSString stringWithUTF8String:"mpg"],
        [NSString stringWithUTF8String:"asf"],
        [NSString stringWithUTF8String:"jpg"],
        [NSString stringWithUTF8String:"png"],
        [NSString stringWithUTF8String:"tif"],
        [NSString stringWithUTF8String:"bmp"],
        [NSString stringWithUTF8String:"gif"],
        [NSString stringWithUTF8String:"pdf"],
        nil];
        
     [tvarNSOpenPanelObj 
        beginSheetForDirectory:nil 
        file:nil
        types:types 
        modalForWindow:[sender window]
        modalDelegate:self 
        didEndSelector:@selector(openFilePanelDidEnd: returnCode: contextInfo:) 
        contextInfo:nil];	
    [tvarNSOpenPanelObj setCanChooseFiles:YES];
} // end openFile


@end
