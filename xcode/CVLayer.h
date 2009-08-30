/*
 *  CVLayer.h
 *  freej
 *
 *  Created by xant on 2/23/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */
#ifndef __CVLAYER_H__
#define __CVLAYER_H__


#include <layer.h>
#include <context.h>
#import <CVLayerView.h>

@class CVLayerView;

class CVLayer: public Layer 
{
    protected:
        int height, width;
        Context *freej;
        int bufsize;    
        virtual bool _init();
    public:
        CVLayerView *input;
        NSString *blendMode;
        void *vbuffer;

        CVLayer();
        CVLayer(CVLayerView *vin);

        ~CVLayer();
        void activate();
        void deactivate();
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
        
        virtual CVTexture *gl_texture();
    
    private:
        virtual void *feed();
};

#endif
