//
//  CVTexture.m
//  freej
//
//  Created by xant on 5/16/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVTexture.h"


@implementation CVTexture

+ (id)alloc
{
    return [super alloc];
}

+ (id)textureWithCIImage:(CIImage *)image pixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    id obj = [super alloc];
    [obj autorelease];
    return [obj initWithCIImage:image pixelBuffer:pixelBuffer];
}

- (id)initWithCIImage:(CIImage *)image pixelBuffer:(CVPixelBufferRef)pixelBuffer
{
    _pixelBuffer = CVPixelBufferRetain(pixelBuffer);
    _image = [image retain];
    return self; 
}

- (void) dealloc
{
   if (_pixelBuffer) {
        //NSLog(@"%d", [_image retainCount]);
        [_image release];
        CVPixelBufferRelease(_pixelBuffer);
    }
    [super dealloc];
}

- (CIImage *)image
{
    return _image;
}

@end
