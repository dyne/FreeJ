//
//  CVFFmpegLayer.mm
//  freej
//
//  Created by xant on 3/27/10.
//  Copyright 2010 dyne.org. All rights reserved.
//

#import "CVFFmpegLayer.h"
#include "CVLayer.h"
#include <libavutil/pixfmt.h>

extern "C"
{
#include "ffdec.h"
}

CVFFmpegLayer::CVFFmpegLayer(CVLayerController *controller) : CVLayer(controller)
{
    ff = NULL;

}

CVFFmpegLayer::~CVFFmpegLayer()
{
}

bool CVFFmpegLayer::open(const char *movie)
{
    if (ff) {
        close_and_free_ff(&ff);
        ff=NULL;
    }
    ffdec_thread((void **)&ff, (char *)movie, geo.w, geo.h, PIX_FMT_ARGB);
    return true;
}

void *CVFFmpegLayer::feed()
{
    if (ff) {
        uint8_t *ffbuffer = get_bufptr(ff);
        if (!ffbuffer)
            return NULL;
        // TODO - handle geometry changes and ensure using the same size of ffmpeg buffer
        if (!currentFrame)
            currentFrame = malloc(geo.bytesize);
        memcpy(currentFrame, ffbuffer, geo.bytesize);
        if (pixelBuffer)
            CVPixelBufferRelease(pixelBuffer);
        CVReturn err = CVPixelBufferCreateWithBytes (
                                                     NULL,
                                                     geo.w,
                                                     geo.h,
                                                     k32ARGBPixelFormat,
                                                     currentFrame,
                                                     geo.w*4,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     &pixelBuffer
                                                     ); 
        if (err == kCVReturnSuccess) {
            [input feedFrame:pixelBuffer];
            if (!ffdec_thread((void **)&ff, NULL, 0, 0, 0) && (void *)ff)
                buffer = NULL;
        } else {
            // TODO - Error messages
        }
    } else {
        currentFrame = NULL;
    }
    return currentFrame;
}

bool CVFFmpegLayer::isDecoding()
{
    return (ff && (get_pt_status(ff)&3) == 1)
           ? true
           : false;
}

int CVFFmpegLayer::scaledWidth()
{
    return get_scaled_width((void *)ff);
}

int CVFFmpegLayer::scaledHeight()
{
    return get_scaled_height((void *)ff);
}

bool CVFFmpegLayer::hasFF()
{
    return ff ? true : false;
}
