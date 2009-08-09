//
//  CVTexture.h
//  freej
//
//  Created by xant on 5/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//


#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <Appkit/NSImage.h>

@interface CVTexture : NSObject {
    CVPixelBufferRef    _pixelBuffer;
    CIImage             *_image;
    NSImage             *_nsImage;
}
+ (id)alloc;
+ (id)textureWithCIImage:(CIImage *)image pixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (id)initWithCIImage:(CIImage *)image pixelBuffer:(CVPixelBufferRef)pixelBuffer;
- (CIImage *)image;
- (NSImage *) nsImage;
- (void)dealloc;
@end
