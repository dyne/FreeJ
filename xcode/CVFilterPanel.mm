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
#include <CVFilterInstance.h>

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
    [activeFilters setDelegate:self];
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
        NSSlider *slider = firstImageParam;
        while (slider && [slider toolTip]) {
            NSNumber *value = [imageParams valueForKey:[slider toolTip]];
            if (value) {
                [slider setDoubleValue:[value floatValue]];
            } else {
                [slider setDoubleValue:([slider minValue]+[slider maxValue])/2];
            }
            slider = (NSSlider *)[[slider nextKeyView] nextKeyView];
        }
    }
    [lock unlock];
}

- (void)updateActiveFilters
{
    @synchronized(self) {
        // cleanup the tabview
        while ([activeFilters numberOfTabViewItems]) {
            NSTabViewItem *filterTab = [activeFilters tabViewItemAtIndex:0];
            [activeFilters removeTabViewItem:filterTab];
        }

        if (layer) {
            CVCocoaLayer *cLayer = layer.layer;
            if (cLayer) {
                Layer *fjLayer = cLayer->fjLayer();
                fjLayer->filters.lock();
                FilterInstance *filt = fjLayer->filters.begin();
                while (filt) {
                    NSData *pointer = [NSData dataWithBytesNoCopy:filt length:sizeof(CVFilterInstance *)];
                    NSTabViewItem *newItem = [[NSTabViewItem alloc] initWithIdentifier:pointer];
                    [newItem setLabel:[NSString stringWithUTF8String:filt->name]];
                    if (newItem) {
                        [activeFilters addTabViewItem:newItem];
                        //[newItem autorelease];
                    }
                    filt = (FilterInstance *)filt->next;
                }
                fjLayer->filters.unlock();
            }
        }
    }
}

- (IBAction)setImageParameter:(id)sender
{
    if (layer)
        [layer setImageParameter:sender];
}

- (IBAction)setFilterParameter:(id)sender
{
    NSData *data = [[activeFilters selectedTabViewItem] identifier];
    CVFilterInstance *currentFilter = (CVFilterInstance *)[data bytes];

    Parameter *param = currentFilter->proto->parameters.pick([sender tag]);
    double value = [sender doubleValue]/1000.0;
    param->multiplier = 1000.0;
    param->set(&value);
}

- (IBAction)removeFilter:(id)sender
{
    if (layer) {
        Layer *lay = layer.layer->fjLayer(); // XXX - cleanup this stuff
        lay->filters.lock();
        NSData *data = [[activeFilters selectedTabViewItem] identifier];
        CVFilterInstance *currentFilter = (CVFilterInstance *)[data bytes];
        delete currentFilter;
        lay->filters.unlock();
        [self updateActiveFilters];
    }
}

- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem
{
    if (layer) {
        int idx = 1;
        Layer *lay = layer.layer->fjLayer(); // XXX - cleanup this stuff
        
        @synchronized(self) {
            NSString *filterName = [tabViewItem label];
            Context *ctx = [cFreej getContext];
            ctx->filters.lock();
            Filter *filt = ctx->filters.search([filterName UTF8String]);
            NSView *container;
            container = [[NSView alloc] initWithFrame:[activeFilters frame]];
            [tabViewItem setView:container];
            [container autorelease];
            filt->parameters.lock();
            Parameter *param = filt->parameters.begin();
            NSRect frame = NSMakeRect(0, [container frame].size.height - 25, 0, 20); // XXX
            while (param) {
                frame.origin.x = 0;
                frame.size.width = [container frame].size.width / 2 - 10; // XXX
                NSTextView *newText = [[NSTextView alloc] initWithFrame:frame];
                [newText setDrawsBackground:NO];
                [newText setTextColor:[NSColor colorWithDeviceRed:1.0 green:1.0 blue:1.0 alpha:1.0]];
                [newText insertText:[NSString stringWithUTF8String:param->name]];
                [newText setFont:[NSFont labelFontOfSize:10.0]];
                [newText sizeToFit];
                [newText setEditable:NO];
                [container addSubview:newText];
                NSRect labelSize = [newText frame];
                frame.origin.x = labelSize.size.width;
                frame.origin.y = labelSize.origin.y + (labelSize.size.height/4)-1; // XXX
                frame.size.width = [container frame].size.width - frame.origin.x - 25;
                NSSlider *newSlider = [[NSSlider alloc] initWithFrame:frame];
                [newSlider setMinValue:0.0];
                // if the parameter express a center coordinate or an input radius, 
                // let's use the actual layer size to compute the maximum value
                // TODO - the same should be done for prameters expressing a radius size
                if (strcmp(param->name, "inputRadius") == 0) {
                    [newSlider setMaxValue:sqrt(exp2(lay->geo.w)+exp2(lay->geo.h))/2];
                }
                if (strcmp(param->name, "CenterX") == 0) {                    
                    [newSlider setMaxValue:lay->geo.w];
                } else if (strcmp(param->name, "CenterY") == 0) {
                    [newSlider setMaxValue:lay->geo.h];
                } else {
                    // otherwise let's use the max_size reported by the parameter itself
                    [newSlider setMaxValue:*(double *)param->max_value];
                }
                [newSlider setDoubleValue:*(double *)param->value]; // XXX - assumes number-only parameters
                [[newSlider cell] setControlSize:NSMiniControlSize];
                [newSlider setTarget:self];
                [newSlider setAction:@selector(setFilterParameter:)];
                [newSlider setTag:idx];
                [container addSubview:newSlider];
                frame.origin.y = labelSize.origin.y - 20;
                // everything should be properly retained at this point
                [newText release];
                [newSlider release];
                param = (Parameter *)param->next;
                idx++;
            }
            frame.origin.x = 0;
            frame.origin.y = 0;
            NSButton *removeButton = [[NSButton alloc] initWithFrame:frame];
            [removeButton setTitle:@"Remove"];
            [[removeButton cell] setControlSize:NSMiniControlSize];
            [removeButton setButtonType:NSMomentaryPushInButton];
            [removeButton setImagePosition:NSNoImage];
            [removeButton setBezelStyle:NSRoundedBezelStyle];
            [removeButton setFont:[NSFont labelFontOfSize:9.0]];
            [removeButton sizeToFit];
            [removeButton setAction:@selector(removeFilter:)];
            [container addSubview:removeButton];
            filt->parameters.unlock();
            ctx->filters.unlock();
        }
    } 
}

- (IBAction)addFilter:(id)sender
{
    if (layer) {        
        NSString *filterName = [[filterButton selectedItem] title];

        Context *ctx = [cFreej getContext];
        Filter *filt = ctx->filters.search([filterName UTF8String]);
        if (!filt) {
            NSLog(@"Can't find filter: %@\n", filterName);
            return;
        }
        CVCocoaLayer *cLay = layer.layer;
        if (!cLay) {
            NSLog(@"Can't add filter: No CVCocoaLayer found on %s.\n", [layer name]);
            return;
        }
        Layer *lay = cLay->fjLayer();
        
        Linklist<FilterInstance> *filters = [layer activeFilters];
        if (filters->len() >= 4) {
            NSLog(@"4-filters limit reached for layer %s", [layer name]);
            return;
        }
        CVFilterInstance *inst = new CVFilterInstance(filt);
        filt->apply(lay, inst);
        NSData *pointer = [NSData dataWithBytesNoCopy:inst length:sizeof(CVFilterInstance *)];
        NSTabViewItem *newItem = [[NSTabViewItem alloc] initWithIdentifier:pointer];
        if (newItem) {
            [newItem setLabel:filterName];
            [activeFilters addTabViewItem:newItem];
            //[newItem autorelease];
        }        
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
