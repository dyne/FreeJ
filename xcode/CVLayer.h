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
    // this is a "fake" buffer exposed to freej core
    // XXX - freej actaully needs a buffer pointer but it's not going to use it.
    // The blitter does...but on OSX the blitter is implemented in the CVScreen class.
    // This means we could expose any address to freej and that would work anyway.
    CVPixelBufferRef       pixelBuffer;

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
    
    CIContext              *ciContext;
    uint64_t               lastRenderedTime;
    bool                   needsReshape;
    //CVImageBufferRef     freejFrame;
    CVLayer                *layer;

    bool                   doFilters;
    IBOutlet CFreej        *freej;
    IBOutlet CVFilterPanel *filterPanel;
    IBOutlet NSSlider      *alphaBar;
    IBOutlet NSButton      *showButton;
    IBOutlet NSButton      *previewButton;
}

@property (readwrite) CVLayer *layer;
- (CVReturn)_renderTime:(const CVTimeStamp *)time;
- (void)awakeFromNib;
- (id)init;
- (void)startPreview;
- (void)stopPreview;
- (void)renderPreview;
- (bool)isVisible;
- (void)activate;
- (void)deactivate;
- (CVTexture *)getTexture;
- (void)mouseDown:(NSEvent *)theEvent;
- (bool)isOpaque;
- (bool)needPreview;
- (void)setPreviewTarget:(CVPreview *)targetView;
- (void)lock;
- (void)unlock;
- (IBAction)togglePlay:(id)sender;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)setAlpha:(id)sender;
- (IBAction)setBlendMode:(id)sender;
- (IBAction)toggleFilters:(id)sender;
- (IBAction)togglePreview:(id)sender;
- (IBAction)toggleVisibility:(id)sender;
@end

class CVLayer: public Layer {
    protected:
        int height, width;
        Context *freej;
        void *vbuffer;
        int bufsize;
    public:
        CVLayerView *input;
        NSString *blendMode;
        
        CVLayer();
        CVLayer(NSObject *vin);

        ~CVLayer();
        void activate();
        void deactivate();
        bool open(const char *path);
        bool init(Context *freej);
        bool init(Context *ctx, int w, int h);
        void *feed();
        void close();
        //void run();
        bool forward();
        bool backward();
        bool backward_one_keyframe();
        
        bool relative_seek(double increment);
        
        bool set_mark_in();
        bool set_mark_out();
        
        void pause();
        
        CVTexture *gl_texture();
        
};

#endif
