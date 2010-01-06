//
//  CVTextLayerView.mm
//  freej
//
//  Created by xant on 12/30/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import <CVTextLayerController.h>
#import "CVTextLayerView.h"

@implementation CVTextLayerView

- (id)init {
    attributes = [[NSMutableDictionary dictionary] retain];
    [attributes
     setObject:[textView font]
     forKey:NSFontAttributeName
    ];
    [attributes
     setObject:[textView textColor]
     forKey:NSForegroundColorAttributeName
    ];
    [attributes
     setObject:[[textView backgroundColor] colorWithAlphaComponent:0.0]
     forKey:NSBackgroundColorAttributeName
    ];
    return [super init];
}

- (void)changeDocumentBackgroundColor:(id)sender
{
    [attributes setObject:[sender color] forKey:NSBackgroundColorAttributeName];
}

- (void)changeAttributes:(id)sender
{
    NSDictionary *oldAttributes = attributes;
    attributes = (NSMutableDictionary *)[sender convertAttributes: oldAttributes];
}

- (void)changeFont:(id)sender {
    NSFont *oldFont = [attributes objectForKey:NSFontAttributeName];
    NSFont *newFont = [sender convertFont:oldFont];
    [attributes setObject:newFont forKey:NSFontAttributeName];

}

- (IBAction)showFontMenu:(id)sender {
    NSFontManager *fontManager = [NSFontManager sharedFontManager];
    [fontManager setDelegate:self];
    [fontManager setTarget:self];
    NSFontPanel *fontPanel = [fontManager fontPanel:YES];
    [fontPanel setNextResponder:self];
    [fontPanel orderFront:self];
}

- (IBAction)startText:(id)sender
{
    NSString *text = [textView string];
    [(CVTextLayerController *)layerController setText:text withAttributes:attributes];
}

// show the text while typing
- (IBAction)doLive:(id)sender
{
    bool newValue = [sender intValue];
    if (live != newValue) {
        if (!live)
            [self startText:self];
        live = [sender intValue];
    }
}

// if we are running in live mode... let's update the text when it changes
- (void)textDidChange:(NSNotification *)aNotification
{
    if (live)
        [self startText:self];
        
}

@end
