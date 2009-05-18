//
//  CVTexture.h
//  freej
//
//  Created by xant on 5/16/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

@interface CVTexture : NSObject {
    CVPixelBufferRef    _pixelBuffer;
    CIImage             *_image;
}
- (id)initWithCIImage:(CIImage *)image pixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (CIImage *)image;
- (void)dealloc;
@end
