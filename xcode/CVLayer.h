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
#include <QuartzCore/QuartzCore.h>
#import  <CVtexture.h>
#import  <CVPreview.h>
#import  <CFreeJ.h>

@class CVFilterPanel;
class CVLayer;

@interface CVLayerView : NSOpenGLView {
@protected
    NSRecursiveLock       *lock;
    
    bool                   newFrame;
    CVPixelBufferRef       currentFrame;    // the current frame from the movie
    CVTexture              *lastFrame;
    CIImage                *posterImage;
    bool                   doPreview;
    CVPreview              *previewTarget;
    CVTexture              *currentPreviewTexture;

    // display link
    CVDisplayLinkRef       displayLink;            // the displayLink that runs the show
    CGDirectDisplayID      viewDisplayID;
    // filters for CI rendering
    CIFilter               *colorCorrectionFilter;        // hue saturation brightness control through one CI filter
    CIFilter               *compositeFilter;        // composites the timecode over the video
    CIFilter               *alphaFilter;
    CIFilter               *exposureAdjustFilter;
    CIFilter               *rotateFilter;
    CIFilter               *translateFilter;
    CIFilter               *effectFilter;
    CIFilter               *scaleFilter;
    NSMutableArray         *paramNames;
    NSMutableArray         *paramValues;
    NSMutableDictionary    *filterParams;
    
    CIContext              *ciContext;
    uint64_t               lastRenderedTime;
    bool                   needsReshape;
    CVLayer                *layer;

    bool                   doFilters;
    IBOutlet CFreej        *freej;
    IBOutlet CVFilterPanel *filterPanel;
    IBOutlet NSSlider      *alphaBar;
    IBOutlet NSButton      *showButton;
    IBOutlet NSButton      *previewButton;
}

@property (readwrite) CVLayer *layer;
// subclass implementation should override this method and update 
// the currentFrame pointer only within the renderFrame implementation
- (CVReturn)renderFrame;
- (id)init;
- (void)startPreview; // enable preview rendering
- (void)stopPreview; // disable preview rendering
- (void)renderPreview; // render the preview frame
- (bool)isVisible; // query the layer to check if it's being sent to the Screen or not
- (void)activate; /// activate the underlying CVLayer
- (void)deactivate; /// deactivate the underlying CVLayer
// Retain currentFrame, apply filters and return a CVTexture
- (CVTexture *)getTexture;
// query the layer to check if it needs to display a preview or not (used by CVPreview)
- (bool)needPreview;
- (NSString *)filterName;
- (NSDictionary *)filterParams;
- (void)setPreviewTarget:(CVPreview *)targetView;
- (void)lock; /// accessor to the internal mutex
- (void)unlock; /// accessor to the internal mutex
- (void)translateXby:(float)x Yby:(float)y;
- (void)rotateBy:(float)deg;
- (void)start;
- (void)stop;

// Interface Builder API 
- (IBAction)setFilterParameter:(id)sender; /// tags from 0 to 10
- (IBAction)setBlendMode:(id)sender; /// tag -1
- (IBAction)toggleFilters:(id)sender; /// toggle CIImage filters
- (IBAction)togglePreview:(id)sender; /// toggle preview rendering
// toggle layer registration on the underlying context
// (so to control wether the layer has to be sent to the Screen or not)
- (IBAction)toggleVisibility:(id)sender;

@end

class CVLayer: public Layer 
{
    protected:
        int height, width;
        Context *freej;
        void *vbuffer;
        int bufsize;    
    public:
        CVLayerView *input;
        NSString *blendMode;
        
        CVLayer();
        CVLayer(CVLayerView *vin);

        ~CVLayer();
        void activate();
        void deactivate();
        virtual bool open(const char *path);
        virtual bool init(Context *ctx);
        virtual bool init(Context *ctx, int w, int h);
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
