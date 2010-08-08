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
#include <QuartzCore/QuartzCore.h>
#include "CVFilter.h"
#include "CVFilterInstance.h"
#include <linklist.h>
#include <layer.h>
#import "CVTexture.h"

#ifdef WITH_COCOA

#define FILTERS_MAX 18

static char *fNames[FILTERS_MAX] = {
    (char *)"ZoomBlur",
    (char *)"BoxBlur",
    (char *)"DiscBlur",
    (char *)"GaussianBlur",
    (char *)"ColorPosterize",
    (char *)"ColorInvert",
    (char *)"ComicEffect",
    (char *)"Crystallize",
    (char *)"Edges",
    (char *)"EdgeWork",
    (char *)"HueAdjust",
    (char *)"HexagonalPixellate",
    (char *)"HoleDistorsion",
    //(char *)"BumpDistorsion",
    (char *)"CircleSplashDistortion",
    (char *)"CircularWrap",
    (char *)"PinchDistortion",
    (char *)"TwirlDistortion",
    (char *)"VortexDistortion"
};

static FilterParams fParams[FILTERS_MAX] =
{
    { 1, { { (char*)"inputAmount", 0.0, 50.0 } } },  // ZoomBlur
    { 1, { { (char*)"inputRadius", 1.0, 100.0 } } },  // BoxBlur
    //{ 2, { { "inputRadius", 0.0, 50.0 }, { "inputAngle", -3.14, 3.14 } } }, // MotionBlur
    { 1, { { (char*)"inputRadius", 0.0, 50.0 } } }, // DiscBlur
    { 1, { { (char*)"inputRadius", 0.0, 100.0 } } }, // GaussianBlur
    { 1, { { (char*)"inputLevels", 2.0, 30.0 } } }, // ColorPosterize
    { 0, { { NULL, 0.0, 0.0  } } }, // ColorInvert
    { 0, { { NULL, 0.0, 0.0 } } }, // ComicEffect
    { 3, { { (char*)"CenterX", 0.0, 100.0 }, { (char*)"CenterY", 0.0, 100.0 }, { (char*)"inputRadius", 1.0, 100.0 } } }, // Crystallize
    { 1, { { (char*)"inputIntensity", 0.0, 10.0 } } }, // Edges
    { 1, { { (char*)"inputRadius", 0.0, 20.0 } } }, // EdgeWork
    { 1, { { (char*)"inputAngle", -3.14, 3.14 } } }, // HueAdjust
    { 3, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputScale", 1.0, 100.0 } } }, // HexagonalPixellate
    { 3, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.01, 1000.0 } } }, // HoleDistortion
    //{ 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 600.0 }, { "inputScale", -1.0, 1.0 } } }, // BumpDistortion
    { 3, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.00, 1000.0 } } }, // CircleSplashDistortion
    { 4, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.00, 600.0 }, { (char*)"inputAngle", -3.14, 3.14 } } }, // CircularWrap
    { 4, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.00, 1000.0 }, { (char*)"inputScale", 0.0, 1.0 } } }, // PinchDistortion
    { 4, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.00, 500.0 }, { (char*)"inputAngle", -12.57, 12.57 } } }, // TwirlDistortion
    { 4, { { (char*)"CenterX", 0.0, 800.0 }, { (char*)"CenterY", 0.0, 600.0 }, { (char*)"inputRadius", 0.00, 800.0 }, { (char*)"inputAngle", -94.25, 94.25 } } }, // VortexDistortion
};

static void getParameter(FilterInstance *filt, Parameter *param, int idx) {
    CVFilter *filter = (CVFilter *)filt->proto;
    CVFilterInstance *instance = (CVFilterInstance *)filt;
    CIFilter * ciFilter = instance->get_filter();
    NSNumber *value = [ciFilter valueForKey:[NSString stringWithUTF8String:param->name]];
    *(double *)param->value = [value doubleValue];
}

static void setParameter(FilterInstance *filt, Parameter *param, int idx) {
    CVFilter *filter = (CVFilter *)filt->proto;
    CVFilterInstance *instance = (CVFilterInstance *)filt;
    CIFilter * ciFilter = instance->get_filter();
    filter->parameters.lock();
    if (strcmp(param->name, "CenterX") == 0) {
        Parameter *centerY = filter->parameters.begin();
        while (centerY && strcmp(centerY->name, "CenterY") != 0) {
            centerY = (Parameter *)centerY->next;
        }
        if (centerY) {
            [ciFilter setValue:[CIVector vectorWithX:[[NSNumber numberWithDouble:*(double *)param->value] floatValue]
                                                   Y:[[NSNumber numberWithDouble:*(double *)centerY->value] floatValue]]
                        forKey:@"inputCenter"];
        } else {
            error("Can't find 'CenterY' param for filter %s\n", filter->name);
        }
    } else if (strcmp(param->name, "CenterY") == 0) {
        Parameter *centerX = filter->parameters.begin();
        while (centerX && strcmp(centerX->name, "CenterX") != 0) {
            centerX = (Parameter *)centerX->next;
        }
        if (centerX) {
            [ciFilter setValue:[CIVector vectorWithX:[[NSNumber numberWithDouble:*(double *)centerX->value] floatValue]
                                                   Y:[[NSNumber numberWithDouble:*(double *)param->value] floatValue]]
                        forKey:@"inputCenter"];
        } else {
            error("Can't find 'CenterY' param for filter %s\n", filter->name);
        }    } else {
        [ciFilter setValue:[NSNumber numberWithDouble:*(double *)param->value]
                    forKey:[NSString stringWithUTF8String:param->name]];        
    }
    filter->parameters.unlock();
}

