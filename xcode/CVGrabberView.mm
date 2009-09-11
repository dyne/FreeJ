/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
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