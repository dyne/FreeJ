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

#ifndef __CVCOCOALAYER_H__
#define __CVCOCOALAYER_H__

#import <CFreej.h>
#import <CVLayerController.h>
#include <geometry.h>

@class CVLayerView;
@class CVLayerController;

class Layer;

class CVCocoaLayer 
{
public:
    CVLayerController *input;
    NSString *blendMode;

    CVCocoaLayer(Layer *lay, CVLayerController *vin = NULL);

    virtual ~CVCocoaLayer();
    void activate();
    void deactivate();
    bool isActive();
    bool isVisible();
    Layer *fjLayer();
    char *fjName();

    Context *context() { return freej; }; 
    virtual CVPixelBufferRef currentFrame();
    NSDictionary *imageParams();
	int width();
	int height();
	void setOrigin(int x, int y);


protected:
    Layer *layer;
    Context *freej;
    
    void setController(CVLayerController *vin);
    
};

#endif
