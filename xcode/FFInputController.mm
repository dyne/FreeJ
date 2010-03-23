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
#import <FFInputController.h>
#import <CFreej.h>
#import <CIAlphaFade.h>

#include <jutils.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <libavutil/pixfmt.h>

extern "C" {
#include "ffdec.h"
}

@implementation FFInputController : CVLayerController

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

    ff = NULL;
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
    //fprintf(stdout, "reOpen: %s\n",movie);

    [lock lock];
    if (ff) {
	//fprintf(stderr,"FFdec: DEBUG: close stream");
	close_and_free_ff(&ff);
        ff=NULL;
    }

    [layerView setPosterImage:icon1];
	[self clearPreview];

    ffdec_thread(&ff, movie, ctx->screen->geo.w, ctx->screen->geo.h, PIX_FMT_ARGB); 


    // register the layer within the freej context
    if (!layer) {
	layer = new CVLayer(self);
	layer->init();
    }
    [lock unlock];

    if (!currentFrame) {
	// http://developer.apple.com/mac/library/DOCUMENTATION/GraphicsImaging/Reference/CoreVideoRef/Reference/reference.html#//apple_ref/c/func/CVOpenGLBufferCreate
       
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
    if (!currentFrame) {
	return kCVReturnError;
    }
    if (!ff && movie) {
        if (timeout == 1) {
			//printf("FFDEC: TIMEOUT!\n");
			if (layerView) {
				[layerView setPosterImage:icon1];
				[self clearPreview];
			}
		}
        if (timeout > 50) { 
		NSLog(@"Stream disconnected.");
        [self tactelldel:movie];
		free(movie);
		movie=NULL;
#if 1
	    if (layerView) {
			[layerView setPosterImage:nil];
			[self clearPreview];
	    }
#endif
#if 0
		[self freeFrame];
#endif
        }
        else if (timeout%8 == 0) [self reOpen];

        timeout++;
		return kCVReturnError;
    }

    if (!movie) {
		return kCVReturnError;
    }

    [lock lock];
    if (ff && (get_pt_status(ff)&3) ==1) {
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
		if (get_scaled_width(ff) == ctx->screen->geo.w && get_scaled_height(ff) == get_scaled_height(ff)) {
			uint8_t *ffbuf = get_bufptr(ff);
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
		} else {
			printf("WARNING: scaled video and screen size mismatch!\n"); 
			[self reOpen];
		}
		CVPixelBufferUnlockBaseAddress(currentFrame, 0);
			CGLUnlockContext(glContext);

		newFrame = YES;
    }

    if (layer)
        layer->vbuffer = currentFrame;
    [lock unlock];

    if (1) // TODO: iFPScount > oFPScount 
    if (ff && !ffdec_thread(&ff, NULL, 0, 0, 0) && ff) {
    ; // ifpsc+=get_fps(ff);
    }

    [self renderPreview];
    return kCVReturnSuccess;
}

- (void)setStream:(NSString*)url
{
    if(movie) { 
        [self tactelldel:movie];
        free(movie);
    }
    movie = strdup((char *)[url UTF8String]);
    printf("setStream: %s\n",movie);
    timeout=1;
    [self tactelladd:movie];
    [self reOpen];
}

// TODO: use Xcode-properties to get ID of current controller
// addLayer knows our "name" ..

- (void)tactelldel:(const char *)mv
{
    Context *ctx = (Context *)[freej getContext];
    if(!ctx->metadata) return;
	if (strncasecmp(mv,"http://",7)) return;

    FlowMixerMetaData *m= (FlowMixerMetaData*)ctx->metadata;
    if (m->streamurl1 && mv && !strcmp(m->streamurl1,mv)) {
        if (m->streamdel1) free(m->streamurl1);
        m->streamdel1=m->streamurl1;
        m->timeout=0;
        m->streamurl1=NULL;
    }
    if (m->streamurl2 && mv && !strcmp(m->streamurl2,mv)) {
        if (m->streamdel2) free(m->streamurl2);
        m->streamdel2=m->streamurl2;
        m->timeout=0;
        m->streamurl2=NULL;
    }
}

- (void)tactelladd:(const char *)mv 
{
    Context *ctx = (Context *)[freej getContext];
    if(!ctx->metadata) return;
	if (strncasecmp(mv,"http://",7)) return;

    FlowMixerMetaData *m= (FlowMixerMetaData*)ctx->metadata;
    if (!m->streamurl1) m->streamurl1 = strdup(mv);
    else if (!m->streamurl2) m->streamurl2 = strdup(mv);
    else {
     fprintf(stderr,"TAC-notification failed to schedule '%s'\n",mv);
    }
    m->timeout=0;
}

@end
