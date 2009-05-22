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
//#define __cocoa
#import <Cocoa/Cocoa.h>

@class CVideoFileInput;
class CVideoFile;

#define CFREEJ_VINPUTS_MAX 8
@interface CFreej : NSObject {
	Context *freej;
	int stdout_pipe[2];
	int stderr_pipe[2];
	NSRecursiveLock *lock;
	IBOutlet NSTextField *scriptPath;
	IBOutlet NSTextView *outputPanel;
    IBOutlet NSPopUpButton *generatorsButton;
}
- (id)init;
- (void)run;
- (void)start;
- (Context *)getContext;
- (NSRecursiveLock *)getLock;
- (IBAction)openScript:(id)sender;
- (IBAction)startGenerator:(id)sender;

@end

#endif