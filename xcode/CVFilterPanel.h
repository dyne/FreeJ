//
//  CVFilterPanel.h
//  freej
//
//  Created by xant on 2/26/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#ifndef __CV_FILTERPANEL_H__
#define __CV_FILTERPANEL_H__

#import <Cocoa/Cocoa.h>
#import <CVLayer.h>

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
    IBOutlet CVFilterBox    *mainView;
    IBOutlet NSPopUpButton  *filterButton;
    IBOutlet CVPreview      *previewBox;
    IBOutlet NSButton       *showButton;
    IBOutlet NSButton       *previewButton;

}
- (void)show;
- (id)init;
- (void)setLayer:(NSView *)lay;
- (FilterParams *)getFilterParamsDescriptorAtIndex:(int)index;
- (IBAction)setFilterParameter:(id)sender;
- (IBAction)togglePreview:(id)sender;
- (IBAction)toggleVisibility:(id)sender;
- (IBAction)setBlendMode:(id)sender;

@end

#endif
