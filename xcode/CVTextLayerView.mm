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

- (IBAction)startText:(id)sender
{
    NSString *text = [textView string];
    [(CVTextLayerController *)layerController setText:text];
}

- (NSFont *)font
{
    return [textView font];
}

- (NSColor *)textColor
{
    return [textView textColor];
}

- (NSColor *)backgroundColor
{
    return [textView backgroundColor];
}

@end
