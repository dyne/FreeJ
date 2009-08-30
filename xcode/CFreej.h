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
#import <CVScreen.h>

@class CVideoFileInput;
@class CVScreenView;
class CVLayer;
class CVScreen;

#define CFREEJ_VINPUTS_MAX 8
@interface CFreej : NSObject {
	Context *freej;
	int stdout_pipe[2];
	int stderr_pipe[2];
    CVScreen *screen;
	NSRecursiveLock *lock;
    IBOutlet NSPopUpButton *layerSelect;
	IBOutlet NSTextField *scriptPath;
	IBOutlet NSTextView *outputPanel;
    IBOutlet CVScreenView *screenView;
}
- (id)init;
//- (void)run;
- (void)start;
- (Context *)getContext;
- (NSRecursiveLock *)getLock;
- (bool)isVisible:(CVLayer *)layer;
- (IBAction)openScript:(id)sender;
- (IBAction)reset:(id)sender;

@end

/*
class CVContext : public Context
{
    public:
        CFreej *_cfreej;
    
        CVContext(CFreej *cfreej);
        ~CVContext();
        void add_layer(Layer *lay);
        bool init(int wx, int hx, VideoMode videomode, int audiomode);
};
*/

#endif