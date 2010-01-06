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

#ifndef __CV_FILEINPUT_H__
#define __CV_FILEINPUT_H__

#include <CVLayer.h>
#include <context.h>
#import  <Cocoa/Cocoa.h>
#import  <QuickTime/QuickTime.h>
#import  <QTKit/QTKit.h>
#import  "CFreej.h";

@interface CVFileInputController : CVLayerController {
    id                    delegate;
    QTMovie               *qtMovie; 
    QTTime                movieDuration;        // cached duration of the movie - just for convenience
    CGLContextObj         qtOpenGLContext;
    QTVisualContextRef    qtVisualContext;        // the context the movie is playing in
    uint64_t              lastPTS;
    bool                  isPlaying;
}
- (BOOL)setQTMovie:(QTMovie *)movie;
- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;
- (BOOL)togglePlay;
- (void)task;
@end

#endif
