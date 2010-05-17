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
    type = Layer::GL_COCOA;
    pixelBuffer = NULL;
    currentFrame = NULL;
    bufsize = 0;
    blendMode = [[NSString stringWithFormat:@"Overlay"] retain];
    input = [[CVLayerController alloc] init];
    set_name("CVGeoLayer");
    if (input)
        set_controller(input);
}

CVGeoLayer::CVGeoLayer(CVLayerController *vin) : CVCocoaLayer(this, vin), GeoLayer()
{
    type = Layer::GL_COCOA;
    pixelBuffer = NULL;
    currentFrame = NULL;
    bufsize = 0;
    blendMode = [[NSString stringWithFormat:@"Overlay"] retain];
    input = [vin retain];
    set_name("CVGeoLayer");
    if (input)
        set_controller(input);
}

CVGeoLayer::~CVGeoLayer()
{
    if (input)
        [input release];
    if (pixelBuffer)
        CVPixelBufferRelease(pixelBuffer);
}

// ensure calling the start method from our CVLayer ancestor
// (and not from the GeoLayer one
int CVGeoLayer::start()
{
    return GeoLayer::start();
}

void *
CVGeoLayer::feed()
{    
    if (surf) {
        // TODO - handle geometry changes
        if (!pixelBuffer || currentFrame != surf->pixels) {
            currentFrame = surf->pixels;
            if (pixelBuffer)
                CVPixelBufferRelease(pixelBuffer);
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
        }

        if (input)
            [input feedFrame:pixelBuffer];
    }
    return surf->pixels;
}

