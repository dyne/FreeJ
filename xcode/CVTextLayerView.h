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
    IBOutlet NSTextView *textView;
    NSMutableDictionary *attributes;
}

- (IBAction)startText:(id)sender;
- (IBAction)showFontMenu:(id)sender;

@end