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
    text = nil;
    // init fonts for use with strings
    return [super init];
}

- (CVReturn)renderFrame
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    Context *ctx = [freej getContext];
    [lock lock];
    if (needsNewFrame) {
        CVPixelBufferRef textFrame;
        NSDictionary *d = [NSDictionary dictionaryWithObjectsAndKeys:
                       [NSNumber numberWithInt:ctx->screen->geo.w], kCVPixelBufferWidthKey,
                       [NSNumber numberWithInt:ctx->screen->geo.h], kCVPixelBufferHeightKey,
                       [NSNumber numberWithInt:ctx->screen->geo.w*4],kCVPixelBufferBytesPerRowAlignmentKey,
                       [NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, 
                       [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, 
                       [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey,
                       nil];
        
        // create pixel buffer
        CVReturn ret = CVPixelBufferCreate(kCFAllocatorDefault,
                                            ctx->screen->geo.w,
                                            ctx->screen->geo.h,
                                            k32ARGBPixelFormat,
                                            (CFDictionaryRef)d,
                                            &textFrame);

        // TODO - Implement properly
        //NSFont * font =[NSFont fontWithName:@"Helvetica" size:32.0];
        [theString initWithString:text withAttributes:stanStringAttrib];
        [theString drawOnBuffer:textFrame];
        [self feedFrame:textFrame];
        CVPixelBufferRelease(textFrame);
        needsNewFrame = NO;
    }
    [lock unlock];
    //[self renderPreview];
    [pool release];
    return [super renderFrame];
}

- (IBAction)setText:(NSString *)theText withAttributes:(NSDictionary *)attributes
{
    if (!layer)
        [self start];
    if (text)
        [text release];
    text = [theText retain];
    stanStringAttrib = attributes;
    needsNewFrame = YES;
}

- (IBAction)setImageParameter:(id)sender
{
    [super setImageParameter:sender];
    needsNewFrame = YES;
}

@end
