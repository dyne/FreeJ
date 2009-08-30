//
//  CVF0rLayer.h
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CVF0RLAYER_H__
#define __CVF0RLAYER_H__

#include <gen_f0r_layer.h>
#include <CVLayer.h>

class CVF0rLayer : public CVLayer
{
	public:
		CVF0rLayer(CVLayerView *view, char *generatorName, Context *freej);
        ~CVF0rLayer();
        bool init(Context *freej);
        bool init(Context *freej, int w, int h) { return init(freej); };
        
        bool open(const char *file);
        void *feed();
        void close();
        FilterInstance *generator;
};

#endif
