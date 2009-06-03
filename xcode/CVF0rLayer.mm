//
//  CVF0rLayer.m
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayer.h"

@implementation CVF0rLayerView : CVLayerView

- (id)init
{
    static char *postfix = "/Contents/Resources/frei0r.png";
    char iconFile[1024];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    FSRefMakePath(&location, (UInt8 *)iconFile, sizeof(iconFile)-strlen(postfix)-1);
    strcat(iconFile, postfix);
    icon = [CIImage imageWithContentsOfURL:
        [NSURL fileURLWithPath:[NSString stringWithCString:iconFile]]];
    [icon retain];
    currentFrame = NULL;
    return [super init];
}

- (void)feedFrame:(void *)frame
{
    //Context *ctx = (Context *)[freej getContext];
    //[lock lock];
    CVPixelBufferRef newPixelBuffer;
    
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
        &currentFrame
    ); 
	
    //if (err == kCVReturnSuccess)
    newFrame = YES;
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
        [[self openGLContext] setValues:&zeroOpacity forParameter:NSOpenGLCPSurfaceOpacity];
        [super drawRect:theRect];
        
        glClearColor(1, 1, 1, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        [ciContext drawImage:icon
            atPoint: imageRect.origin
            fromRect: imageRect];
        [[self openGLContext] makeCurrentContext];
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

@end

CVF0rLayer::CVF0rLayer(CVLayerView *view, char *generatorName, Context *_freej)
    : GenF0rLayer()
{
    input = view;
    freej = _freej;
    CVLayer *layerPersonality = (CVLayer *)this;
    GenF0rLayer *f0rPersonality = (GenF0rLayer *)this;
    [input setLayer:this];
    layerPersonality->type = Layer::GL_COCOA;
    blendMode = NULL;
    if (!f0rPersonality->init(freej)) {
        error("can't initialize generator layer");
    }
    layerPersonality->init(freej, freej->screen->w, freej->screen->h);
    if (f0rPersonality->open(generatorName)) {
          error("generator %s hasn't been found", generatorName);
    }
    layerPersonality->set_name("F0R");
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
