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

#import <CVGeoLayer.h>
#import <CFreej.h> 
#include <jutils.h>

FACTORY_REGISTER_INSTANTIATOR(Layer, CVGeoLayer, GeometryLayer, cocoa);

CVGeoLayer::CVGeoLayer()
    : GeoLayer(), CVCocoaLayer(this)
{
    frame = NULL;
    type = Layer::GL_COCOA;
    blendMode = [[NSString stringWithFormat:@"Overlay"] retain];
    input = [[CVLayerController alloc] init]; // create a new layer-controller
    set_name("CVGeoLayer"); // and set our name
    if (input)
        setController(input);
}

CVGeoLayer::CVGeoLayer(CVLayerController *vin) : CVCocoaLayer(this, vin), GeoLayer()
{
    frame = NULL;
    type = Layer::GL_COCOA;
    blendMode = [[NSString stringWithFormat:@"Overlay"] retain];
    input = [vin retain];
    set_name("CVGeoLayer");
    if (input)
        setController(input);
}

CVGeoLayer::~CVGeoLayer()
{
    if (input)
        [input release];
    if (frame)
        free(frame);
}

void *
CVGeoLayer::feed()
{    
    if (surf && input) {
        CVPixelBufferRef pixelBuffer;
        // TODO - handle geometry changes
        CVPixelBufferCreateWithBytes (NULL,
                                      geo.w,
                                      geo.h,
                                      k32ARGBPixelFormat,
                                      surf->pixels,
                                      geo.w*4,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &pixelBuffer
                                     );

        [input feedFrame:pixelBuffer];
        CVPixelBufferRelease(pixelBuffer);
    }
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
    CVPixelBufferRef pixelBuffer = [input currentFrame];
    if (pixelBuffer) {
        CVPixelBufferLockBaseAddress(pixelBuffer, 0);
        
        //void *frame = CVPixelBufferGetBaseAddress(pixelBuffer);
        if (!frame)
            frame = malloc(CVPixelBufferGetDataSize(pixelBuffer));
        // TODO - try to avoid this copy!!
		//lock();
        memcpy(frame, CVPixelBufferGetBaseAddress(pixelBuffer), CVPixelBufferGetDataSize(pixelBuffer));
        //unlock();
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
		CVPixelBufferRelease(pixelBuffer);
	}
    [pool release];
    return frame;
}

void *CVGeoLayer::do_filters(void *buf) {
    int cnt = 0;
    bool no_opt = false;
    
    if( filters.len() ) {
        FilterInstance *filt;
        filters.lock();
        filt = (FilterInstance *)filters.begin();
        while(filt) {
            if(filt->active) {
                buf = filt->process(fps.fps, (uint32_t *)buf);
            }
            filt = (FilterInstance *)filt->next;
        }
        filters.unlock();
    }
    // now that we have applied filters (if any)
	if ([input respondsToSelector:@selector(frameFiltered:)])
		[input frameFiltered:(void *)buf];
    return buf;
}
