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
    [input setLayer:this];
    set_name([input name]);
}

CVLayer::~CVLayer()
{
    stop();
    close();
}

/*
void 
CVLayer::run()
{
    feed();
    [input renderPreview];
}
*/


bool
CVLayer::open(const char *path)
{
    //[input openFile: path];
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
    lock();
    if ([input isVisible] || [input needPreview])
        [input renderFrame];
    unlock();
    return (void *)vbuffer;
}
