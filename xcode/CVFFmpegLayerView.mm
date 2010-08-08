/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
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

#import <CVLayerController.h>
#import <CVFFmpegLayerView.h>
#import <CVFFmpegLayerPanel.h>

@implementation CVFFmpegLayerView

- (void)openStreamPanelDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSOKButton) {
        //NSLog(@"didEnd: %@",[((FFInputPanel*)ffInputPanel) getURL]);
	[(CVFFmpegLayerController *)layerController setStream:[ffInputPanel getURL]];
    }
    [sheet orderOut:self];
    [[NSApplication sharedApplication] endSheet:ffInputPanel];
}

- (IBAction)open:(id)sender 
{
    //NSLog(@"open Stream select window..");
   
    [ffInputPanel reset]; 
    [[NSApplication sharedApplication] beginSheet:ffInputPanel 
	modalForWindow:[sender window]
	modalDelegate:self 
	didEndSelector:@selector(openStreamPanelDidEnd:returnCode:contextInfo:) 
	contextInfo:self];    
}

- (IBAction)close:(id)sender
{
    [(CVFFmpegLayerController *)layerController setStream:nil];
}

- (void) drawRect:(NSRect)theRect
{
    [super drawRect:theRect];
}

- (IBAction)toggleRepeat:(id)sender
{
	if ([sender state] == NSOnState)
		[(CVFFmpegLayerController *)layerController setRepeat:YES];
	else
		[(CVFFmpegLayerController *)layerController setRepeat:NO];
}

@end
