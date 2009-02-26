//
//  CVFilterPanel.m
//  freej
//
//  Created by xant on 2/26/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CVFilterPanel.h"


@implementation CVFilterPanel
- (id) init
{
	if (![super initWithWindowNibName:@"EffectsPanel"])
		return nil;
	layer = nil;
	return self;
}

- (void)windowDidLoad
{
	//NSLog(@"Nib file loaded");
}

- (void)show
{
	// force opening the EffectsPanel under the mouse pointer
	NSPoint origin = [NSEvent mouseLocation];
	NSRect frame = [[self window] frame];
	origin.x -= frame.size.width/2;
	origin.y -= frame.size.height/2;
	[[self window] setFrameOrigin:origin];
	[[self window] makeKeyAndOrderFront:self];
}

- (void)setLayer:(NSView *)lay
{
	layer = lay;
	[[self window] setTitle:[layer toolTip]];
}

- (IBAction)setFilterParameter:(id)sender
{
	if(layer)
		[layer setFilterParameter:sender];
}

@end

@implementation CVFilterBox

- (void)awakeFromNib
{
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
				options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow)
				owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];

}

- (void)mouseEntered:(NSEvent *)theEvent {
}
 
- (void)mouseExited:(NSEvent *)theEvent {
	[[self window] orderOut:self];
}

@end