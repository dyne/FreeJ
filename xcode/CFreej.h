//
//  Freej.h
//  freej
//
//  Created by xant on 2/8/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//
#import <Cocoa/Cocoa.h>
#include <context.h>
#import "CVideoGrabber.h"

@interface CFreej : NSObject {
	Context *freej;
	CVideoGrabber *layer;
}
@property(readwrite) Context *freej;
@property(readwrite) CVideoGrabber *layer;

- (id)init;
- (IBAction)run:(id)sender;

@end
