//
//  CVInternalLayer.h
//  freej
//
//  Created by xant on 6/14/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <CVLayer.h>

@interface CVInternalLayerView : CVLayerController {
}
- (void)feedFrame:(void *)frame;
@end

class CVInternalLayer: public CVLayer
{
    public:
        Layer *layer;
        CVInternalLayer(CVInternalLayerView *view, Layer *lay);
        ~CVInternalLayer();
        bool init(Context *ctx);
        bool init(Context *ctx, int w, int h);
        void attach(Layer *lay);
        void *feed();
};
