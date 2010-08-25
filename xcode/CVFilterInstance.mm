/*
 *  CVFilter.mm
 *  freej
 *
 *  Created by xant on 5/4/10.
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

#include "CVFilterInstance.h"
#include "CVFilter.h"
#include <layer.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSString.h>
#import <QuartzCore/CIFilter.h>
#ifdef WITH_COCOA

FACTORY_REGISTER_INSTANTIATOR(FilterInstance, CVFilterInstance, FilterInstance, cocoa);

CVFilterInstance::CVFilterInstance()
    : FilterInstance()
{
    cgContextRef = nil;
    ciContext = nil;
    ciFilter = nil;
    image = nil;
}

CVFilterInstance::CVFilterInstance(Filter *fr)
	: FilterInstance()
{
    cgContextRef = nil;
    ciContext = nil;
    ciFilter = nil;
    image = nil;
    init(fr);
}

void CVFilterInstance::init(Filter *fr)
{
	FilterInstance::init(fr);
    ciFilter = [[CIFilter filterWithName:[NSString stringWithFormat:@"CI%s", fr->name]] retain];
    [ciFilter setDefaults];
    if (fr->type() == Filter::COREIMAGE) {
        Parameter *param = parameters.begin();
        while (param) {
            param->set(param->min_value);
            param = (Parameter *)param->next;
        }
    }    
}

void CVFilterInstance::set_layer(Layer *lay)
{
    FilterInstance::set_layer(lay);
    /*
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    cgContextRef = CGBitmapContextCreate (NULL,
                                          lay->geo.w,
                                          lay->geo.h,
                                          8,      // bits per component
                                          lay->geo.w*4,
                                          colorSpace,
                                          kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    if (cgContextRef == NULL)
        NSLog(@"WARNING: filter-CI-context not created!");
    */
}

CVFilterInstance::~CVFilterInstance()
{
    if (image)
        [image release];
    /*
    if (ciContext)
        [ciContext release];
    if (cgContextRef)
        CGContextRelease(cgContextRef);
     */
}

CIFilter *CVFilterInstance::get_filter()
{
    return ciFilter;
}

CIContext *CVFilterInstance::get_context()
{
    return ciContext;
}

#endif