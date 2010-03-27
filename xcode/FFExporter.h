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

#ifndef __FFEXPORTER_H__
#define __FFEXPORTER_H__

#import "Exporter.h"
@class CVScreenView;

#define DEFAULT_FFOUTPUT_FILE "/tmp/freej_export.avi"

@interface FFExporter : NSObject <Exporter> {
    NSString *outputFile;
    CVScreenView *screen;
    NSRecursiveLock *lock;
    CVTimeStamp  lastTimestamp;
    NSImage *lastImage;
    BOOL running;
}
- (id)initWithScreen:(CVScreenView *)cvscreen;
- (BOOL)setOutputFile:(NSString *)path;
- (void)addPixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (BOOL)startExport;
- (void)stopExport;
- (BOOL)isRunning;
@end

#endif
