//
//  CVFileInput.h
//  freej
//
//  Created by xant on 2/16/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//
#ifndef __CV_FILEINPUT_H__
#define __CV_FILEINPUT_H__

#include <context.h>
#import  <Cocoa/Cocoa.h>
#import  <QuickTime/QuickTime.h>
#import  <QTKit/QTKit.h>
#import  "CFreej.h";
#include "CVLayer.h"
#include "CVFilterPanel.h"
#import  "FrameRate.h"
#import  "CVPreview.h"
#import  "CVTexture.h"

@interface CVFileInput : CVLayerView {
    id                    delegate;
    QTMovie               *qtMovie; 
    QTTime                movieDuration;        // cached duration of the movie - just for convenience
    CGLContextObj         qtOpenGLContext;
    QTVisualContextRef    qtVisualContext;        // the context the movie is playing in
    
    //FrameRate            *tick;
}
//- (bool)stepBackward;
//- (bool)stepForward;
//- (void)updateCurrentFrame;
- (QTTime)movieDuration;
- (QTTime)currentTime;
- (void)setTime:(QTTime)inTime;
- (CVReturn)_renderTime:(const CVTimeStamp *)timeStamp;
- (IBAction)setMovieTime:(id)sender;
- (IBAction)openFile:(id)sender;

@end

#endif
