//
//  CVF0rLayerView.h
//  freej
//
//  Created by xant on 8/30/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifndef __CVF0RLAYERVIEW_H__
#define __CVF0RLAYERVIEW_H__

#import <Cocoa/Cocoa.h>
#import "CVLayerView.h"
#import "CVF0rLayerController.h"


@interface CVF0rLayerView : CVLayerView
{
    NSImage *icon;
    IBOutlet NSPopUpButton *selectButton;
    IBOutlet NSButton *startButton;
}
- (IBAction)start:(id)sender;
- (IBAction)stop:(id)sender;
@end

#endif

