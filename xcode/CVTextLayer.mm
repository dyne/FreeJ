//
//  CVTextLayer.mm
//  freej
//
//  Created by xant on 6/11/09.
//
//  Copyright 2009 dyne.org.
//

#import "CVTextLayer.h"

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

@implementation CVTextLayerController

- (id)init
{
    theString = [GLString alloc];
    // init fonts for use with strings
    return [super init];
}

- (CVReturn)renderFrame
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Context *ctx = [freej getContext];
    [lock lock];

    if (needsNewFrame) {
        if (currentFrame)
            CVPixelBufferRelease(currentFrame);
        NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:
                           [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, 
                           [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, 
                           [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey,
                           nil];
        
        // create pixel buffer
        CVPixelBufferCreate(kCFAllocatorDefault, ctx->screen->geo.w, ctx->screen->geo.h, k32ARGBPixelFormat, (CFDictionaryRef)d, &currentFrame);

        NSMutableDictionary *stanStringAttrib = [[NSMutableDictionary dictionary] retain];
        if (layerView) {
            [stanStringAttrib setObject:[(CVTextLayerView *)layerView font] forKey:NSFontAttributeName];
            [stanStringAttrib setObject:[(CVTextLayerView *)layerView textColor] forKey:NSForegroundColorAttributeName];
            [stanStringAttrib setObject:[(CVTextLayerView *)layerView backgroundColor] forKey:NSBackgroundColorAttributeName];
            [theString initWithString:text withAttributes:stanStringAttrib];

        } else {
            // TODO - Implement properly
            //NSFont * font =[NSFont fontWithName:@"Helvetica" size:32.0];
            [theString initWithString:text withAttributes:stanStringAttrib 
                withTextColor:[NSColor colorWithDeviceRed:1.0f green:1.0f blue:1.0f alpha:1.0f] 
                withBoxColor:[NSColor colorWithDeviceRed:0.5f green:0.5f blue:0.0f alpha:0.5f] 
                withBorderColor:[NSColor colorWithDeviceRed:0.3f green:0.8f blue:0.3f alpha:0.8f]];
            //[font release];
        }
        CGLLockContext(glContext);
        CGLSetCurrentContext(glContext);
        [theString drawOnBuffer:currentFrame];
        CGLUnlockContext(glContext);
        layer->buffer = currentFrame;
        needsNewFrame = NO;
    }
    newFrame = YES;
    [lock unlock];
    [self renderPreview];
    [pool release];
    return 0;
}

- (IBAction)setText:(NSString *)theText
{
    if (!layer)
        [self start];
    text = theText;
    //NSLog(@"%@\n",theText);
    needsNewFrame = YES;
}

@end
