/*  FreeJ
 *  (c) Copyright 2009 Xant <xant@dyne.org>
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

#import "CVFilterPanel.h"

#define FILTERS_MAX 18
static FilterParams fParams[FILTERS_MAX] =
{
    { 1, { { "inputAmount", 0.0, 50.0 } } },  // ZoomBlur
    { 1, { { "inputRadius", 1.0, 100.0 } } },  // BoxBlur
    //{ 2, { { "inputRadius", 0.0, 50.0 }, { "inputAngle", -3.14, 3.14 } } }, // MotionBlur
    { 1, { { "inputRadius", 0.0, 50.0 } } }, // DiscBlur
    { 1, { { "inputRadius", 0.0, 100.0 } } }, // GaussianBlur
    { 1, { { "inputLevels", 2.0, 30.0 } } }, // ColorPosterize
    { 0, { { NULL, 0.0, 0.0  } } }, // ColorInvert
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

- (void)awakeFromNib
{
    initialWindowFrame = [[self window] frame];
    initialFrame = [mainView frame];
    initialBounds = [mainView bounds];
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

- (void)setLayer:(CVLayerView *)lay
{
    [lock lock];
    if (layer)
        [layer stopPreview];
    layer = lay;
    [[self window] setTitle:[layer toolTip]];
    /* check if the layer has an already configured filter */
    NSString *filter = [layer filterName];
    if (filter) {
        [filterButton selectItemWithTitle:filter];
    } else {
        filter = [filterButton itemTitleAtIndex:0];
    }
    [self setFilterParameter:filterButton]; // configure default filter

    /* TODO - move this logic out of the FilterPanel. This should really be done in the 
       CVLayerView implementation perhaps in setFilterParameter , but it would be better 
       to do this in a sort of notification callback executed when the filterpanel attaches
       the CVLayer */
    NSDictionary *filterParams = [layer filterParams];
    /* restore filter selection and parameters for this specific layer */
    if (filterParams) {
        /* restore image parameters (brightness, contrast, exposure and such) */
        NSSlider *imageParam = firstImageParam;
        while ([imageParam tag] <= [lastImageParam tag]) {
            NSNumber *value = [filterParams valueForKey:[imageParam toolTip]];
            if (value) {
                [imageParam setDoubleValue:[value floatValue]];
            } else {
                [imageParam setDoubleValue:([imageParam minValue]+[imageParam maxValue])/2];
            }
            imageParam = (NSSlider *)[[imageParam nextKeyView] nextKeyView];
        }
    }
    [lock unlock];
}

- (IBAction)setFilterParameter:(id)sender
{
    if(layer) // propagate the event if we have a controlling layer
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


// handle keystrokes
- (void)keyDown:(NSEvent *)event
{
    NSLog(@"Keypress (%hu) %@\n", [event keyCode], [event characters]);
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
    NSPoint eventLocation = [theEvent locationInWindow];
    NSRect bounds = [self bounds];
    // check if the pointer is really outside the panel's bounds
    if (eventLocation.x <= 0 || eventLocation.x >= bounds.size.width ||
        eventLocation.y <= 0 || eventLocation.y >= bounds.size.height)
    {
        [[self window] orderOut:self];
        [filterPanel hide];
    }
}

@end
