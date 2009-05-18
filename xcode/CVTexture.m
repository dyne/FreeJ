//
//  CVTexture.m
//  freej
//
//  Created by xant on 5/16/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CVTexture.h"


@implementation CVTexture

/*
- (id)alloc
{
    [super alloc];
    _image = nil;
    _pixelBuffer = nil;
}
*/

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
