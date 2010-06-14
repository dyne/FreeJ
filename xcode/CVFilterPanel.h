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

#ifndef __CV_FILTERPANEL_H__
#define __CV_FILTERPANEL_H__

#import <CVLayer.h>
#import <Cocoa/Cocoa.h>
#include <linklist.h>

@class CVFilterPanel;

@interface CVFilterBox : NSView
{
    NSTrackingArea *trackingArea;
    IBOutlet CVFilterPanel *filterPanel;
}
- (void)mouseExited:(NSEvent *)theEvent;
/*
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)theEvent;
*/
@end

@interface CVFilterPanel : NSWindowController {
    CVLayerController       *layer;
    NSRecursiveLock         *lock;
    NSRect initialWindowFrame;
    NSRect initialFrame;
    NSRect initialBounds;
    IBOutlet CVFilterBox    *mainView; /// our container
    IBOutlet NSPopUpButton  *blendModeButton;
    IBOutlet NSPopUpButton  *filterButton; /// filters-selection button
    IBOutlet CVPreview      *previewBox; /// the preview box
    IBOutlet NSButton       *showButton; /// toggle visibility button
    IBOutlet NSButton       *previewButton; /// toggle preview button
    /* TODO - use mainView to access each slider ...  this outlets should be removed asap */
    IBOutlet NSSlider       *firstImageParam; /// first "image-parameters' slider
    IBOutlet NSSlider       *lastImageParam;  /// last "image-parameters' slider
    IBOutlet NSTabView      *activeFilters;
}
- (void)show;
- (id)init;
- (void)setLayer:(CVLayerController *)lay;
- (void)updateActiveFilters;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)addFilter:(id)sender;
- (IBAction)togglePreview:(id)sender;
- (IBAction)toggleVisibility:(id)sender;
- (IBAction)setBlendMode:(id)sender;

@end

#endif
