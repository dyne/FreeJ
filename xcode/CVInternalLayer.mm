/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
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

#import "CVInternalLayer.h"

@implementation CVInternalLayerView : CVLayerController

- (void)feedFrame:(void *)frame
{
    CVPixelBufferRef newPixelBuffer;
    //Context *ctx = (Context *)[freej getContext];
    [lock lock];
    CVReturn err = CVPixelBufferCreateWithBytes (
         NULL,
         layer->geo.w,
         layer->geo.h,
         k32ARGBPixelFormat,
         frame,
         layer->geo.w*4,
         NULL,
         NULL,
         NULL,
         &newPixelBuffer
    ); 
    if (err == kCVReturnSuccess) {
        if (currentFrame)
            CVPixelBufferRelease(currentFrame);
            currentFrame = newPixelBuffer;
            newFrame = YES;
            }
    [lock unlock];
    [self renderPreview];
}

@end
CVInternalLayer::CVInternalLayer(CVInternalLayerView *view, Layer *lay) : CVLayer(view)
{
    layer = lay;
}

CVInternalLayer::~CVInternalLayer()
{
}

void CVInternalLayer::attach(Layer *lay)
{
    lock();
    layer = lay;
    unlock();
}

bool CVInternalLayer::init(Context *ctx) {
    freej = ctx;
    width = layer->geo.w;
    height = layer->geo.h;
    return true;
}

bool CVInternalLayer::init(Context *ctx, int w, int h)
{
    return false;
}

void *
CVInternalLayer::feed()
{
    layer->lock();
    buffer = layer->buffer;
    layer->unlock();
    if (buffer) 
        [(CVInternalLayerView *)input feedFrame:buffer]; 
    return buffer;
}

