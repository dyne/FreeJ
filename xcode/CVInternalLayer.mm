//
//  CVInternalLayer.m
//  freej
//
//  Created by xant on 6/14/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVInternalLayer.h"

@implementation CVInternalLayerView : CVLayerView

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

