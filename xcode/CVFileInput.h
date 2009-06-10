//
//  CVFileInput.h
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//
#ifndef __CV_FILEINPUT_H__
#define __CV_FILEINPUT_H__

#include <context.h>
#import  <Cocoa/Cocoa.h>
#import  <QuickTime/QuickTime.h>
#import  <QTKit/QTKit.h>
#import  "CFreej.h";
#include "CVLayer.h"

@interface CVFileInput : CVLayerView {
    id                    delegate;
    QTMovie               *qtMovie; 
    QTTime                movieDuration;        // cached duration of the movie - just for convenience
    CGLContextObj         qtOpenGLContext;
    QTVisualContextRef    qtVisualContext;        // the context the movie is playing in
    bool                  isPlaying;
}

- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;
- (IBAction)setMovieTime:(id)sender;
- (IBAction)openFile:(id)sender;
- (IBAction)togglePlay:(id)sender;
@end

#endif
