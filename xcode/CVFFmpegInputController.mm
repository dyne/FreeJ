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
#import <CVFFmpegInputView.h>
#import <CFreej.h>
#import <CIAlphaFade.h>

#include <jutils.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <CVFFmpegLayer.h>

@implementation CVFFmpegInputController : CVLayerController

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

    movie=NULL;
    preview=0;
	previewImage=NULL;
    timeout=1;
    currentFrame = nil;
#if 0 
	bufDict = [[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, nil] retain];

#else 
	bufDict = [[NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], kCVPixelBufferCGImageCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferCGBitmapContextCompatibilityKey, [NSNumber numberWithBool:YES], kCVPixelBufferOpenGLCompatibilityKey, nil] retain];
#endif
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
	if (preview && previewImage) {
		[previewImage release];
	}
	preview=0;
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

    [layerView setPosterImage:icon1];
	[self clearPreview];

    // register the layer within the freej context
    if (!layer) {
        layer = new CVFFmpegLayer(self);
        layer->init(ctx->screen->geo.w, ctx->screen->geo.h, ctx->screen->geo.bpp);
    }
    layer->open(movie);
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

- (CVReturn)renderFrame
{
    Context *ctx = [freej getContext];
    if (!currentFrame)
        return kCVReturnError;

    if (!movie)
		return kCVReturnError;

    CVFFmpegLayer *lay = (CVFFmpegLayer *)layer;
    [lock lock];
    if (lay->isDecoding()) {
		if (timeout>0 && layerView) {
			[layerView setPosterImage:icon0];
			[self clearPreview];
		}
		timeout=0;

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

                long blackness=(ctx->screen->geo.w * ctx->screen->geo.h);
    #if 1 // histogram
                blackness=0;
                if (!preview) {
                    int i;
                    for (i=0; i< 4 * ctx->screen->geo.w * ctx->screen->geo.h *sizeof(uint8_t);i+=4) {
                        blackness+=((buf[i+1]>>2) || (buf[i+2]>>2) || (buf[i+3]>>2))?1:0;
                    }
                }
    #endif
                if (!preview && blackness >= (ctx->screen->geo.w * ctx->screen->geo.h)>>4) {
                    NSAutoreleasePool *pool;
                    pool = [[NSAutoreleasePool alloc] init];
                    NSBitmapImageRep *imr = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:(unsigned char**)&buf 
                                                                      pixelsWide:ctx->screen->geo.w
                                                                      pixelsHigh:ctx->screen->geo.h
                                                                      bitsPerSample:8
                                                                      samplesPerPixel:4
                                                                      hasAlpha:YES
                                                                      isPlanar:NO
                                                                      colorSpaceName:NSCalibratedRGBColorSpace
                                                                      bitmapFormat:NSAlphaFirstBitmapFormat
                                                                      bytesPerRow:(4*ctx->screen->geo.w)
                                                                      bitsPerPixel:32] retain];
                    previewImage = [[[NSImage alloc] initWithSize:NSZeroSize] retain];
                    [previewImage addRepresentation:imr];
                    [layerView setPosterImage:previewImage];
                    preview=1;	
                    [imr release];
                    [pool release];
                }
                layer->vbuffer = currentFrame;
                newFrame = YES;
            }
		} else {
			printf("WARNING: scaled video and screen size mismatch!\n"); 
			[self reOpen];
		}
		CVPixelBufferUnlockBaseAddress(currentFrame, 0);
			CGLUnlockContext(glContext);
            
    } else if (!lay->hasFF()) {
        if (timeout == 1) {
			//printf("FFDEC: TIMEOUT!\n");
			if (layerView) {
				[layerView setPosterImage:icon1];
				[self clearPreview];
			}
		}
        if (timeout > 50) { 
            NSLog(@"Stream disconnected.");
            free(movie);
            movie=NULL;
            if (layerView) {
                [layerView setPosterImage:nil];
                [self clearPreview];
            }
        }
        //else if (timeout%8 == 0)
          //  [self reOpen];
        
        timeout++;
        CVPixelBufferRelease(currentFrame);
        currentFrame = NULL;
        lay->stop();
        [self deactivate];
        [lock unlock];
		return kCVReturnError;        
    }
    [lock unlock];

    [self renderPreview];
    return kCVReturnSuccess;
}

- (void)setStream:(NSString*)url
{
    if(movie) { 
        free(movie);
    }
    movie = strdup((char *)[url UTF8String]);
    printf("setStream: %s\n",movie);
    timeout=1;
    [self reOpen];
}

@end
