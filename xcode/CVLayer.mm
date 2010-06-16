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

#include <CVLayer.h>

CVLayer::CVLayer() : CVCocoaLayer(this), Layer()
{
    type = Layer::GL_COCOA;
    buffer = NULL;
}

CVLayer::CVLayer(CVLayerController *vin) : Layer(), CVCocoaLayer(this, vin)
{
    type = Layer::GL_COCOA;
    buffer = NULL;
    set_name([input name]);
    [input setLayer:this];
}

CVLayer::~CVLayer()
{
    stop();
    close();
}

bool
CVLayer::open(const char *path)
{
    // must be overridden if providing open() to upper layers
    return false;
}

bool
CVLayer::_init()
{
    return start();
}

void
CVLayer::close()
{
    //[input release];
}

bool
CVLayer::forward()
{
    if ([input respondsToSelector:@selector(stepForward)])
        [(id)input stepForward];
    return true;
}

bool
CVLayer::backward()
{
    if ([input respondsToSelector:@selector(stepForward)])
        [(id)input stepBackward];
    return true;
}

bool
CVLayer::backward_one_keyframe()
{
    return backward();
}

bool
CVLayer::set_mark_in()
{
    return false;
}

bool
CVLayer::set_mark_out()
{
    return false;
}

void
CVLayer::pause()
{
    if ([input respondsToSelector:@selector(pause)])
        [(id)input pause];
}

bool
CVLayer::relative_seek(double increment)
{
    return false;
}

void *
CVLayer::feed()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    lock();
    //if ([input isVisible] || [input needPreview])
    [input renderFrame];    
    CVTexture *texture = [input getTexture];
    if (pixelBuffer)
        CVPixelBufferRelease(pixelBuffer);
    pixelBuffer = CVPixelBufferRetain([texture pixelBuffer]);
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    void *buf = CVPixelBufferGetBaseAddress(pixelBuffer);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    unlock();

    [pool release];
    return buf;
}

void *CVLayer::do_filters(void *buf) {
    int cnt = 0;
    bool no_opt = false;
    
    if( filters.len() ) {
        FilterInstance *filt;
        filters.lock();
        filt = (FilterInstance *)filters.begin();
        while(filt) {
            if(filt->active)
                [input filterFrame:filt];
            filt = (FilterInstance *)filt->next;
        }
        filters.unlock();
    }
    // now that we have applied filters (if any)
    // we can render the preview (if needed)
    [input renderPreview];
    return buf;
}
