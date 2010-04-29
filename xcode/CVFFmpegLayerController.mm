/*  FreeJ
 *  (c) Copyright 2010 Robin Gareus <robin@gareus.org>
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

#import <CVScreenView.h>
#import <CVFFmpegLayerView.h>
#import <CFreej.h>
#import <CIAlphaFade.h>

#include <jutils.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <CVFFmpegLayer.h>

@implementation CVFFmpegLayerController : CVLayerController

- (id)init
{
    static char *suffix0 = (char*)"/Contents/Resources/live_w.png";
    static char *suffix1 = (char*)"/Contents/Resources/live_r.png";
    char iconFile[1024];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(suffix0)-1);
    strcat(iconFile, suffix0);
    icon0 = [[[NSImage alloc] initWithContentsOfURL:
            [NSURL fileURLWithPath:[NSString stringWithCString:iconFile encoding:NSASCIIStringEncoding]]] retain];
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(suffix1)-1);
    strcat(iconFile, suffix1);
    icon1 = [[[NSImage alloc] initWithContentsOfURL:
            [NSURL fileURLWithPath:[NSString stringWithCString:iconFile encoding:NSASCIIStringEncoding]]] retain];

    skipCount = 0;
    movie = NULL;
	previewImage = nil;
    timeout=1;
    currentFrame = nil;
	bufDict = [[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey, nil] retain];

    return [super init];
}

- (void)dealloc
{
    if (currentFrame) {
        CVPixelBufferRelease(currentFrame);
    }
    [bufDict release];
    [icon0 release];
    [icon1 release];
	[self clearPreview];
    [super dealloc];
}

- (void)clearPreview
{
	if (previewImage) {
		[previewImage release];
        [layerView setPosterImage:nil];
        previewImage = nil;
	}
}

- (void)deactivate
{
    [self clearPreview];
    [super deactivate];
}

- (CVTexture *)getTexture
{
    CVTexture *ret;
    ret = [super getTexture];
    return ret;
}

- (void)freeFrame
{
    if (currentFrame) {
        CVPixelBufferRelease(currentFrame);
        currentFrame = nil;	
    }
}

- (void)reOpen
{
    Context *ctx = [freej getContext];
    if(!movie) return;

    [lock lock];
    skipCount = 0;
    [layerView setPosterImage:icon1];
	[self clearPreview];

    // register the layer within the freej context
    if (!layer) {
        CVFFmpegLayer *ffLayer = new CVFFmpegLayer(self);
        ffLayer->init(ctx->screen->geo.w, ctx->screen->geo.h, ctx->screen->geo.bpp);
        layer = ffLayer;
    }
    ((CVFFmpegLayer *)layer)->open(movie);
    [lock unlock];

    if (!currentFrame) {
        CVPixelBufferCreate(
		kCFAllocatorDefault,
		ctx->screen->geo.w,
		ctx->screen->geo.h,
		k32ARGBPixelFormat,
		(CFDictionaryRef)bufDict, 
		&currentFrame);
    }
}

- (void)feedFrame:(CVPixelBufferRef)frame
{
    [super feedFrame:frame];
    if (!previewImage) {
        Context *ctx = [freej getContext];
        CVFFmpegLayer *lay = (CVFFmpegLayer *)layer;
        CGLLockContext(glContext);
        CGLSetCurrentContext(glContext);
        
        CVPixelBufferLockBaseAddress(currentFrame, 0);
        uint8_t* buf= (uint8_t*) CVPixelBufferGetBaseAddress(currentFrame);
        /*
         printf("%dx%d -> %dx%d\n", get_scaled_width(ff), get_scaled_height(ff),
         ctx->screen->geo.w, ctx->screen->geo.h);
         */
        if (lay->scaledWidth() == ctx->screen->geo.w && lay->scaledHeight() == ctx->screen->geo.h) {
            uint8_t *ffbuf = (uint8_t *)lay->buffer;
            if (lay->buffer) {
                memcpy(buf, ffbuf, 4 * ctx->screen->geo.w * ctx->screen->geo.h *sizeof(uint8_t));
                /*
                long blackness=(ctx->screen->geo.w * ctx->screen->geo.h);
                // histogram
                blackness=0;
                int i;
                for (i=0; i< 4 * ctx->screen->geo.w * ctx->screen->geo.h *sizeof(uint8_t);i+=4) {
                    blackness+=((buf[i+1]>>2) || (buf[i+2]>>2) || (buf[i+3]>>2))?1:0;
                }
                */
                //if (blackness >= (ctx->screen->geo.w * ctx->screen->geo.h)>>4) {
                if (skipCount > 50) { // TODO - implement properly
    
                    NSAutoreleasePool *pool;
                    pool = [[NSAutoreleasePool alloc] init];
                    NSBitmapImageRep *imr = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char**)&buf 
                                                                                     pixelsWide:ctx->screen->geo.w
                                                                                     pixelsHigh:ctx->screen->geo.h
                                                                                  bitsPerSample:8
                                                                                samplesPerPixel:4
                                                                                       hasAlpha:YES
                                                                                       isPlanar:NO
                                                                                 colorSpaceName:NSCalibratedRGBColorSpace
                                                                                   bitmapFormat:NSAlphaFirstBitmapFormat
                                                                                    bytesPerRow:(4*ctx->screen->geo.w)
                                                                                   bitsPerPixel:32];
                    [lock lock];
                    previewImage = [[NSImage alloc] initWithSize:NSZeroSize];
                    [previewImage addRepresentation:imr];
                    [layerView setPosterImage:previewImage];
                    //[previewImage release]; // XXX - this will be released by clearPreview
                    [lock unlock];
                    [imr release];
                    [pool release];
                } else {
                    skipCount++;
                }
                layer->vbuffer = currentFrame;
                newFrame = YES;
            }
        }
        CVPixelBufferUnlockBaseAddress(currentFrame, 0);
        CGLUnlockContext(glContext);
    }
}

- (void)setStream:(NSString*)url
{
    if(movie) { 
        free(movie);
        movie = NULL;
    }
    if (url) {
        movie = strdup((char *)[url UTF8String]);
        printf("setStream: %s\n",movie);
        timeout=1;
        [self reOpen];
    } else {
        if (layer)
            ((CVFFmpegLayer *)layer)->close();
    }
}

- (void)setRepeat:(BOOL)repeat
{
    CVFFmpegLayer *lay = (CVFFmpegLayer *)layer;
    if (layer)
        lay->repeat = repeat;
}

@end
