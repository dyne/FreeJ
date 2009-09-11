//
//  CVF0rLayerView.mm
//  freej
//
//  Created by xant on 8/30/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayerView.h"

@implementation CVF0rLayerView : CVLayerView

- (id)init
{
    static char *suffix = "/Contents/Resources/frei0r.png";
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

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    Context *ctx = [freej getContext];
    Filter *gen = ctx->generators.begin();
    while (gen) {
        [selectButton addItemWithTitle:[NSString stringWithCString:gen->name]];
        gen = (Filter *)gen->next;
    }
    return [super prepareOpenGL];
}

- (void)drawRect:(NSRect)theRect
{
    if (!posterImage)
        [self setPosterImage:icon];
    [super drawRect:theRect];
}

- (bool)isOpaque
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
