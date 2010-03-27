/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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
#import <FFExporter.h>
#include <fps.h>

extern "C" { // prototypes in ffenc.c
    int open_output_file(const char *file_name, int inW, int inH, int inFramerate);
    void close_output_file(void);
    void encode_buffer(uint8_t *buffer);
}

@implementation FFExporter

- (id)initWithScreen:(CVScreenView *)cvscreen
{
    running = NO;
    outputFile = nil;
    screen = cvscreen;
    lock = [[NSRecursiveLock alloc] init];
    memset(&lastTimestamp, 0, sizeof(lastTimestamp));
    lastImage = nil;
    return [super init];
}

- (void)dealloc
{
    if (running) {
        [self stopExport];
    }
    [lock release];
    [super dealloc];
}

- (BOOL)pathExists:(NSString *)aFilePath
{
	NSFileManager *defaultMgr = [NSFileManager defaultManager]; 
	return [defaultMgr fileExistsAtPath:aFilePath];
}

- (void)addPixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    timeval done, now_tv;
    if (!pixelBuffer) return;
    if (firstTime==0) {
        gettimeofday(&calc_tv, NULL);
	firstTime=1;
    }
    gettimeofday(&now_tv, NULL);
    timersub(&now_tv, &calc_tv, &done);

    int rate = 1000000 / (outFramerate);
    while ( (done.tv_sec > 0) || (done.tv_usec >= rate) ) {

	CVPixelBufferLockBaseAddress(pixelBuffer, 0);
	encode_buffer((uint8_t*) CVPixelBufferGetBaseAddress(pixelBuffer));
	CVPixelBufferUnlockBaseAddress(pixelBuffer, 0 );
	calc_tv.tv_sec  = now_tv.tv_sec  - done.tv_sec;
	calc_tv.tv_usec = now_tv.tv_usec - done.tv_usec + rate;
	timersub(&now_tv, &calc_tv, &done);
    }

    CVPixelBufferRelease(pixelBuffer);
}

- (void) exporterThread:(id)arg
{
    NSAutoreleasePool *pool;
    FPS fps;
    fps.init(2*outFramerate); 
    while ([self isRunning]) {
        pool = [[NSAutoreleasePool alloc] init];
	[lock lock];
        [self addPixelBuffer:[screen exportPixelBuffer]];
	[lock unlock];
        fps.calc();
        fps.delay();
        [pool release];
    }
}

- (BOOL)startExport:(int)fps width:(int)w height:(int)h
{
    if (!outputFile)
        outputFile = [[NSString stringWithCString:DEFAULT_FFOUTPUT_FILE  encoding:NSUTF8StringEncoding] retain];
    if (!open_output_file([outputFile UTF8String], w, h, fps)) {
        outFramerate = fps;
        firstTime = 0;
        running = YES;
        [NSThread detachNewThreadSelector:@selector(exporterThread:) 
                                 toTarget:self withObject:nil];
        return YES;
    }
    return NO;
}

- (BOOL)setOutputFile:(NSString *)path
{
    if (outputFile)
        [outputFile release];
    outputFile = [path retain];
    return YES;
}

- (void)stopExport
{
    [lock lock];
    if (running) {
        close_output_file();
    }
    running = NO;
    [lock unlock];
}

- (BOOL)isRunning
{
    return running?YES:NO;
}

@end
