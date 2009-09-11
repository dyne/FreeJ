//
//  CVGrabberView.h
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CVGRABBERVIEW_H__
#define __CVGRABBERVIEW_H__

#import <Cocoa/Cocoa.h>
#import "CVLayerView.h"
#import "CVGrabber.h"

@interface CVGrabberView : CVLayerView {
    IBOutlet CVGrabber *grabber;
    NSImage *icon;
}

- (void)drawRect:(NSRect)theRect;
- (bool)isOpaque;

@end

#endif