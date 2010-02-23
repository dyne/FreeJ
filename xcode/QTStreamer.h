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

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
@class CVScreenView;

#define DEFAULT_OUTPUT_FILE "/tmp/freej_export.mov"

@interface QTStreamer : NSObject {
    int active;
    int firstTime;
    int iceConnected;
    CVScreenView *screen;
    NSMutableDictionary *streamerProperties;
    NSRecursiveLock *lock;
    CVTimeStamp  lastTimestamp;
    NSImage *lastImage;
}
- (id)initWithScreen:(CVScreenView *)cvscreen;
- (void)addImage:(CIImage *)image;
- (BOOL)startStream;
- (void)stopStream;
- (BOOL)isRunning;
- (void)setParams:(NSDictionary *) params;
@end

#endif
