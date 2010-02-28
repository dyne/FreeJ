/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
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
#import <QTStreamer.h>
#include <fps.h>


/*
 * quick & dirty hack
 * -> debug,fix and then use freeJ's internal video-encoder & shouter
 */

//#include "encoder_example.h"
#import <Carbon/Carbon.h>
#import <QuickTime/QuickTime.h>
extern "C" {
        int myOggfwd_init( const char* outIceIp, int outIcePort, const char* outPassword, const char* outIceMount,
                           const char *description, const char *genre, const char *name, const char *url );
        void myOggfwd_close() ;
        void encoder_example_init(int inW, int inH, int inFramerate, int in_video_r, int in_video_q) ;
        int encoder_example_loop( CVPixelBufferRef imBuffRef ) ;
        int encoder_example_end() ;
};

@implementation QTStreamer

- (id)initWithScreen:(CVScreenView *)cvscreen
{
    active = 0;
    firstTime =0;
    iceConnected =0;
    screen = cvscreen;
    streamerProperties = [[NSDictionary dictionaryWithObjectsAndKeys:@"mp4v",
                           QTAddImageCodecType,
                           [NSNumber numberWithLong:codecNormalQuality],
                           QTAddImageCodecQuality,
                           nil] retain];
    
    lock = [[NSRecursiveLock alloc] init];
    memset(&lastTimestamp, 0, sizeof(lastTimestamp));
    lastImage = nil;
    return [super init];
}

- (void)dealloc
{
    [streamerProperties release];
    [lock release];
    [super dealloc];
}

- (CVPixelBufferRef)fastImageFromNSImage:(NSImage *)image
{
    CVPixelBufferRef buffer = NULL;

    size_t width = [image size].width;
    size_t height = [image size].height;
    size_t bitsPerComponent = 8; // *not* CGImageGetBitsPerComponent(image);

    CGColorSpaceRef cs = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
  //CGBitmapInfo bi    = CGImageGetBitmapInfo(image);
    CGBitmapInfo bi    = kCGImageAlphaNoneSkipFirst;
  //CGBitmapInfo bi    = kCGImageAlphaNoneSkipLast; 
  //CGBitmapInfo bi    = kCGImageAlphaPremultipliedFirst; 
  //CGBitmapInfo bi    = kCGImageAlphaPremultipliedLast; 

    NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, nil];
    CVPixelBufferCreate(kCFAllocatorDefault, width, height, k32ARGBPixelFormat, (CFDictionaryRef)d, &buffer);

    CVPixelBufferLockBaseAddress(buffer, 0);

    void *rasterData = CVPixelBufferGetBaseAddress(buffer);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(buffer);

    CGContextRef ctxt = CGBitmapContextCreate(rasterData, width, height, bitsPerComponent, bytesPerRow, cs, bi);

    if(ctxt == NULL){
	NSLog(@"could not create context");
	return NULL;
    }

    NSGraphicsContext *nsctxt = [NSGraphicsContext graphicsContextWithGraphicsPort:ctxt flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:nsctxt];
#if 0
    NSRect bounds;
    bounds.origin.x=0; bounds.origin.y=0;
    bounds.size.width=width; bounds.size.height=height;

    NSEraseRect(bounds);
    [image compositeToPoint:NSMakePoint(0.0, 0.0) operation:NSCompositeSourceOver];
#else
    [image compositeToPoint:NSMakePoint(0.0, 0.0) operation:NSCompositeCopy];
#endif
    [NSGraphicsContext restoreGraphicsState];
    CVPixelBufferUnlockBaseAddress(buffer, 0);

    CFRelease(ctxt);
    return buffer;
}


//
// addImage
//
// given an array a CIImage pointer, convert it to NSImage * 
// and add the resulting image to the movie as a new MPEG4 frame
//

