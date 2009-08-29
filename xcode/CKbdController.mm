/*
 *  CKbdController.cpp
 *  freej
 *
 *  Created by xant on 8/28/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */

#include "CKbdController.h"

#define MMASK_SHIFT (1)
#define MMASK_CTRL  (1<<1)
#define MMASK_ALT   (1<<2)
#define MMASK_CMD   (1<<3)
#define MMASK_FN    (1<<4)
#define MMASK_NUM   (1<<5)

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
        char modifier = 0;
        static jsval fval = JSVAL_NULL;
        NSEvent *event = [entry valueForKey:@"event"];
        NSString *state = [entry valueForKey:@"state"];
        switch ([event modifierFlags]) { 
            case NSShiftKeyMask:
                modifier |= MMASK_SHIFT;
                break;
            case NSControlKeyMask:
                modifier |= MMASK_CTRL;
                break;
            case NSAlternateKeyMask:
                modifier |= MMASK_ALT;
                break;
            case NSCommandKeyMask:
                modifier |= MMASK_CMD;
                break;
            case NSFunctionKeyMask:
                modifier |= MMASK_FN;
                break;
            case NSNumericPadKeyMask:
                modifier |= MMASK_NUM;
                break;
        }
        snprintf(funcname, sizeof(funcname), "%s_%s%s%s%s%s",
                 [state UTF8String],
                 ((modifier&MMASK_SHIFT)? "shift_" : ""),
                 ((modifier&MMASK_CTRL)?  "ctrl_"  : ""),
                 ((modifier&MMASK_ALT)?   "alt_"   : ""),
                 ((modifier&MMASK_NUM)?   "num_"   : ""),
                 [[event charactersIgnoringModifiers] UTF8String] );
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
    @synchronized(self) {
        // create the entry
        entry = [[NSDictionary 
                  dictionaryWithObjects:
                    [NSArray arrayWithObjects:event, state, type, nil]
                  forKeys:
                    [NSArray arrayWithObjects:@"event", @"state", @"type", nil]
                ] retain];

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

