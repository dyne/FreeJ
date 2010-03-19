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

#ifndef __QTSTREAMER_H__
#define __QTSTREAMER_H__

#define STREAMSTATS // print fps statistics
//#define PRINTSTREAMSTATS // print fps statistics

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
@class CVScreenView;

#define DEFAULT_OUTPUT_FILE "/tmp/freej_export.mov"

@interface QTStreamer : NSObject {
    int active;
    int firstTime;
    int outQuality;
    int outBitrate;
    int outFramerate;
    int iceConnected;
    void *metadata;
    CVScreenView *screen;
    NSMutableDictionary *streamerProperties;
    NSRecursiveLock *lock;
    CVTimeStamp  lastTimestamp;
    NSImage *lastImage;
    timeval calc_tv, prev_tv;
    int sent_packages;
#ifdef STREAMSTATS // statistics
#define MAX_FPS_STATISTICS (30)
    double stream_fps;
    double fps_sum;
    double fps_data[MAX_FPS_STATISTICS];
    int fps_i;
#endif
}
- (id)initWithScreen:(CVScreenView *)cvscreen meta:(void*)m;
//- (void)addImage:(CIImage *)image;
- (void)addPixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (BOOL)startStream;
- (void)stopStream;
- (BOOL)isRunning;
- (double)currentFPS;
- (int)sentPkgs;
- (void)setParams:(NSDictionary *) params;
- (void)announceStreamer;
@end

#endif
