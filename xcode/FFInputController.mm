/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
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

#import <FFInputController.h>
#import <CFreej.h>
#import <CIAlphaFade.h>

#include <jutils.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

@implementation FFInputController : CVLayerController

- (id)init
{
    exportedFrame = nil;
    currentFrame = nil;
    return [super init];
}

- (void)dealloc
{
    if (currentFrame)
        CVPixelBufferRelease(currentFrame);
    [super dealloc];
}


- (CVTexture *)getTexture
{
    CVTexture *ret;
    ret = [super getTexture];
    return ret;
}


- (CVReturn)renderFrame
{
    return kCVReturnError;

    [lock lock];
    [self renderPreview];
    if (layer)
        layer->vbuffer = currentFrame;
    [lock unlock];
    return kCVReturnSuccess;
}

- (BOOL)togglePlay
{
    return NO;
}

@end
