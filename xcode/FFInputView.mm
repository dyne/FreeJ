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
#import <FFInputView.h>
#import <FFInputPanel.h>

@implementation FFInputView

- (void)openStreamPanelDidEnd:(NSWindow *)sheet returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    if (returnCode == NSOKButton) {
        //NSLog(@"didEnd: %@",[((FFInputPanel*)ffInputPanel) getURL]);
	[(FFInputController *)layerController setStream:[((FFInputPanel*)ffInputPanel) getURL]];
    }
    [sheet orderOut:self];
    [[NSApplication sharedApplication] endSheet:ffInputPanel];
}

- (IBAction)openStream:(id)sender 
{
    //NSLog(@"open Stream select window..");
   
    [ffInputPanel reset]; 
    [[NSApplication sharedApplication] beginSheet:ffInputPanel 
	modalForWindow:[sender window]
	modalDelegate:self 
	didEndSelector:@selector(openStreamPanelDidEnd:returnCode:contextInfo:) 
	contextInfo:self];    
}

- (void) drawRect:(NSRect)theRect
{
    [super drawRect:theRect];
}

@end
