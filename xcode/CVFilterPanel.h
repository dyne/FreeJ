//
//  CVFilterPanel.h
//  freej
//
//  Created by xant on 2/26/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#ifndef __CV_FILTERPANEL_H__
#define __CV_FILTERPANEL_H__

#import <CVLayer.h>
#import <Cocoa/Cocoa.h>

@class CVFilterPanel;

typedef struct __FilterParams {
    int nParams;
    struct __ParamDescr {
        char *label;
        double min;
        double max;
    } params[4];
} FilterParams;

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
    CVLayerView *layer;
    NSRecursiveLock         *lock;
    NSRect initialWindowFrame;
    NSRect initialFrame;
    NSRect initialBounds;
    IBOutlet CVFilterBox    *mainView; /// our container
    IBOutlet NSPopUpButton  *filterButton; /// filters-selection button
    IBOutlet CVPreview      *previewBox; /// the preview box
    IBOutlet NSButton       *showButton; /// toggle visibility button
    IBOutlet NSButton       *previewButton; /// toggle preview button
    /* TODO - use mainView to access each slider ...  this outlets should be removed asap */
    IBOutlet NSSlider       *firstImageParam; /// first "image-parameters' slider
    IBOutlet NSSlider       *lastImageParam;  /// last "image-parameters' slider
}
- (void)show;
- (id)init;
- (void)setLayer:(CVLayerView *)lay;
- (FilterParams *)getFilterParamsDescriptorAtIndex:(int)index;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)togglePreview:(id)sender;
- (IBAction)toggleVisibility:(id)sender;
- (IBAction)setBlendMode:(id)sender;

@end

#endif
