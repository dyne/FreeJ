//
//  CVF0rLayerController.h
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CVF0RLAYERCONTROLLER_H__
#define __CVF0RLAYERCONTROLLER_H__

#import <Cocoa/Cocoa.h>
#import "CVLayerController.h"

@interface CVF0rLayerController : CVLayerController 
{
    CVPixelBufferRef exportedFrame;
}

- (void)reset;
- (void)feedFrame:(CVPixelBufferRef)frame;

@end

#endif
