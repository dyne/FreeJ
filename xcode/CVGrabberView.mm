//
//  CVGrabberView.m
//  freej
//
//  Created by xant on 9/2/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVGrabberView.h"


@implementation CVGrabberView

- (id)init
{
    static char *suffix = "/Contents/Resources/webcam.png";
    char iconFile[1024];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(suffix)-1);
    strcat(iconFile, suffix);
    icon = [[NSImage alloc] initWithContentsOfURL:
            [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]];
    //posterImage = [[CIImage imageWithContentsOfURL:
    //                [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]] retain];    
    return [super init];
}


- (bool)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)theRect
{
    if (!posterImage)
        [self setPosterImage:icon];
    [super drawRect:theRect];
}

@end