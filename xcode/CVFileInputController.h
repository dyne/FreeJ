//
//  CVFileInput.h
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//
#ifndef __CV_FILEINPUT_H__
#define __CV_FILEINPUT_H__
#include "CVLayer.h"
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
    bool                  isPlaying;
}
- (BOOL)setQTMovie:(QTMovie *)movie;
- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;
@end

#endif
