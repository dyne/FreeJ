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

#include <CVF0rLayer.h>
#import <CVF0rLayerView.h>
#include <frei0r_freej.h>
#include <freeframe_freej.h>
#include <jutils.h>

static void set_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
#ifdef WITH_FREI0R
static void get_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
    Freior::get_parameter(static_cast<CVF0rLayer *>(lay)->generator, param, idx);
}
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
    Freior::set_parameter(static_cast<CVF0rLayer *>(lay)->generator, param, idx);
}
#endif

CVF0rLayer::CVF0rLayer(CVLayerController *controller)
    : CVLayer()
{
    input = controller;
    generator = NULL;
    type = Layer::GL_COCOA;   
    set_name([input name]);
    [input setLayer:this];
}

CVF0rLayer::~CVF0rLayer()
{
    close();
}

void *
CVF0rLayer::feed()
{
    void *res;
	CVPixelBufferRef pixelBuffer;
    if (generator) 
        res = generator->process(fps.get(), NULL);
    if (res) {
        // TODO - handle geometry changes
	    CVReturn err = CVPixelBufferCreateWithBytes (
													 NULL,
													 geo.w,
													 geo.h,
													 k32ARGBPixelFormat,
													 res,
													 geo.w*4,
													 NULL,
													 NULL,
													 NULL,
													 &pixelBuffer
													 );
		if (err == noErr) {
			[(CVF0rLayerController *)input feedFrame:pixelBuffer];
			CVPixelBufferRelease(pixelBuffer);
		}
    }
    return CVLayer::feed();
}

bool CVF0rLayer::open(const char *file) {
    int idx;
    Filter *proto;
    
    proto = (Filter*) generators->search(file, &idx);
    if(!proto) {
        error("generator not found: %s", file);
        return(false);
    }
    
    close();
    
    generator = Factory<FilterInstance>::new_instance("FilterInstance");
    if (!generator) return false;
    generator->init(proto);
    
#ifdef WITH_FREI0R
    if(proto->type() == Filter::FREIOR) {
        if (!static_cast<Freior *>(proto)->construct(generator, geo.w, geo.h)) {
            error("freior constructor returned NULL instantiating generator %s",file);
            delete generator;
            generator = NULL;
            return false;
        }
        parameters = &generator->parameters;
        
        Parameter *p = (Parameter*)parameters->begin();
        while(p) {
            p->layer_set_f = p->filter_set_f ? set_frei0r_layer_parameter : nullptr;
            p->layer_get_f = p->filter_get_f ? get_frei0r_layer_parameter : nullptr;
            p = (Parameter*)p->next;
        }
    }
#endif
    
    if(proto->type() == Filter::FREEFRAME) {
        VideoInfoStruct vidinfo;
        vidinfo.frameWidth = geo.w;
        vidinfo.frameHeight = geo.h;
        vidinfo.orientation = 1;
        vidinfo.bitDepth = FF_CAP_32BITVIDEO;
        generator->intcore = ((Freeframe *)proto)->plugmain(FF_INSTANTIATE, &vidinfo, 0).ivalue;
        if(generator->intcore == FF_FAIL) {
            error("Freeframe generator %s cannot be instantiated", name);
            delete generator;
            generator = NULL;
            return false;
        }
        // todo: parameters in freeframe
        generator->freeframe_active_ = true;
        parameters = &generator->parameters;
        Parameter *p = (Parameter*)parameters->begin();
        while(p) {
            p->layer_set_f = set_freeframe_layer_parameter;
            p->layer_get_f = get_freeframe_layer_parameter;
            p = (Parameter*)p->next;
        }
    }
    
    if (!generator->allocate_output(geo.bytesize)) {
        close();
        return false;
    }
    set_filename(file);
    opened = true;
    return true;
}

void CVF0rLayer::close() {
    if(generator) {
        parameters = NULL;
        delete generator;
        generator = NULL;
    }
    opened = false;
}

bool CVF0rLayer::_init() {    
    return(true);
}

void CVF0rLayer::register_generators(Linklist<Filter> *gens) {
    generators = gens;
    act("%u registered generators found", gens->len());
}

