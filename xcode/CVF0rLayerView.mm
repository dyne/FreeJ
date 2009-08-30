//
//  CVF0rLayerView.mm
//  freej
//
//  Created by xant on 8/30/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CVF0rLayerView.h"

@implementation CVF0rLayerView : CVLayerView

- (void)prepareOpenGL
{
    [super prepareOpenGL];
    Context *ctx = [freej getContext];
    Filter *gen = ctx->generators.begin();
    while (gen) {
        [selectButton addItemWithTitle:[NSString stringWithCString:gen->name]];
        gen = (Filter *)gen->next;
    }
    
}
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
    icon = [CIImage imageWithContentsOfURL:
            [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]];
    [icon retain];
    currentFrame = NULL;
    return [super init];
}

- (void)feedFrame:(void *)frame
{
    CVPixelBufferRef newPixelBuffer;
    //Context *ctx = (Context *)[freej getContext];
    [lock lock];
    CVReturn err = CVPixelBufferCreateWithBytes (
                                                 NULL,
                                                 layer->geo.w,
                                                 layer->geo.h,
                                                 k32ARGBPixelFormat,
                                                 frame,
                                                 layer->geo.w*4,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 &newPixelBuffer
                                                 ); 
    if (err == kCVReturnSuccess) {
        if (currentFrame)
            CVPixelBufferRelease(currentFrame);
        currentFrame = newPixelBuffer;
        newFrame = YES;
    }
    [lock unlock];
    [self renderPreview];
}

- (void)drawRect:(NSRect)theRect
{
    GLint zeroOpacity = 0;
    if (needsReshape) {
        NSRect bounds = [self bounds];
        NSRect frame = [self frame];
        CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
                                       NSWidth(bounds), NSHeight(bounds));
        if( kCGLNoError != CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]) )
            return;
        [[self openGLContext] makeCurrentContext];
        
        [[self openGLContext] setValues:&zeroOpacity forParameter:NSOpenGLCPSurfaceOpacity];
        [super drawRect:theRect];
        
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        [ciContext drawImage:icon
                     atPoint: imageRect.origin
                    fromRect: imageRect];
        [[self openGLContext] flushBuffer];
        needsReshape = NO;
        CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);       
    }
    [self setNeedsDisplay:NO];
}

- (bool)isOpaque
{
    return NO;
}

- (void) setLayer:(CVLayer *)lay
{
    if (layer) // ensure to remove/stop old genf0rlayer if we are setting a new one
        [self reset];
    [super setLayer:lay];
}

- (void)reset
{
    CVLayer *toDelete;
    if (layer) {
        [lock lock];
        toDelete = (CVLayer *)layer;
        layer = NULL;
        [lock unlock];
        toDelete->stop();
        delete toDelete;
    }
}

- (IBAction)stop:(id)sender
{
    [self reset];
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
    newLayer = new CVF0rLayer(self, name, [freej getContext]);
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
