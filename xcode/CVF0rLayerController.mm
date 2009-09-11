//
//  CVF0rLayerController.mn
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayerController.h"


@implementation CVF0rLayerController : CVLayerController


- (id)init
{
    NSLog(@"MADONNA!!! \n");

    return [super init];
}

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

- (void)reset
{
    CVLayer *toDelete;
    if (layer) {
        [lock lock];
        if (currentFrame) {
            CVPixelBufferRelease(currentFrame);
            currentFrame = NULL;
        }
        toDelete = (CVLayer *)layer;
        layer = NULL;
        [lock unlock];
        toDelete->stop();
        delete toDelete;
    }
}

- (void)setLayer:(CVLayer *)lay
{
    if (layer) // ensure to remove/stop old genf0rlayer if we are setting a new one
        [self reset];
    [super setLayer:lay];
}

- (char *)name {
    if (layerView)
        return (char *)[[layerView toolTip] UTF8String];
    return (char *)"F0R";
}

@end
