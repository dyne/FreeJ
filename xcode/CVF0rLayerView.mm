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

#include <CVF0rLayer.h>
#import <CVF0rLayerView.h>

@implementation CVF0rLayerView : CVLayerView

- (id)init
{
    static char *suffix = (char*)"/Contents/Resources/frei0r.png";
    char iconFile[1024];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(suffix)-1);
    strcat(iconFile, suffix);
    icon = [[NSImage alloc] initWithContentsOfURL:
            [NSURL fileURLWithPath:[NSString stringWithCString:iconFile encoding:NSASCIIStringEncoding]]];
    //posterImage = [[CIImage imageWithContentsOfURL:
    //                [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]] retain];    
    return [super init];
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    Context *ctx = [freej getContext];
    Filter *gen = ctx->generators.begin();
    while (gen) {
        [selectButton addItemWithTitle:[NSString stringWithCString:gen->name encoding:NSASCIIStringEncoding]];
        gen = (Filter *)gen->next;
    }
}

- (void)drawRect:(NSRect)theRect
{
    if (!posterImage)
        [self setPosterImage:icon];
    [super drawRect:theRect];
}

- (BOOL)isOpaque
{
    return NO;
}

- (IBAction)stop:(id)sender
{
    [(CVF0rLayerController *)layerController reset];
    [lock lock];
    [sender setTitle:@"Start"];
    [sender setAction:@selector(start:)];
    [selectButton setEnabled:YES];
    [lock unlock];
}

- (IBAction)start:(id)sender
{
    CVF0rLayer *newLayer = NULL;
    [lock lock];
    char *name = (char *)[[[selectButton selectedItem] title] UTF8String];
    newLayer = new CVF0rLayer(layerController, [freej getContext]);
    Context *ctx = [freej getContext];
    newLayer->init(ctx->screen->geo.w, ctx->screen->geo.h, ctx->screen->geo.bpp); // XXX 
    newLayer->register_generators(&ctx->generators);
    newLayer->open(name);
    if(newLayer) { 
        //[self setLayer:(CVLayer *)newLayer];
        [sender setTitle:@"Stop"];
        [sender setAction:@selector(stop:)];
        [selectButton setEnabled:NO];
        newLayer->start();
        notice("generator %s succesfully created", newLayer->name);
    } else {
        error("Can't create F0R layer %s", name);
    }
    [lock unlock];
}

@end
