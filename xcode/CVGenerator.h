//
//  CVGenerator.h
//  freej
//
//  Created by xant on 3/7/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIImage.h>
#import <QuartzCore/CIContext.h>
#include "CVFilterPanel.h"


@interface CVGenerator : NSOpenGLView {
	CIFilter *generator;
	CIImage				*freejImage;
	CIImage				*currentImage;
	CIContext			*ciContext;
	NSRecursiveLock		*lock;
	NSTimer				*renderTimer;
	bool				needsReshape;
}

@end
