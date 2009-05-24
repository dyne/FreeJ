//
//  CVF0rLayer.m
//  freej
//
//  Created by xant on 3/8/09.
//  Copyright 2009 dyne.org. All rights reserved.
//

#import "CVF0rLayer.h"

@implementation CVF0rLayerView : CVLayerView

- (void)feedFrame:(void *)frame
{
    //Context *ctx = (Context *)[freej getContext];
    [lock lock];
    if (currentFrame)
        CVPixelBufferRelease(currentFrame);
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
    [lock unlock];
    [self renderPreview];
}

- (void)drawRect:(NSRect)theRect
{
    char temp[256];
    GLint zeroOpacity = 0;
    [[self openGLContext] setValues:&zeroOpacity forParameter:NSOpenGLCPSurfaceOpacity];
    [super drawRect:theRect];
    ProcessSerialNumber psn;
    GetProcessForPID(getpid(), &psn);
    FSRef location;
    GetProcessBundleLocation(&psn, &location);
    // 238 == 256 - strlen("/Contents/Plugins") - 1
    FSRefMakePath(&location, (UInt8 *)temp, 238);
    strcat(temp, "/Contents/Resources/frei0r.png");
    CIImage *image = [CIImage imageWithContentsOfURL:[NSURL fileURLWithPath:[NSString stringWithCString:temp]]];
    NSRect bounds = [self bounds];
    NSRect frame = [self frame];
    CGRect  imageRect = CGRectMake(NSMinX(bounds), NSMinY(bounds),
        NSWidth(bounds), NSHeight(bounds));
    glClearColor(1, 1, 1, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    [ciContext drawImage:image
        atPoint: imageRect.origin
        fromRect: imageRect];
    [[self openGLContext] makeCurrentContext];
    [[self openGLContext] flushBuffer];

}

- (bool)isOpaque
{
    return NO;
}

@end

CVF0rLayer::CVF0rLayer(CVLayerView *view, char *file, Context *_freej)
    : GenF0rLayer()
{
    input = view;
    freej = _freej;
    [input setLayer:this];
    ((CVLayer *)this)->type = Layer::GL_COCOA;
    blendMode = NULL;
    if (!((GenF0rLayer *)this)->init(freej)) {
        error("can't initialize generator layer");
    }
    ((CVLayer *)this)->init(freej, freej->screen->w, freej->screen->h);
    if (!((GenF0rLayer *)this)->open(file)) {
          error("generator Partik0l is not found");
    }
    ((CVLayer *)this)->set_name("F0R");
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
