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

#ifndef __CVLAYERCONTROLLER_H__
#define __CVLAYERCONTROLLER_H__

#include <CVCocoaLayer.h>

#import <CFreej.h>
#import <CVPreview.h>
#import <Cocoa/Cocoa.h>
#import <CVLayerView.h>

@class CVLayerView;
class CVCocoaLayer;

@interface CVLayerController : NSObject {
        CFreej                *freej;
        NSRecursiveLock       *lock;
        
        bool                   newFrame;
        CVPixelBufferRef       currentFrame;    // the current frame from the movie
        CVTexture              *lastFrame;
        CIImage                *posterImage;
        bool                   doPreview;
        CVTexture              *currentPreviewTexture;
        CGLContextObj          glContext;

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
        CVCocoaLayer           *layer;
        IBOutlet CVLayerView   *layerView;
        bool                   doFilters;
        bool                   filtersInitialized;
    }
    
    @property (readwrite) CVCocoaLayer *layer;
    @property (readonly) CVPixelBufferRef currentFrame;
    // subclass implementation should override this method and update 
    // the currentFrame pointer only within the renderFrame implementation
    - (char *)name;
    - (void)feedFrame:(CVPixelBufferRef)frame;
    - (CVReturn)renderFrame;
    - (id)initWithContext:(CFreej *)context;
    - (void)startPreview; // enable preview rendering
    - (void)stopPreview; // disable preview rendering
    - (void)renderPreview; // render the preview frame
    - (bool)isVisible; // query the layer to check if it's being sent to the Screen or not
    - (NSString *)blendMode;
    - (void)activate; /// activate the underlying CVLayer
    - (void)deactivate; /// deactivate the underlying CVLayer
    // Retain currentFrame, apply filters and return a CVTexture
    - (CVTexture *)getTexture;
    // query the layer to check if it needs to display a preview or not (used by CVPreview)
    - (bool)needPreview;
    - (NSString *)filterName;
    - (NSDictionary *)filterParams;
    - (void)setContext:(CFreej *)ctx;
    - (void)setPreviewTarget:(CVPreview *)targetView;
    - (void)lock; /// accessor to the internal mutex
    - (void)unlock; /// accessor to the internal mutex
    - (void)translateXby:(float)x Yby:(float)y;
    - (void)rotateBy:(float)deg;
    - (void)start;
    - (void)stop;
    - (void)setLayer:(CVCocoaLayer *)lay;
    - (void)toggleFilters;
    - (void)toggleVisibility;
    - (void)togglePreview;
    - (bool)doPreview;
    - (IBAction)setFilterParameter:(id)sender; /// tags from 0 to 10
    - (void)setBlendMode:(NSString *)mode;
    - (void)initFilters;
@end

#endif