//
//  CVF0rLayer.h
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CVF0RLAYER_H__
#define __CVF0RLAYER_H__

#include <CVLayer.h>

class CVF0rLayer : public CVLayer
{
    protected:
        bool _init();

	public:
		CVF0rLayer(CVLayerController *controller, Context *freej);
        ~CVF0rLayer();
        void register_generators(Linklist<Filter> *gens);
        void *lastFrame;
        bool open(const char *file);
        void *feed();
        void close();
        Linklist<Filter> *generators; ///< linked list of registered generators
        FilterInstance *generator;
};

#endif
