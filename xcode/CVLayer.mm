/*
 *  CVLayer.cpp
 *  freej
 *
 *  Created by xant on 2/23/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "Cocoa/Cocoa.h";
#include "CVLayer.h"

CVLayer::CVLayer(NSObject *vin) : Layer()
{
    input = vin;
    bufsize = 0;
    blendMode = NULL;
    type = Layer::GL_COCOA;
    data = (void *)this;
    [input setLayer:this];
    set_name([[(NSView *)input toolTip] UTF8String]);
}

CVLayer::~CVLayer()
{
    close();
    deactivate();
    [input togglePlay:nil];
    [input setLayer:nil];
}

void
CVLayer::activate()
{
    freej->add_layer(this);
    active = true;
    notice("Activating %s", name);
    //blitter.set_blit("RGB");
    //blitter.current_blit->lay
}

void
CVLayer::deactivate()
{
    freej->rem_layer(this);
    active = false;
}

bool
CVLayer::open(const char *path)
{
    //[input openFile: path];
    return false;
}

bool
CVLayer::init(Context *ctx)
{
     // * TODO - probe resolution of the default input device
    return init(ctx, ctx->screen->w, ctx->screen->h);
}

bool
CVLayer::init(Context *ctx, int w, int h)
{
    width = w;
    height = h;
    freej = ctx;
    _init(width,height);
    return true;
}

void *
CVLayer::feed()
{
    unsigned long tick = 0;
    //Delay(1, &tick);
    return [input grabFrame];
}

void
CVLayer::close()
{
    //[input release];
}

bool
CVLayer::forward()
{
    [input stepForward];
    return true;
}

bool
CVLayer::backward()
{
    [input stepBackward];
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
    [input stop];
}

bool
CVLayer::relative_seek(double increment)
{
    return false;
}

CIImage * 
CVLayer::gl_texture()
{
    return [input getTexture];
}
