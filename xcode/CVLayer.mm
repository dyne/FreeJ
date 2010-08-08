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
	memset(frame, 0, sizeof(void *) * 2);
    num = 0;
}

CVLayer::CVLayer(CVLayerController *vin) : Layer(), CVCocoaLayer(this, vin)
{
    type = Layer::GL_COCOA;
    buffer = NULL;
    memset(frame, 0, sizeof(void *) * 2);
    num = 0;
    set_name([input name]);
    [input setLayer:this];
}

CVLayer::~CVLayer()
{
    stop();
    close();
    // release the double buffer if present
    for (int i = 0; i < 2; i++) {
		if (frame[i])
			free(frame[i]);
    }
}

bool
CVLayer::open(const char *path)
{
    // must be overridden if providing open() to upper layers
    return false;
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
	void *output = NULL;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	CVPixelBufferRef pixelBuffer = [input currentFrame];
    if (pixelBuffer) {
        num++;
		int idx = num % 2;
        //lock();
        // ensure providing the pixelbuffer to upper cocoa-related layers

        CVPixelBufferLockBaseAddress(pixelBuffer, 0);

        //void *frame = CVPixelBufferGetBaseAddress(pixelBuffer);
        if (!frame[idx])
            frame[idx] = malloc(CVPixelBufferGetDataSize(pixelBuffer));
        // TODO - try to avoid this copy!!
		//lock();
        memcpy(frame[idx], CVPixelBufferGetBaseAddress(pixelBuffer), CVPixelBufferGetDataSize(pixelBuffer));
		//unlock();
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
		output = frame[idx];
		CVPixelBufferRelease(pixelBuffer);
        //unlock();
    }
    [pool release];
    return output;
}

void *CVLayer::do_filters(void *buf) {
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
	if ([input respondsToSelector:@selector(frameFiltered:)])
		[input frameFiltered:(void *)buf];

    return buf;
}
