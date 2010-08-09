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
#include <linklist.h>

@class CVLayerView;
class CVCocoaLayer;

@interface CVLayerController : NSObject {
        CFreej                *freej;
        NSRecursiveLock       *lock;
        
        CVPixelBufferRef       currentFrame;    // the current frame from the movie
        CIImage                *posterImage;
        CGLContextObj          glContext;
        // filters for CI rendering
        CIFilter               *colorCorrectionFilter;        // hue saturation brightness control through one CI filter
        CIFilter               *compositeFilter;        // composites the timecode over the video
        CIFilter               *alphaFilter;
        CIFilter               *exposureAdjustFilter;
        CIFilter               *rotateFilter;
        CIFilter               *translateFilter;
        CIFilter               *scaleFilter;
        NSMutableArray         *paramNames;
        NSMutableArray         *paramValues;
        NSMutableDictionary    *imageParams;
        
        uint64_t               lastRenderedTime;
        CVCocoaLayer           *layer;
        CVPreview              *previewTarget;
        IBOutlet CVLayerView   *layerView;
		BOOL                   doPreview;
        BOOL                   doFilters;
		BOOL				   wantsRepeat;
        BOOL                   filtersInitialized;
    }
    
    @property (readwrite, assign) CFreej *freej;
    @property (readwrite) CVCocoaLayer *layer;
    @property (readonly) CVLayerView *layerView;
	@property (readwrite) BOOL doPreview;
	@property (readwrite) BOOL doFilters;
	@property (readwrite) BOOL wantsRepeat;

	// Subclasses can override this method to feed the current frame before returning it
	// To provide the frame [self feedFrame:] must be called.
	// The returned CVPixelBufferRef is retained and MUST be released by the caller.
	// NOTE : usually this is called from the C++ layer implementation , within the feed() method. 
	//        But some Cocoa-layers could lack a specific C++ class since the frame is created 
	//		  directly in the cocoa implementation. 
	//        In such cases (for instance QTLayerController and CVTextLayerController) 'currentFrame' 
	//		  is overridden to provide the new pixel buffer before calling [super currentFrame]
	- (CVPixelBufferRef)currentFrame;
    - (char *)name;
	// applies image parameters (brightness, contrast, exposure, etc) and stores the new frame as 'current' on
	// (to be later returned by [self currentFrame]
    - (void)feedFrame:(CVPixelBufferRef)frame;
	- (void)frameFiltered:(void *)buffer;
    - (id)initWithContext:(CFreej *)context;
    - (void)startPreview; // enable preview rendering
    - (void)stopPreview; // disable preview rendering
	- (void)renderPreview:(CVPixelBufferRef)frame; // render the preview frame
    - (bool)isVisible; // query the layer to check if it's being sent to the Screen or not
    - (NSString *)blendMode;
    - (void)activate; /// activate the underlying CVLayer
    - (void)deactivate; /// deactivate the underlying CVLayer
    // query the layer to check if it needs to display a preview or not (used by CVPreview)
    - (NSDictionary *)imageParams;
    - (void)setPreviewTarget:(CVPreview *)targetView;
    - (CVPreview *)getPreviewTarget;
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
    - (BOOL)doPreview;
	- (BOOL)wantsRepeat;
	- (void)setRepeat:(BOOL)repeat;
    - (void)setBlendMode:(NSString *)mode;
    - (void)initFilters;
    - (int)width;
    - (int)height;
    - (Linklist<FilterInstance> *)activeFilters;
    - (IBAction)setImageParameter:(id)sender; /// tags from 0 to 10
    - (IBAction)setValue:(NSNumber *)value forImageParameter:(NSString *)parameter;
@end

#endif