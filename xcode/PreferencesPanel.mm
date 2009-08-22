//
//  PreferencesPanel.m
//  freej
//
//  Created by xant on 8/20/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "PreferencesPanel.h"

static struct {
    int w;
    int h;
} allowedScreenSizes[4] = 
{
    { 320, 240 },
    { 352, 288 },
    { 512, 384 },
    { 640, 480 }
};

@implementation PreferencesPanel

- (IBAction)setScreenSize:(id)sender
{
    int idx = [sender integerValue]-1;
    
    [mainScreen setSizeWidth:allowedScreenSizes[idx].w Height:allowedScreenSizes[idx].h];
}

@end
