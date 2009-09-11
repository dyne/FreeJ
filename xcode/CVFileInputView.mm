//
//  CVFileInputView.mm
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CVFileInputView.h"


@implementation CVFileInputView

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
        if (![(CVFileInputController *)layerController setQTMovie:movie])
            warning("Can't open file: %s", [tvarFilename UTF8String]);
    }
}

- (IBAction)openFile:(id)sender 
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

- (void) drawRect:(NSRect)theRect
{
    [super drawRect:theRect];
}

@end
