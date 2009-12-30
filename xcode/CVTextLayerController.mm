/*  FreeJ
 *  (c) Copyright 2009 Andrea Guzzo <xant@dyne.org>
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

#import <CVTextLayerController.h>

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
