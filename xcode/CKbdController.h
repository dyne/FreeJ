/*
 *  CKbdController.h
 *  freej
 *
 *  Created by xant on 8/28/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */
#include <factory.h>
#include <controller.h>
#include <CFreej.h>
#include <CVScreen.h>
#import <Cocoa/Cocoa.h>

class CKbdController;

@interface CVScreenController : NSWindowController {
    CKbdController *_kbdController;
    NSMutableArray *_keyEvents;
}
- (NSDictionary *)getEvent;
- (void)setKbdController:(CKbdController *)kbdController;
@end

class CKbdController: public Controller {
  private:
    CVScreenController *windowController;
  public:
    CKbdController();
    ~CKbdController();

    bool init(Context *freej);
    int  poll();
    virtual int dispatch();
    FACTORY_ALLOWED
};