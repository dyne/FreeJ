//
//  CVF0rLayer.m
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#include "CVF0rLayer.h"
#import "CVF0rLayerView.h"
#include <frei0r_freej.h>
#include <freeframe_freej.h>
#include <jutils.h>

static void set_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_freeframe_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void get_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) { }
static void set_frei0r_layer_parameter(Layer *lay, Parameter *param, int idx) {
    CVF0rLayer *layer = (CVF0rLayer*)lay;
    
    Freior *f = layer->generator->proto->freior;
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
}

CVF0rLayer::CVF0rLayer(CVLayerView *view, char *generatorName, Context *_freej)
    : CVLayer()
{
    input = view;
    freej = _freej;
    generator = NULL;
    
    type = Layer::F0R_GENERATOR;
    set_name("F0R");
    //jsclass = &gen0r_layer_class;
    //  set_filename("/particle generator");    
    init(freej, freej->screens.begin()->geo.w, freej->screens.begin()->geo.h);
    if (!open(generatorName)) {
        error("generator %s hasn't been found", generatorName);
        return;
    }
    set_name([[view toolTip] UTF8String]);
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
    if (generator) 
        res = generator->process(fps.get(), NULL);

    [(CVF0rLayerView *)input feedFrame:res]; 
    return res;
}

bool CVF0rLayer::open(const char *file) {
    int idx;
    Filter *proto;
    
    proto = NULL; //(Filter*) generators.search(file, &idx);
    if(!proto) {
        error("generator not found: %s", file);
        return(false);
    }
    
    close();
    
    generator = new FilterInstance((Filter*)proto);
    
#ifdef WITH_FREI0R
    if(proto->freior) {
        generator->core = (void*)(*proto->freior->f0r_construct)(geo.w, geo.h);
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
    
    if(proto->freeframe) {
        VideoInfoStruct vidinfo;
        vidinfo.frameWidth = geo.w;
        vidinfo.frameHeight = geo.h;
        vidinfo.orientation = 1;
        vidinfo.bitDepth = FF_CAP_32BITVIDEO;
        generator->intcore = proto->freeframe->main(FF_INSTANTIATE, &vidinfo, 0).ivalue;
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

bool CVF0rLayer::init(Context *freej) {
    int width  = freej->screen->geo.w;
    int height = freej->screen->geo.h;
    
    //env = freej; 
    
    /* internal initalization */
    _init();
    
    //  open("lissajous0r");
    //  open("ising0r");
    
    return(true);
}

