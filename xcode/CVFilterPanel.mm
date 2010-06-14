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

#import <CVFilterPanel.h>

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

- (void)setLayer:(CVLayerController *)lay
{
    [lock lock];
    if (layer)
        [layer stopPreview];
    layer = lay;
    [[self window] setTitle:[NSString stringWithUTF8String:[layer name]]];
    
    // check if a blendmode has been defined for this layer and update the selectbutton
    NSMenuItem *blendMode = [layer blendMode]?[blendModeButton itemWithTitle:[layer blendMode]]:nil;
    if (blendMode)
        [blendModeButton selectItem:blendMode];
    else
        [blendModeButton selectItemAtIndex:0];
    [self updateActiveFilters];
#if 0
    /* check if the layer has an already configured filter */
    NSString *filter = [layer filterName];
    if (filter) {
        [filterButton selectItemWithTitle:filter];
    } else {
        filter = [filterButton itemTitleAtIndex:0];
    }
    [self setFilterParameter:filterButton]; // configure default filter
#endif
    /* TODO - move this logic out of the FilterPanel. This should really be done in the 
       CVLayerView implementation perhaps in setFilterParameter , but it would be better 
       to do this in a sort of notification callback executed when the filterpanel attaches
       the CVLayer */
    NSDictionary *imageParams = [layer imageParams];
    /* restore filter selection and parameters for this specific layer */
    if (imageParams) {
        /* restore image parameters (brightness, contrast, exposure and such) */
        NSSlider *imageParam = firstImageParam;
        while ([imageParam tag] <= [lastImageParam tag]) {
            NSNumber *value = [imageParams valueForKey:[imageParam toolTip]];
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

- (void)updateActiveFilters
{
    NSArray *actualTabs = [activeFilters tabViewItems];
    for (int i = 0; i < [actualTabs count]; i++) {
        [activeFilters removeTabViewItem:[actualTabs objectAtIndex:i]];
    }

    if (layer) {
        CVCocoaLayer *cLayer = layer.layer;
        if (cLayer) {
            Layer *fjLayer = cLayer->fj_layer();
            FilterInstance *filt = fjLayer->filters.begin();
            while (filt) {
                NSTabViewItem *newItem = [[NSTabViewItem alloc] initWithIdentifier:(id)filt];
                [newItem setLabel:[NSString stringWithUTF8String:filt->name]];
                if (newItem)
                    [activeFilters addTabViewItem:newItem];
                filt = (FilterInstance *)filt->next;
            }
        }
    }
}

- (IBAction)setFilterParameter:(id)sender
{
    if(layer) // propagate the event if we have a controlling layer
        [layer setFilterParameter:sender];
}

- (IBAction)addFilter:(id)sender
{
/*
    if ([activeFilters numberOfTabViewItems] == 4) {
        NSLog(@"4-filters limit reached for layer %s", [layer name]);
        return;
    }
    NSTabViewItem *newItem = [[NSTabViewItem alloc] initWithIdentifier:self];
    [newItem setLabel:@""];
    if (newItem)
        [activeFilters addTabViewItem:newItem];
    NSRect frame = NSMakeRect(0, 0, 100, 20);
    NSView *container = [[NSView alloc] initWithFrame:[activeFilters frame]];
    NSSlider *newSlider = [[NSSlider alloc] initWithFrame:frame];
    [newSlider setMinValue:0.0];
    [newSlider setMaxValue:100.0];
    frame.origin.x = 50;
    NSTextField *newText = [[NSTextField alloc] initWithFrame:frame];
    [newText setStringValue:@"BLAH"];

    NSRect frame2 = NSMakeRect(0, 50, 100, 20);

    NSSlider *newSlider2 = [[NSSlider alloc] initWithFrame:frame2];
    [[newSlider cell] setControlSize:NSMiniControlSize];
    [container addSubview:newSlider];
    //[newSlider sizeToFit];
    [container addSubview:newText];
    [newText sizeToFit];
    [container addSubview:newSlider2];
    [newSlider2 sizeToFit];

    [newItem setView:container];
    NSLog(@"%d", [activeFilters numberOfTabViewItems]);
*/
    if (layer) {        
        NSString *filterName = [[filterButton selectedItem] title];

        CFreej *cFreej = layer.freej;
        Context *ctx = [cFreej getContext];
        Filter *filt = ctx->filters.search([filterName UTF8String]);
        CVCocoaLayer *cLay = layer.layer;
        Layer *lay = cLay->fj_layer();
        
        Linklist<FilterInstance> *filters = [layer activeFilters];
        if (filters->len() >= 4) {
            NSLog(@"4-filters limit reached for layer %s", [layer name]);
            return;
        }
        filt->apply(lay);
       // NSString *filterName = [NSString stringWithFormat:@"CI%@", [[sender selectedItem] title]];
        
    }
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
    if (layer) {
        NSString *blendMode = [[sender selectedItem] title];
        [layer setBlendMode:blendMode];
    }
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
