/*
 *  CKbdController.cpp
 *  freej
 *
 *  Created by xant on 8/28/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */

#include "CKbdController.h"

FACTORY_REGISTER_INSTANTIATOR(Controller, CKbdController, KeyboardController, cocoa);

CKbdController::CKbdController() 
    : Controller()
{
    windowController = NULL;
}

CKbdController::~CKbdController()
{
    if (windowController)
        [windowController release];
}

bool CKbdController::init(Context *freej) {
    if (freej) {
        // XXX - we are assuming we will always get a CVSCreen on OSX
        CVScreen *screen = (CVScreen *)freej->screen;
        CVScreenView *sView = screen->get_view();
        windowController = [[CVScreenController alloc] initWithWindow:[sView getWindow]];
        [windowController setKbdController:this];
    }
    return Controller::init(freej);
}

int CKbdController::dispatch()
{
    return 0;
}

int CKbdController::poll()
{
    NSDictionary *entry;
    while (entry = [windowController getEvent]) {
        char funcname[256];
        static jsval fval = JSVAL_NULL;
        NSEvent *event = [entry valueForKey:@"event"];
        NSString *state = [entry valueForKey:@"state"];
        NSUInteger modifierFlags = [event modifierFlags];
        snprintf(funcname, sizeof(funcname), "%s_%s%s%s%s%s",
                 [state UTF8String],
                 ((modifierFlags&NSShiftKeyMask)?      "shift_" : ""),
                 ((modifierFlags&NSControlKeyMask)?    "ctrl_"  : ""),
                 ((modifierFlags&NSAlternateKeyMask)?  "alt_"   : ""),
                 ((modifierFlags&NSNumericPadKeyMask)? "num_"   : ""),
                 [[event charactersIgnoringModifiers] UTF8String]
        );
        func("%s calling method %s()", __func__, funcname);
        [entry release];
        return JSCall(funcname, 0, &fval);
    }
    return 0;
}

@implementation CVScreenController

- (id)initWithWindow:(NSWindow *)window
{
    _keyEvents = [[NSMutableArray arrayWithCapacity:100] retain];
    return [super initWithWindow:window];
}

- (NSDictionary *)getEvent
{
    NSDictionary *event = NULL;
    @synchronized(self) {
        if ([_keyEvents count]) {
            event = [_keyEvents objectAtIndex:0];
            [_keyEvents removeObjectAtIndex:0];
        }
    }
    return event;
}

- (void)setKbdController:(CKbdController *)kbdController
{
    _kbdController = kbdController;
}


- (void)insertEvent:(NSEvent *)event OfType:(NSString *)type WithState:(NSString *)state
{
    NSDictionary *entry;
    // create the entry
    entry = [[NSDictionary 
              dictionaryWithObjects:
                [NSArray arrayWithObjects:
                    event, state, type, nil
                ]
              forKeys:
                [NSArray arrayWithObjects:
                    @"event", @"state", @"type", nil
                ]
            ] retain];
    @synchronized(self) {
        [_keyEvents addObject:entry];
    }
}

- (void)keyUp:(NSEvent *)event
{
    //NSLog(@"Keyrelease (%hu, modifier flags: 0x%x) %@\n", [event keyCode], [event modifierFlags], [event charactersIgnoringModifiers]);
    [self insertEvent:[event retain] OfType:@"kbd" WithState:@"released"];
}

// handle keystrokes
- (void)keyDown:(NSEvent *)event
{
    //NSLog(@"Keypress (%hu, modifier flags: 0x%x) %@\n", [event keyCode], [event modifierFlags], [event charactersIgnoringModifiers]);
    [self insertEvent:event OfType:@"kbd" WithState:@"pressed"];
}

@end