void CVFilter::listFilters(Linklist<Filter> &outputList) {
    for (int i = 0; i < FILTERS_MAX; i++) {
        CVFilter *filt = new CVFilter();
        filt->open(fNames[i]);
        outputList.append((Filter *)filt);
    }
}

CVFilter::CVFilter()
{
    opened = false;
    //filterDescr = NULL;
    //filterName = NULL;
    set_name((char *)"Unknown");
}

CVFilter::~CVFilter()
{
 //   if (filterName)
   //     free(filterName)
  //  if (filterDescr)
    //    free(filterDescr);
}

int CVFilter::type()
{
    return Filter::COREIMAGE;
}

int CVFilter::open(char *name)
{
    int i;
    
    if (opened) {
        // TODO - Error messages
        return 0;
    }
    //filterName = [[NSString stringWithFormat:@"CI%s", name] retain];
    set_name(name);
    for (i = 0; i < FILTERS_MAX; i++) {
        if (strcmp(fNames[i], name) == 0) {
            desc = &fParams[i];
            opened = true;
        }
    }
    if (opened && desc) {
        // Get the list of params.
        for (i = 0; i < desc->nParams; i++) {
            Parameter *param = new Parameter(Parameter::NUMBER);
            snprintf(param->name, 255, "%s", desc->params[i].label);
            func("registering parameter %s for filter %s\n", param->name, name);
            
            snprintf(param->description, 512, "%s", desc->params[i].label);
            param->filter_set_f = setParameter;
            param->filter_get_f = getParameter;
            *(double *)param->min_value = desc->params[i].min;
            *(double *)param->max_value = desc->params[i].max;
            parameters.append(param);
        }
        return 1;
    }
    //[ciFilter setName:[[sender selectedItem] title]];
    return 0;
}

bool CVFilter::apply(Layer *layer, FilterInstance *instance)
{
    if (Filter::apply(layer, instance)) {
        if (layer->type == Layer::GL_COCOA) {
            CVFilterInstance *cvInst = (CVFilterInstance *)instance;            
            CGRect bounds = CGRectMake(0, 0, layer->geo.w, layer->geo.h);
            CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            
            CGContextRef context = CGBitmapContextCreateWithData(
                                                                 instance->outframe,
                                                                 layer->geo.w,
                                                                 layer->geo.h,
                                                                 8,
                                                                 layer->geo.w*4,
                                                                 colorSpace,
                                                                 kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little,
                                                                 NULL,
                                                                 NULL
                                                                 );
            if (!context) {
                // todo - error messages
                return false;
            }
            NSDictionary *ciContextOptions = [NSDictionary dictionaryWithObject:(NSString*)kCGColorSpaceGenericRGB 
                                                                         forKey: kCIContextOutputColorSpace];
            cvInst->ciContext = [[CIContext contextWithCGContext:context options:ciContextOptions] retain];
            CGContextRelease(context);
        } else {
        }
        return true;
    }
    return false;
}

const char *CVFilter::description()
{
    //return (char *)[[CIFilter localizedDescriptionForFilterName:[NSString stringWithUTF8String:name]] UTF8String];
    return NULL;
}

void CVFilter::print_info()
{
    notice("Name             : %s", name);
    act("%s",description());
    act("Type             : CoreImage Filter");
    /*
    NSArray *paramNames = [cifilter inputKeys];
    act("Parameters [%i total]", [paramNames count]);
    for (int i = 1; i <= [paramNames count]; ++i) {
        char tmp[256];
        snprintf(tmp,255,"  [%i] %s ",i-1, [(NSString *)[paramNames objectAtIndex:i] UTF8String]);
        act("%s (double) %s",tmp, param_infos[i].explanation);
    }
    */
}

char *CVFilter::get_parameter_description(int i)
{
    if (i >= desc->nParams)
        return NULL;
    return desc->params[i].label;
}

void CVFilter::destruct(FilterInstance *inst)
{
    CVFilterInstance *cvInst = (CVFilterInstance *)inst;
    if (cvInst->ciContext)
        [cvInst->ciContext release];
    return Filter::destruct(inst);
}

void CVFilter::update(FilterInstance *inst, double time, uint32_t *inframe, uint32_t *outframe) {
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Filter::update(inst, time, inframe, outframe);
    CVFilterInstance *cvInst = (CVFilterInstance *)inst; // XXX - highly unsafe ... find a proper solution
    // first wrap the input buffer in a CVPixelBuffer
    CVPixelBufferRef pixelBufferIn;
    Layer *layer = inst->get_layer();
    if (layer) {
        CVReturn cvRet = CVPixelBufferCreateWithBytes (
                                                       NULL,
                                                       layer->geo.w,
                                                       layer->geo.h,
                                                       k32ARGBPixelFormat,
                                                       inframe,
                                                       layer->geo.w*4,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       &pixelBufferIn
                                                       );
        if (cvRet != noErr) {
            // TODO - Error Messages
        } 
        // than create a CIImage to use as input-image
        CIImage *inputImage = [CIImage imageWithCVImageBuffer:pixelBufferIn];
        
        CIFilter *ciFilter = cvInst->get_filter();
        // apply the filter
        [ciFilter setValue:inputImage forKey:@"inputImage"];
        CIImage *outputImage = [ciFilter valueForKey:@"outputImage"];
        
        // draw the result image in the given output buffer
        CGRect bounds = CGRectMake(0, 0, layer->geo.w, layer->geo.h);
          
        CIContext *ctx = cvInst->get_context();

        [ctx render:outputImage 
           toBitmap:outframe
           rowBytes:layer->geo.w*4
             bounds:bounds
             format:kCIFormatARGB8 
         colorSpace:nil];

        CVPixelBufferRelease(pixelBufferIn);
    }
    [pool release];
}


#endif
