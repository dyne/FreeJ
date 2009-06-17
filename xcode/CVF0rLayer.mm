//
//  CVF0rLayer.m
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayer.h"

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
        toDelete = layer;
        layer = NULL;
        toDelete->stop();
        [lock unlock];
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
    GenF0rLayer *newLayer = NULL;
    [lock lock];
    char *name = (char *)[[[selectButton selectedItem] title] UTF8String];
    newLayer = new CVF0rLayer(self, name, [freej getContext]);
    if(!newLayer) 
        error("Can't create F0R layer %s", name);
    [sender setTitle:@"Stop"];
    [sender setAction:@selector(stop:)];
    [selectButton setEnabled:NO];
    notice("generator %s succesfully created", newLayer->name);
    [lock unlock];
}

@end

CVF0rLayer::CVF0rLayer(CVLayerView *view, char *generatorName, Context *_freej)
    : GenF0rLayer()
{
    input = view;
    freej = _freej;
    CVLayer *layerPersonality = (CVLayer *)this;
    GenF0rLayer *f0rPersonality = (GenF0rLayer *)this;
    layerPersonality->type = Layer::GL_COCOA;
    [input setLayer:this];
    blendMode = NULL;
    if (!f0rPersonality->init(freej)) {
        error("can't initialize generator layer");
        return;
    }
    if (!f0rPersonality->open(generatorName)) {
        error("generator %s hasn't been found", generatorName);
        return;
    }
    layerPersonality->init(freej, freej->screen->w, freej->screen->h);

    layerPersonality->set_name([[view toolTip] UTF8String]);
}

CVF0rLayer::~CVF0rLayer()
{

}

void *
CVF0rLayer::feed()
{
    GenF0rLayer::feed();
    [(CVF0rLayerView *)input feedFrame:swap_buffer]; 
    return swap_buffer;
}

void
CVF0rLayer::start()
{
    ((GenF0rLayer *)this)->start();
}

void
CVF0rLayer::stop()
{
    GenF0rLayer *f0rPersonality = (GenF0rLayer *)this;
    f0rPersonality->stop();
}
