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

#ifndef __CVLAYER_H__
#define __CVLAYER_H__

#include <layer.h>
#include <context.h>

#import <CFreej.h>
#import <CVLayerController.h>
#import <CVTexture.h>
#import <CVCocoaLayer.h>


class CVLayer: public Layer, public CVCocoaLayer
{
    private:
        virtual bool _init();
        virtual void *feed();
    public:
        CVLayer();
        CVLayer(CVLayerController *vin);

        virtual ~CVLayer();
        virtual bool open(const char *path);
        virtual void close();
        //void run();
        virtual bool forward();
        virtual bool backward();
        virtual bool backward_one_keyframe();
        
        virtual bool relative_seek(double increment);
        
        virtual bool set_mark_in();
        virtual bool set_mark_out();
        
        virtual void pause();
};

#endif
