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


CVCocoaLayer::CVCocoaLayer(Layer *lay, CVLayerController *vin)
{
    input = vin;
    bufsize = 0;
    blendMode = NULL;
    layer = lay;
    if (input)
        [input setLayer:this];
    if (layer) {
        layer->set_name(input ? [input name] : "CVCocoaLayer");
        layer->set_data((void *)this); // XXX
    }
}

CVCocoaLayer::~CVCocoaLayer()
{
    deactivate();
    //[input togglePlay:nil];
    [input setLayer:nil];
}

/*
void 
CVLayer::run()
{
    feed();
    [input renderPreview];
}
*/

void
CVCocoaLayer::activate()
{
    if (layer && !layer->active) {
        layer->opened = true;
        layer->active = true;
        layer->set_data((void *)this);
        notice("Activating %s", layer->name);
        layer->start();
    }
}

void
CVCocoaLayer::deactivate()
{
    if (layer) {
        if (layer->screen) {
            layer->screen->rem_layer(layer);
            layer->screen = NULL;
        }
        layer->active = false;
    }
}

// accessor to get a texture from the CVLayerView cocoa class
CVTexture * 
CVCocoaLayer::gl_texture()
{
    if (input)
        return [input getTexture];
    return NULL;
}

bool CVCocoaLayer::is_active()
{
    if (layer)
        return layer->active;
    return false;
}

bool CVCocoaLayer::is_visible()
{
    if (layer)
        return layer->screen?true:false;
    return false;
}

Layer *CVCocoaLayer::fj_layer()
{
    return layer;
}

void CVCocoaLayer::set_controller(CVLayerController *vin)
{
    input = vin;
    if (input) {
        [input setLayer:this];
    }
}
