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

@interface CVFilterBox : NSView
{
	NSTrackingArea *trackingArea;
}
- (void)mouseExited:(NSEvent *)theEvent;
/*
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseDown:(NSEvent *)theEvent;
*/
@end

@interface CVFilterPanel : NSWindowController {
	NSView *layer;
	IBOutlet CVFilterBox	*mainView;
}
- (void)show;
- (void)setLayer:(NSView *)lay;
- (IBAction)setFilterParameter:(id)sender;
@end

#endif