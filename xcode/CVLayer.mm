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
	frame = NULL;
	pixelBuffer = NULL;
}

CVLayer::CVLayer(CVLayerController *vin) : Layer(), CVCocoaLayer(this, vin)
{
    type = Layer::GL_COCOA;
    buffer = NULL;
	frame = NULL;
	pixelBuffer = NULL;
	set_name([input name]);
    [input setLayer:this];
}

CVLayer::~CVLayer()
{
    stop();
    close();
    if (frame)
		free(frame);
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
	
	CVPixelBufferRef newPixelBuffer = [input currentFrame];
	if (newPixelBuffer) {
		if (pixelBuffer)
			CVPixelBufferRelease(pixelBuffer);
		pixelBuffer = newPixelBuffer;
		CVPixelBufferLockBaseAddress(pixelBuffer, 0);
		output = CVPixelBufferGetBaseAddress(pixelBuffer);
		CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
	}

    [pool release];
    return output;
}

void *CVLayer::do_filters(void *buf) {
    bool no_opt = false;
    
    if( filters.len() ) {
		// If we have filters to apply , we don't need to copy the frame
		// since we will get a different buffer out of the filter chain
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
    } else {
		// We are now at the end of video pipeline for a layer
		// if there is no filter to apply we are still referencing 
		// the storage from the underlying CVLayerController.
		// Note that the controller could reuse the buffer to render
		// next frame and we need to make a copy here.
		// TODO - we really need an internal buffer pool to keep track
		//        of referenced video-buffers provided by the layers
		if (!frame)
			frame = malloc(geo.bytesize);
		// XXX - remove this copy as soon as we have a proper video-buffer pool
		memcpy(frame, buf, geo.bytesize);
		buf = frame;
	}
	if ([input respondsToSelector:@selector(frameFiltered:)])
		[input frameFiltered:(void *)buf];

    return buf;
}
