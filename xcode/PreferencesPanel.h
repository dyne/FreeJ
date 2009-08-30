//
//  PreferencesPanel.h
//  freej
//
//  Created by xant on 8/20/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CVScreen.h>

@interface PreferencesPanel : NSWindow {
    IBOutlet CFreej *freej;
    IBOutlet CVScreenView *mainScreen;
}

- (IBAction)setScreenSize:(id)sender;
- (IBAction)setExportQuality:(id)sender;
- (IBAction)setPluginsPath:(id)sender;
- (IBAction)setJScriptsPath:(id)sender;

@end
