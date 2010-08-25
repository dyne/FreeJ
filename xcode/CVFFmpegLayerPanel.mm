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

#import <CVFFmpegLayerPanel.h>
#import <CVScreenView.h>

@implementation CVFFmpegLayerPanel


- (void)awakeFromNib
{
    [self init];
}


- (id)init
{
    myHTTPResponse = [[NSMutableData data] retain];
    URL=[[NSMutableString stringWithCapacity:0] retain];
    streamUrls = [[NSMutableArray arrayWithCapacity:0] retain]; 
    [streamList setAction:@selector(myDoubleClickAction:)]; 
    [streamList setDataSource:(id)self];
    [streamList reloadData];
    return self;
}

- (void)dealloc
{
    [URL release];
    [myHTTPResponse release];
    [streamUrls release];
    [super dealloc];
}

- (void)reset
{
    [self makeFirstResponder:streamURL];
    [streamURL setStringValue:@""];
    [streamUrls removeAllObjects];
    [streamList reloadData];
}

- (IBAction)btnLoad:(id)sender
{
	if (streamURL && [[streamURL stringValue] length]) {
		[URL setString:[streamURL stringValue]];
		[[NSApplication sharedApplication] endSheet:self returnCode:NSOKButton];
	}
}

- (IBAction)btnCancel:(id)sender
{
    [[NSApplication sharedApplication] endSheet:self returnCode:NSCancelButton];
}

- (NSString*)getURL
{
    return URL;
}


- (void)openFilePanelDidEnd:(NSOpenPanel *)panel returnCode:(int)returnCode  contextInfo:(void  *)contextInfo
{
    if(returnCode == NSOKButton){
        func("openFilePanel: OK");    
    } else if(returnCode == NSCancelButton) {
        func("openFilePanel: Cancel");
        return;
    } else {
        error("openFilePanel: Error %3d",returnCode);
        return;
    } // end if     

    NSString *tvarFilename = [panel filename];    
    if (tvarFilename && [tvarFilename length]) {
		func("openFile filename = %@",tvarFilename);
	    [streamURL setStringValue:[NSString stringWithString:tvarFilename]];
    }
}


- (IBAction)btnFile:(id)sender
{
    NSOpenPanel *fileSelectionPanel    = [NSOpenPanel openPanel];
    NSArray *types = [NSArray arrayWithObjects:
                      @"avi", @"mov", @"mpg", @"asf", @"mp4", 
                      @"ogg", @"ogv", @"flv", @"wmv", @"mkv", 
                      @"vob", @"mpeg", @"dv", @"mxf",
                      @"jpg", @"png", @"tif", @"bmp", @"gif", @"pdf", nil];
    
    [fileSelectionPanel 
     beginSheetForDirectory:nil 
     file:nil
     types:types 
     modalForWindow:[sender window]
     modalDelegate:self 
     didEndSelector:@selector(openFilePanelDidEnd: returnCode: contextInfo:) 
     contextInfo:nil];    
    [fileSelectionPanel setCanChooseFiles:YES];
}

/* Table interface */
- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
    if (aTableView == streamList) {
      return [streamUrls count];
    }
    return 0;
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
    if (aTableView == streamList) {
	return [NSString stringWithString:[streamUrls objectAtIndex:rowIndex]];
    }
    return nil;
}

- (IBAction) myDoubleClickAction:(id)sender { 
    NSInteger rowIndex = [sender selectedRow];
    if(rowIndex>=0 && rowIndex < [streamUrls count])
	[streamURL setStringValue:[NSString stringWithString:[streamUrls objectAtIndex:rowIndex]]];
}

/* HTTP interface */

- (void)connection:(NSURLConnection *)connection didReceiveResponse:(NSURLResponse *)response
{
    [myHTTPResponse setLength:0];
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data
{
    [myHTTPResponse appendData:data];
}

- (void)connection:(NSURLConnection *)connection didFailWithError:(NSError *)error
{
    //NSLog(@"HTTP: error.");
    [[NSAlert alertWithError:error] runModal]; // XXX
}

- (NSString *)contentString
{
	return [[[NSString alloc] initWithData:myHTTPResponse encoding:NSUTF8StringEncoding] autorelease];
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection
{
    // Once this method is invoked, "myHTTPResponse" contains the complete result

    NSString* str = [[NSString alloc] initWithData:myHTTPResponse encoding:NSUTF8StringEncoding];
    //NSLog(@"HTTP: %@", str);

    NSArray * myUrls = [str componentsSeparatedByString:@"\n"];
    NSUInteger i;
    for (i=0; i <  [myUrls count]; i++) {
        //NSLog(@"HTTP: %@", [myUrls objectAtIndex:i]);
        if ([[myUrls objectAtIndex:i] hasPrefix:@"http://"])
            [streamUrls addObject:[myUrls objectAtIndex:i]];
    }
    [streamList reloadData];
    [str release];
}

@end
