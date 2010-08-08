//
//  CVFFmpegLayer.mm
//  freej
//
//  Created by xant on 3/27/10.
//  Copyright 2010 dyne.org. All rights reserved.
//

#import "CVFFmpegLayer.h"
#import "CVFFmpegLayerController.h"
#include "CVLayer.h"
#include <libavutil/pixfmt.h>

extern "C"
{
#include "ffdec.h"
}

CVFFmpegLayer::CVFFmpegLayer(CVLayerController *controller) : CVLayer(controller)
{
    ff = NULL;
    pixelBuffer = NULL;
}

CVFFmpegLayer::~CVFFmpegLayer()
{
}

bool CVFFmpegLayer::open(const char *movie)
{
    if (ff)
        close_and_free_ff(&ff);
    strncpy(filename, movie, sizeof(filename)-1);
    return true;
}

void *CVFFmpegLayer::feed()
{
    if (!ff) 
    {
        if (strlen(filename)) { // XXX - (argh!, find a better way)

            if (open_movie(&ff, filename) == 0) {
                init_moviebuffer(ff, geo.w, geo.h, PIX_FMT_ARGB);
                decode_frame(ff);
            } else {
                close_and_free_ff(&ff);
                if (![(CVFFmpegLayerController *)input wantsRepeat]) {
                    memset(filename, 0, sizeof(filename)); // XXX
					[input stop];
					return NULL;
				}
            }
        } else {
			[input stop];
			return NULL;
		}
    } 
	uint8_t *ffbuffer = get_bufptr(ff);
	if (!ffbuffer)
		return NULL;
	CVReturn err = CVPixelBufferCreateWithBytes (
												 NULL,
												 geo.w,
												 geo.h,
												 k32ARGBPixelFormat,
												 ffbuffer,
												 geo.w*4,
												 NULL,
												 NULL,
												 NULL,
												 &pixelBuffer
												); 
	if (err == kCVReturnSuccess) {
		// first decode the frame
		if (!decode_frame(ff)) {
			close_and_free_ff(&ff);
			//buffer = NULL;
			// XXX - find a cleaner way instead of blindly resetting the filename
			// TODO - allow looping on a stream by reopening it
			if (![input wantsRepeat]) {
				memset(filename, 0, sizeof(filename));
				if (input) {
					[(CVFFmpegLayerController *)input deactivate];
					[(CVFFmpegLayerController *)input clearPreview];
				}
			}
		}
		// and than provide it to our controller
		[input feedFrame:pixelBuffer];
		CVPixelBufferRelease(pixelBuffer);

	} else {
		// TODO - Error messages
	}
    return CVLayer::feed();
}

bool CVFFmpegLayer::isDecoding()
{
    return (ff && (get_pt_status(ff)&3) == 1)
           ? true
           : false;
}

int CVFFmpegLayer::scaledWidth()
{
	if (ff)
		return get_scaled_width(ff);
	return 0;
}

int CVFFmpegLayer::scaledHeight()
{
	if (ff)
		return get_scaled_height(ff);
	return 0;
}

bool CVFFmpegLayer::hasFF()
{
    return ff ? true : false;
}

void
CVFFmpegLayer::close()
{
    lock();
    close_and_free_ff(&ff);
    //buffer = NULL;
    memset(filename, 0, sizeof(filename));
    if (input) {
        [(CVFFmpegLayerController *)input deactivate];
        [(CVFFmpegLayerController *)input clearPreview];
    }
    unlock();
}

