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

#import <CVLayerController.h>
#import <CVQtLayerView.h>


@implementation CVQtLayerView

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
    NSString * tvarDirectory = [panel directory];
    func("openFile directory = %@",tvarDirectory);
    
    NSString * tvarFilename = [panel filename];
    func("openFile filename = %@",tvarFilename);
    
    if (tvarFilename) {
        NSDictionary *movieAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                         [NSNumber numberWithBool:YES], QTMovieOpenAsyncOKAttribute,
                                         tvarFilename, QTMovieFileNameAttribute,
                                         [NSNumber numberWithBool:NO] , QTMovieHasAudioAttribute,
                                         nil];
        QTMovie *movie = [[QTMovie alloc] initWithAttributes:movieAttributes error:nil];
        [movie setIdling:NO];
        if (![(CVQtLayerController *)layerController setQTMovie:movie])
            warning("Can't open file: %s", [tvarFilename UTF8String]);
    }
}

- (IBAction)open:(id)sender 
{    
    NSOpenPanel *fileSelectionPanel    = [NSOpenPanel openPanel];
    NSArray *types = [NSArray arrayWithObjects:
                      @"avi", @"mov", @"mpg", @"asf", @"jpg", 
                      @"png", @"tif", @"bmp", @"gif", @"pdf", nil];
    
    [fileSelectionPanel 
     beginSheetForDirectory:nil 
     file:nil
     types:types 
     modalForWindow:[sender window]
     modalDelegate:self 
     didEndSelector:@selector(openFilePanelDidEnd: returnCode: contextInfo:) 
     contextInfo:nil];    
    [fileSelectionPanel setCanChooseFiles:YES];
} // end openFile

- (IBAction)close:(id)sender
{
    [(CVQtLayerController *)layerController unloadMovie];
    [self setPosterImage:nil];
}

- (void) drawRect:(NSRect)theRect
{
    [super drawRect:theRect];
}

- (IBAction)setMovieTime:(id)sender
{
    // TODO - Implement
    NSLog(@"ImplementMe()");
}

- (IBAction)togglePlay:(id)sender
{
    // TODO - Implement
    NSLog(@"ImplementMe()");
}

- (IBAction)toggleRepeat:(id)sender
{
    [(CVQtLayerController *)layerController setRepeat:
     ([repeatButton state] == NSOnState)?YES:NO];
}

@end
