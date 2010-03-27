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

#import <CVLayer.h>
#import <Cocoa/Cocoa.h>

@class CFreej;

@interface CVFFmpegLayerPanel : NSWindow {
    IBOutlet CFreej *freej;
    IBOutlet NSTableView *streamList;
    IBOutlet NSTextField *streamURL;
    NSMutableString *URL;
    NSMutableArray *streamUrls;
    NSMutableData *myHTTPResponse;
}

- (IBAction)btnLoad:(id)sender;
- (IBAction)btnFile:(id)sender;
- (IBAction)btnCancel:(id)sender;
- (IBAction) myDoubleClickAction:(id)sender;
- (NSString*)getURL;
- (void)reset;
- (NSString *)contentString;
- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex;
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView;

@end
