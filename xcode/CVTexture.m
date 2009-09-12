/*  FreeJ
 *  (c) Copyright 2009 Andrea Guzzo <xant@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#import <CVTexture.h>


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
