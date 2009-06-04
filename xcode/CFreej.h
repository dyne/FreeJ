//
//  Freej.h
//  freej
//
//  Created by xant on 2/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//
#ifndef __CFREEJ_H__
#define __CFREEJ_H__


#include <context.h>
#import <Cocoa/Cocoa.h>
#define __cocoa
#import <CVScreen.h>

@class CVideoFileInput;
@class CVF0rLayerView;
class CVLayer;

#define CFREEJ_VINPUTS_MAX 8
@interface CFreej : NSObject {
	Context *freej;
	int stdout_pipe[2];
	int stderr_pipe[2];
	NSRecursiveLock *lock;
	IBOutlet NSTextField *scriptPath;
	IBOutlet NSTextView *outputPanel;
    IBOutlet NSPopUpButton *generatorsButton;
    IBOutlet CVF0rLayerView *f0rView;
    IBOutlet CVScreenView *screen;
}
- (id)init;
//- (void)run;
- (void)start;
- (Context *)getContext;
- (NSRecursiveLock *)getLock;
- (bool)isVisible:(CVLayer *)layer;
- (IBAction)openScript:(id)sender;
- (IBAction)startGenerator:(id)sender;
- (IBAction)reset:(id)sender;

@end

#endif