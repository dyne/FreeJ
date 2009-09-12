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

#ifndef __CFREEJ_H__
#define __CFREEJ_H__

#include <context.h>

#define _UINT64
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