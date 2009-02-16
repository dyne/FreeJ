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
#import "CVideoGrabber.h"

#import <Cocoa/Cocoa.h>

@interface CFreej : NSObject {
	Context *freej;
	CVideoGrabber *captureLayer;
	IBOutlet CVideoOutput *captureDevice;
	IBOutlet NSTextField *scriptPath;
}

- (id)init;
- (IBAction)run:(id)sender;
- (IBAction)openScript:(id)sender;

@end

#endif