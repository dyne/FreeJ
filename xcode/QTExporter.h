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

#ifndef __QTEXPORTER_H__
#define __QTEXPORTER_H__

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>
@class CVScreenView;

#define DEFAULT_OUTPUT_FILE "/tmp/freej_export.mov"

#ifdef __x86_64
#define DataHandler ComponentInstance
#endif

@interface QTExporter : NSObject {
    NSString *outputFile;
    QTMovie *mMovie;
    //DataHandler mDataHandlerRef;
    NSDictionary *savedMovieAttributes;
    NSMutableDictionary *encodingProperties;
    NSRecursiveLock *lock;
    CVTimeStamp  lastTimestamp;
    NSImage *lastImage;
    IBOutlet CVScreenView *screen;
}
- (id)initWithScreen:(CVScreenView *)cvscreen;
- (void)addImage:(CIImage *)image;
- (BOOL)startExport;
- (void)stopExport;
- (BOOL)isRunning;
- (BOOL)setOutputFile:(NSString *)path;
- (void)setOutputQuality:(int)quality;
- (void)setOutputFormat:(NSNumber *)format Codec:(NSString *)codec;
@end

#endif
