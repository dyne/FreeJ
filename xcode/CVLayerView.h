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

#ifndef __CVLAYERVIEW_H__
#define __CVLAYERVIEW_H__

#import  <CVLayerController.h>
#import  <CVtexture.h>
#import  <CFreeJ.h>
#include <QuartzCore/QuartzCore.h>


@class CVFilterPanel;
@class CVLayerController;
@class CVPreview;
class CVLayer;

@interface CVLayerView : NSOpenGLView {
@protected
    NSRecursiveLock       *lock;
    bool                   needsReshape;

    CIFilter               *scaleFilter;

    CIContext              *ciContext;
    CIImage                *posterImage;
    IBOutlet CFreej        *freej;
    IBOutlet CVFilterPanel *filterPanel;
    IBOutlet NSSlider      *alphaBar;
    IBOutlet NSButton      *showButton;
    IBOutlet NSButton      *previewButton;
    IBOutlet CVLayerController *layerController;
}

- (void)clear;
- (CVFilterPanel *)filterPanel;
- (void)setPosterImage:(NSImage *)image;
//- (void)setPreviewTarget:(CVPreview *)targetView;
- (bool)needPreview; // true if we need to provide a preview, else otherwise
- (void)startPreview; // enable preview rendering
- (void)stopPreview; // disable preview rendering
- (bool)isVisible; // query the layer to check if it's being sent to the Screen or not
- (void)activate; /// activate the underlying CVLayer
- (void)deactivate; /// deactivate the underlying CVLayer
- (NSString *)blendMode;
// Interface Builder API 
- (IBAction)setBlendMode:(id)sender; /// tag -1
- (IBAction)toggleFilters:(id)sender; /// toggle CIImage filters
- (IBAction)togglePreview:(id)sender; /// toggle preview rendering
// toggle layer registration on the underlying context
// (so to control wether the layer has to be sent to the Screen or not)
- (IBAction)toggleVisibility:(id)sender;

@end

#endif
