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
		encoder_example_init(vidW, vidH, outFramerate, outBitrate, outQuality) ;
		firstTime=2;
	    }

	    if (firstTime==2 && iceConnected) {
	      CVPixelBufferRef givenBuff = [self fastImageFromNSImage:lastImage];
	      int res = encoder_example_loop( givenBuff ) ;
	      [givenBuff release];
	      if (!res) { 
		if (lastImage)
		    [lastImage release];
		[self stopStream];
	      }
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

//
// addImage - alternative
//
- (void)addPixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    timeval done, now_tv;
    if (!pixelBuffer) return;

    if (firstTime==0 && iceConnected) {
	firstTime=1;
	int vidW = CVPixelBufferGetWidth(pixelBuffer);
	int vidH = CVPixelBufferGetHeight(pixelBuffer);
	encoder_example_init(vidW, vidH, outFramerate, outBitrate, outQuality) ;
	firstTime=2;
        gettimeofday(&calc_tv, NULL);
        gettimeofday(&prev_tv, NULL);
    }

    gettimeofday(&now_tv, NULL);
    timersub(&now_tv, &calc_tv, &done);
    int rate = 1000000 / (outFramerate);
    if ( (done.tv_sec > 0) || (done.tv_usec >= rate) ) {
	if (firstTime==2 && iceConnected) {
	  int res = encoder_example_loop( pixelBuffer ) ;
	  if (!res) [self stopStream];

#ifdef STREAMSTATS // statistics
	  timeval stats;
	  timersub(&now_tv, &prev_tv, &stats);
	  double curr_fps = 1000000.0 /  stats.tv_usec;
	  fps_sum = fps_sum - fps_data[fps_i] + curr_fps;
	  fps_data[fps_i] = curr_fps;
	  if (++fps_i >= MAX_FPS_STATISTICS) { fps_i = 0;
	    printf("stream fps: %.1f\n", fps_sum/((double)MAX_FPS_STATISTICS));
	  }
#endif
	}

	calc_tv.tv_sec  = now_tv.tv_sec  - done.tv_sec;
	calc_tv.tv_usec = now_tv.tv_usec - done.tv_usec + rate;

#ifdef STREAMSTATS // statistics
	prev_tv.tv_sec  = now_tv.tv_sec;
	prev_tv.tv_usec = now_tv.tv_usec;
#endif
    }

    CVPixelBufferRelease(pixelBuffer);
}


- (void) streamerThread:(id)arg
{
    NSAutoreleasePool *pool;
    FPS fps;
    fps.init(2*outFramerate); // XXX 
    while ([self isRunning]) {
        pool = [[NSAutoreleasePool alloc] init];
#if 0
        [self addImage:[screen exportSurface]];
#else
        [self addPixelBuffer:[screen exportPixelBuffer]];
#endif
        fps.calc();
        fps.delay();
        [pool release];
    }
}

- (BOOL)startStream
{
    [lock lock];
    if (active) { 
      [lock unlock];
      return NO;
    }
    active =1;
    firstTime =0;
    outFramerate = [[streamerProperties objectForKey:@"Framerate" ] intValue];
    outBitrate =   [[streamerProperties objectForKey:@"Bitrate" ] intValue];
    outQuality =   [[streamerProperties objectForKey:@"Quality" ] intValue];
    if (outFramerate < 1  || outFramerate > 30)   outFramerate=12;
    if (outQuality < 2    || outQuality > 32)     outQuality=16;
    if (outBitrate < 8000 || outBitrate > 1048576) outBitrate=128000;

#ifdef STREAMSTATS // statistics
    for (int i=0; i<MAX_FPS_STATISTICS; i++) { fps_data[i] = 0; }
    fps_sum=0; fps_i=0;
#endif

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

    [lock unlock];
    if (iceConnected)
	return YES;
    active=0;
    return NO;
}

- (void)stopStream
{
    [lock lock];
    if (!active) {
      [lock unlock];
      return;
    }
    iceConnected = 0;
    struct timespec delay = { 0, 1000000000/outFramerate };
    nanosleep(&delay,NULL); // wait for current encoder.
    encoder_example_end() ;
    myOggfwd_close() ;
    active=0;
    [lock unlock];
}

- (void)setParams:(NSDictionary *) params
{
    [lock lock];
    if (active) {
      [lock unlock];
      return;
    }
    [streamerProperties release];
    streamerProperties= (NSMutableDictionary*) CFPropertyListCreateDeepCopy (
	kCFAllocatorDefault, params, kCFPropertyListMutableContainersAndLeaves
    );
    [lock unlock];
}

- (BOOL)isRunning
{
    return active?YES:NO;
}

@end
