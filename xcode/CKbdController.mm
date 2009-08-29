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
        windowController = [CVScreenController alloc];
        [windowController setWindow:[sView getWindow]];
    }
    return Controller::init(freej);
}

int CKbdController::dispatch()
{
    return 0;
}

int CKbdController::poll()
{
    return 0;
}

@implementation CVScreenController

// handle keystrokes
- (void)keyDown:(NSEvent *)event
{
    NSLog(@"Keypress (%hu, modifier flags: 0x%x) %@\n", [event keyCode], [event modifierFlags], [event charactersIgnoringModifiers]);
    NSString *modifier = nil;
    switch ([event modifierFlags]) { 
        case NSShiftKeyMask:
            modifier = [NSString stringWithUTF8String:"shift_"];
            break;
        case NSControlKeyMask:
            modifier = [NSString stringWithUTF8String:"control_"];
            break;
        case NSAlternateKeyMask:
            modifier = [NSString stringWithUTF8String:"alt_"];
            break;
        case NSCommandKeyMask:
            modifier = [NSString stringWithUTF8String:"cmd_"];
            break;
        case NSFunctionKeyMask:
            modifier = [NSString stringWithUTF8String:"fn_"];
            break;
    }
}

@end

