//
//  CVFilterPanel.m
//  freej
//
//  Created by xant on 2/26/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "CVFilterPanel.h"

#define FILTERS_MAX 17
static FilterParams fParams[FILTERS_MAX] =
{
    { 1, { { "inputAmount", 0.0, 50.0 } } },  // ZoomBlur
    { 1, { { "inputRadius", 1.0, 100.0 } } },  // BoxBlur
    //{ 2, { { "inputRadius", 0.0, 50.0 }, { "inputAngle", -3.14, 3.14 } } }, // MotionBlur
    { 1, { { "inputRadius", 0.0, 50.0 } } }, // DiscBlur
    { 1, { { "inputRadius", 0.0, 100.0 } } }, // GaussianBlur
    { 1, { { "inputLevels", 2.0, 30.0 } } }, // ColorPosterize
    { 0, { { NULL, 0.0, 0.0 } } }, // ComicEffect
    { 3, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 1.0, 100.0 } } }, // Crystalize
    { 1, { { "inputIntensity", 0.0, 10.0 } } }, // Edges
    { 1, { { "inputRadius", 0.0, 20.0 } } }, // EdgeWork
    { 1, { { "inputAngle", -3.14, 3.14 } } }, // HueAdjust
    { 3, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputScale", 1.0, 100.0 } } }, // HexagonalPixellate
    { 3, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.01, 1000.0 } } }, // HoleDistortion
    //{ 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 600.0 }, { "inputScale", -1.0, 1.0 } } }, // BumpDistortion
    { 3, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 1000.0 } } }, // CircleSplashDistortion
    { 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 600 }, { "inputAngle", -3.14, 3.14 } } }, // CircularWrap
    { 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 1000.0 }, { "inputScale", 0.0, 1.0 } } }, // PinchDistortion
    { 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 500 }, { "inputAngle", -12.57, 12.57 } } }, // TwirlDistortion
    { 4, { { "CenterX", 0.0, 100.0 }, { "CenterY", 0.0, 100.0 }, { "inputRadius", 0.00, 800 }, { "inputAngle", -94.25, 94.25 } } }, // VortexDistortion
};

@implementation CVFilterPanel
- (id) init
{
    //if (![super initWithWindowNibName:@"EffectsPanel"])
    //    return nil;
    layer = nil;
    lock = [[NSRecursiveLock alloc] init];
    [lock retain];
    return self;
}

- (void)dealloc
{
    [lock release];
    [super dealloc];
}
- (void)windowDidLoad
{
    //NSLog(@"Nib file loaded");
    
}

- (void)show
{
    // force opening the EffectsPanel under the mouse pointer
    NSPoint origin = [NSEvent mouseLocation];
    NSRect frame = [[self window] frame];
    origin.x -= frame.size.width/2;
    origin.y -= frame.size.height/2;
    if(layer) {
        [layer setPreviewTarget:previewBox];
        [showButton setState:[layer isVisible]?NSOnState:NSOffState];
        if ([layer needPreview]) {
            [previewButton setState:NSOnState];
        } else {
           [previewButton setState:NSOffState];
        }
    }
    [[self window] setFrameOrigin:origin];
    [[self window] makeKeyAndOrderFront:self];
    [previewBox clear];
}

- (void)hide
{
    if (layer) {
        //[layer stopPreview];
        [layer setPreviewTarget:nil];
        [previewBox clear];
    }
    layer = nil;
}

- (void)setLayer:(NSView *)lay
{
    [lock lock];
    if (layer)
        [layer stopPreview];
    layer = lay;
    [[self window] setTitle:[layer toolTip]];
    [self setFilterParameter:filterButton]; // configure default filter
    [lock unlock];
}

- (IBAction)setFilterParameter:(id)sender
{
    if(layer)
        [layer setFilterParameter:sender];
}

- (IBAction)togglePreview:(id)sender
{
    [previewBox clear];
    if(layer) 
        if ([previewButton intValue])
            [layer startPreview];
        else
            [layer stopPreview];
}

- (IBAction)toggleVisibility:(id)sender
{
    if (layer)
        if ([showButton state] == NSOnState)
            [layer activate];
        else
            [layer deactivate];
}

- (IBAction)setBlendMode:(id)sender
{
    if (layer)
        [layer setBlendMode:sender];
}
- (FilterParams *)getFilterParamsDescriptorAtIndex:(int)index
{
    if (index >= FILTERS_MAX)
        return nil;
    return &fParams[index];
}

@end

@implementation CVFilterBox

- (void)awakeFromNib
{
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow)
                owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
}


- (void)mouseEntered:(NSEvent *)theEvent {
}
 
- (void)mouseExited:(NSEvent *)theEvent {
    
    [[self window] orderOut:self];
    [filterPanel hide];

}



@end
