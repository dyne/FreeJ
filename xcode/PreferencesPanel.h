//
//  PreferencesPanel.h
//  freej
//
//  Created by xant on 8/20/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <CVLayer.h>
#import <Cocoa/Cocoa.h>

@class CVScreenView;
@class CFreej;

@interface PreferencesPanel : NSWindow {
    IBOutlet CFreej *freej;
    IBOutlet CVScreenView *mainScreen;
}

- (IBAction)setScreenSize:(id)sender;
- (IBAction)setExportQuality:(id)sender;
- (IBAction)setPluginsPath:(id)sender;
- (IBAction)setJScriptsPath:(id)sender;

@end
