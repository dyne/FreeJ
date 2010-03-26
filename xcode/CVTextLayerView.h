//
//  CVTextLayerView.h
//  freej
//
//  Created by xant on 12/30/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CVLayerView.h>



@interface CVTextLayerView : CVLayerView {
    NSImage *icon;
    IBOutlet NSTextView *textView;
    NSMutableDictionary *attributes;
    bool live;
}

- (IBAction)startText:(id)sender;
- (IBAction)showFontMenu:(id)sender;
- (IBAction)doLive:(id)sender;
@end