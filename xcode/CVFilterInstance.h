/*
 *  CVFilterInstance.h
 *  freej
 *
 *  Created by xant on 5/23/10.
 *  Copyright 2010 dyne.org. All rights reserved.
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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
 * "$Id:$"
 *
 */

#ifndef __CVFILTER_INSTANCE_H__
#define __CVFILTER_INSTANCE_H__

#include <config.h>
#ifdef WITH_COCOA
#include <filter_instance.h>
#import <QuartzCore/CIFilter.h>
#import <QuartzCore/CIContext.h>
#include <factory.h>

class Filter;

class CVFilterInstance : public FilterInstance {
    friend class Filter;
    
public:
    CVFilterInstance();
    CVFilterInstance(Filter *fr);
    virtual ~CVFilterInstance();
    void init(Filter *fr);
    CIFilter *get_filter();
    CIContext *get_context();
    
protected:
    void set_layer(Layer *lay);

private:
    CIImage *image;
    CVFilterInstance *nextFilter;
    CGContextRef cgContextRef;
    CIContext *ciContext;
    CIFilter *ciFilter;
    FACTORY_ALLOWED
};


#endif // WITH_COCOA

#endif
