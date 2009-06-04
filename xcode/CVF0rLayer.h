//
//  CVF0rLayer.h
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#include <gen_f0r_layer.h>
#include <CVLayer.h>

@interface CVF0rLayerView : CVLayerView
{
    CIImage *icon;
}
- (void)reset;
- (void)feedFrame:(void *)frame;
@end;

class CVF0rLayer : public GenF0rLayer, public CVLayer
{
	public:
		CVF0rLayer(CVLayerView *view, char *generatorName, Context *freej);
        ~CVF0rLayer();
		void *feed();
        void start();
        void stop();
};