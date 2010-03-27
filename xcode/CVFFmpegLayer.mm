//
//  CVFFmpegLayer.mm
//  freej
//
//  Created by xant on 3/27/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
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
        CVLayer::feed();
        if (!ffdec_thread((void **)&ff, NULL, 0, 0, 0) && (void *)ff)
            ; // ifpsc+=get_fps(ff);
        buffer = get_bufptr(ff);
    }
    return buffer;
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
