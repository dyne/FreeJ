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
static void get_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
    CVF0rLayer *layer = (CVF0rLayer*)lay;
#ifdef WITH_FREI0R    
    Freior *f = (Freior *)layer->generator->proto;
    bool *val = (bool*)param->value;
    
    switch(f->param_infos[idx-1].type) {
            
            // idx-1 because frei0r's index starts from 0
        case F0R_PARAM_BOOL:
            (*f->f0r_set_param_value)
            (layer->generator->core, new f0r_param_bool(val[0]), idx-1);
            break;
            
        case F0R_PARAM_DOUBLE:
            (*f->f0r_set_param_value)(layer->generator->core, new f0r_param_double(val[0]), idx-1);
            break;
            
        case F0R_PARAM_COLOR:
        { f0r_param_color *color = new f0r_param_color;
            color->r = val[0];
            color->g = val[1];
            color->b = val[2];
            (*f->f0r_set_param_value)(layer->generator->core, color, idx-1);
            // QUAAA: should we delete the new allocated object? -jrml
        } break;
            
        case F0R_PARAM_POSITION:
        { f0r_param_position *position = new f0r_param_position;
            position->x = val[0];
            position->y = val[1];
            (*f->f0r_set_param_value)(layer->generator->core, position, idx-1);
            // QUAAA: should we delete the new allocated object? -jrml
        } break;
            
        default:
            
            error("Unrecognized parameter type %u for set_parameter_value",
                  f->param_infos[idx].type);
    }
#endif
}

CVF0rLayer::CVF0rLayer(CVLayerController *controller)
    : CVLayer()
{
    input = controller;
    generator = NULL;
    type = Layer::GL_COCOA;   
    set_name([input name]);
    currentFrame = NULL;
    pixelBuffer = NULL;
    [input setLayer:this];
}

CVF0rLayer::~CVF0rLayer()
{
    close();
    if (pixelBuffer)
        CVPixelBufferRelease(pixelBuffer);
    if (currentFrame)
        free(currentFrame);
}

void *
CVF0rLayer::feed()
{
    void *res;

    if (generator) 
        res = generator->process(fps.get(), NULL);

    // TODO - handle geometry changes
    if (!currentFrame) {
        currentFrame = malloc(geo.bytesize);
        CVReturn err = CVPixelBufferCreateWithBytes (
                                                     NULL,
                                                     geo.w,
                                                     geo.h,
                                                     k32ARGBPixelFormat,
                                                     currentFrame,
                                                     geo.w*4,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     &pixelBuffer
                                                     ); 
    }
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    memcpy(currentFrame, res, geo.bytesize);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
    [(CVF0rLayerController *)input feedFrame:pixelBuffer];
    return currentFrame;
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
    
    generator = new FilterInstance((Filter*)proto);
    
#ifdef WITH_FREI0R
    if(proto->type() == Filter::FREIOR) {
        generator->core = (void*)(*((Freior *)proto)->f0r_construct)(geo.w, geo.h);
        if(!generator->core) {
            error("freior constructor returned NULL instantiating generator %s",file);
            delete generator;
            generator = NULL;
            return false;
        }
        parameters = &proto->parameters;
        
        Parameter *p = (Parameter*)parameters->begin();
        while(p) {
            p->layer_set_f = set_frei0r_layer_parameter;
            p->layer_get_f = get_frei0r_layer_parameter;
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
        parameters = &proto->parameters;
        Parameter *p = (Parameter*)parameters->begin();
        while(p) {
            p->layer_set_f = set_freeframe_layer_parameter;
            p->layer_get_f = get_freeframe_layer_parameter;
            p = (Parameter*)p->next;
        }
    }
    
    generator->outframe = (uint32_t*) calloc(geo.bytesize, 1);
    
    
    set_filename(file);
    opened = true;
    return true;
}

void CVF0rLayer::close() {
    if(generator) {
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

