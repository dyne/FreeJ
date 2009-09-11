//
//  CVLayerController.h
//  freej
//
//  Created by xant on 9/1/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifndef __CVLAYERCONTROLLER_H__
#define __CVLAYERCONTROLLER_H__

#import <Cocoa/Cocoa.h>
#import <CVLayer.h>
#import <CVLayerView.h>

@class CVLayerView;
class CVLayer;

@interface CVLayerController : NSObject {
        CFreej                *freej;
        NSRecursiveLock       *lock;
        
        bool                   newFrame;
        CVPixelBufferRef       currentFrame;    // the current frame from the movie
        CVTexture              *lastFrame;
        CIImage                *posterImage;
        bool                   doPreview;
        CVTexture              *currentPreviewTexture;
        CGLContextObj         glContext;

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
        
        uint64_t               lastRenderedTime;
        CVLayer                *layer;
        IBOutlet CVLayerView   *layerView;
        bool                   doFilters;
    }
    
    @property (readwrite) CVLayer *layer;
    @property (readonly) CVPixelBufferRef currentFrame;
    // subclass implementation should override this method and update 
    // the currentFrame pointer only within the renderFrame implementation
    - (char *)name;
    - (CVReturn)renderFrame;
    - (id)initWithOpenGLContext:(CGLContextObj)context pixelFormat:(CGLPixelFormatObj)pixelFormat Context:(CFreej *)ctx;
    - (id)initWithContext:(CFreej *)context;
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
    - (void)setLayer:(CVLayer *)lay;
    - (void)toggleFilters;
    - (void)toggleVisibility;
    - (void)togglePreview;
    - (bool)doPreview;
    - (IBAction)setFilterParameter:(id)sender; /// tags from 0 to 10
    - (IBAction)setBlendMode:(id)sender;
@end

#endif