- (void)addImage:(CIImage *)image
{
    NSImage *nsImage = [[NSImage alloc] initWithSize:NSMakeSize([image extent].size.width, [image extent].size.height)];
    [nsImage addRepresentation:[NSCIImageRep imageRepWithCIImage:image]];
    //NSLog(@"%d\n", [images count]);
    CVTimeStamp now;
    memset(&now, 0 , sizeof(now));
    if (CVDisplayLinkGetCurrentTime((CVDisplayLinkRef)[screen getDisplayLink], &now) == kCVReturnSuccess) {
        if (lastImage) {
#if 0
            int64_t timedelta = lastTimestamp.videoTime?now.videoTime-lastTimestamp.videoTime:now.videoTimeScale/25;            
            memcpy(&lastTimestamp, &now, sizeof(lastTimestamp));
#endif

	    if (firstTime==0 && iceConnected) {
		firstTime=1;
		int vidW = lastImage.size.width;
		int vidH = lastImage.size.height;
		int outQuality = 24;
		int outFramerate = 12; 
		int outBitrate = 20000;
		encoder_example_init(vidW, vidH, outFramerate, outBitrate, outQuality) ;
		firstTime=2;
	    }

	    if (firstTime==2 && iceConnected) {
	      CVPixelBufferRef givenBuff = [self fastImageFromNSImage:lastImage];
	      int res = encoder_example_loop( givenBuff ) ;
	      [givenBuff release];
            }
        }
        if (lastImage)
            [lastImage release];
        lastImage = nsImage;
        
    } else {
        /* TODO - Error Messages */
    }
    
bail:
	return;
  
}

- (void) streamerThread:(id)arg
{
    NSAutoreleasePool *pool;
    FPS fps;
    fps.init(2*12); // XXX 
    while ([self isRunning]) {
        pool = [[NSAutoreleasePool alloc] init];
        [self addImage:[screen exportSurface]];
        fps.calc();
        fps.delay();
        [pool release];
    }
}

- (BOOL)startStream
{
    if (active) return NO;
    active =1;
    firstTime =0;

    [NSThread detachNewThreadSelector:@selector(streamerThread:) 
                             toTarget:self withObject:nil];
#if 0
    NSString * mount =  [NSString stringWithFormat:@"%@.ogg", [[streamerProperties objectForKey:@"Title" ] stringByReplacingOccurancesOfString:@" " withString:@"_"]];
#else
    NSString * mount =  [NSString stringWithFormat:@"%@.ogg", [streamerProperties objectForKey:@"Title" ]];
#endif
    NSString * author = [NSString stringWithFormat:@"http://wiki.citu.fr/users/%@", [streamerProperties objectForKey:@"Author" ]];
  //iceConnected = myOggfwd_init("theartcollider.net", 8002, "inoutsource", "/test.ogg", "desc", "tags", "title", "me");
    iceConnected = myOggfwd_init(
	[[streamerProperties objectForKey:@"Server" ] UTF8String],
	[[streamerProperties objectForKey:@"Port" ] intValue],
	[[streamerProperties objectForKey:@"Password" ] UTF8String],
	[mount UTF8String],
	[[streamerProperties objectForKey:@"Description" ] UTF8String],
	[[streamerProperties objectForKey:@"Tags" ] UTF8String],
	[[streamerProperties objectForKey:@"Title" ] UTF8String],
	[author UTF8String]);
    if (iceConnected)
	return YES;
    active=0;
    return NO;
}

- (void)stopStream
{
    [lock lock];
    if (!active) return;
    myOggfwd_close() ;
    encoder_example_end() ;
    active=0;
    iceConnected = 0;
    [lock unlock];
}

- (void)setParams:(NSDictionary *) params
{
    if (active) return;
    [streamerProperties release];
#if 0
    [[streamerProperties initWithDictionary:params copyItems:true] retain];
#else
    streamerProperties= (NSMutableDictionary*) CFPropertyListCreateDeepCopy (
	kCFAllocatorDefault, params, kCFPropertyListMutableContainersAndLeaves
    );
#endif
    NSLog(@"TXX: %@", [streamerProperties objectForKey:@"Title" ] );
}

- (BOOL)isRunning
{
    return active?YES:NO;
}

@end
