/*
 *  CKbdController.h
 *  freej
 *
 *  Created by xant on 8/28/09.
 *  Copyright 2009 dyne.org. All rights reserved.
 *
 */

#include <factory.h>

class CKbdController: public Controller {
  private:
  
  public:
    CKbdController();
    ~CKbdController();

    bool init(Context *freej);
    int  poll();
    virtual int dispatch();
    FACTORY_ALLOWED
};