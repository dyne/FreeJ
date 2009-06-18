//
//  CVTextLayer.mm
//  freej
//
//  Created by xant on 6/11/09.
//
//  Copyright 2009 dyne.org.
//

#import "CVTextLayer.h"


@implementation CVTextLayer

- (id)init
{
    theString = [GLString alloc];
    // init fonts for use with strings
    return [super init];
}

- (CVReturn)renderFrame
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [lock lock];

    if (needsNewFrame) {
        if (currentFrame)
            CVPixelBufferRelease(currentFrame);
        NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, 
                           [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, 
                           nil];
        
        // create pixel buffer
        CVPixelBufferCreate(kCFAllocatorDefault, layer->geo.w, layer->geo.h, k32ARGBPixelFormat, (CFDictionaryRef)d, &currentFrame);

        //NSFont * font =[NSFont fontWithName:@"Helvetica" size:32.0];
        NSMutableDictionary *stanStringAttrib = [[NSMutableDictionary dictionary] retain];
        [stanStringAttrib setObject:[textView font] forKey:NSFontAttributeName];
        [stanStringAttrib setObject:[textView textColor] forKey:NSForegroundColorAttributeName];
        [stanStringAttrib setObject:[textView backgroundColor] forKey:NSBackgroundColorAttributeName];

        //[font release];
        //[theString initWithString:string withAttributes:stanStringAttrib withTextColor:[NSColor colorWithDeviceRed:1.0f green:1.0f blue:1.0f alpha:1.0f] withBoxColor:[NSColor colorWithDeviceRed:0.5f green:0.5f blue:0.0f alpha:0.5f] withBorderColor:[NSColor colorWithDeviceRed:0.3f green:0.8f blue:0.3f alpha:0.8f]];
        [theString initWithString:text withAttributes:stanStringAttrib];
        [[self openGLContext] makeCurrentContext];
        [theString drawOnBuffer:currentFrame];
        layer->buffer = currentFrame;
        needsNewFrame = NO;
    }
    newFrame = YES;
    [lock unlock];
    [self renderPreview];
    [pool release];
    return 0;
}

- (IBAction)startText:(id)sender
{
    if (!layer)
        [self start];
    text = [textView string];
    needsNewFrame = YES;
}

@end
