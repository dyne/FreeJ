/*
 *  kbd_ctrl_js.cpp
 *  freej
 *
 *  Created by xant on 8/22/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include <callbacks_js.h>
#include <jsparser_data.h>
#include <kbd_ctrl.h>
#include <config.h>

JSFunctionSpec js_kbd_ctrl_methods[] = {
// idee: dis/enable repeat
{0}
};

/////// Javascript KeyboardController
JS(js_kbd_ctrl_constructor);

DECLARE_CLASS("KeyboardController",js_kbd_ctrl_class, js_kbd_ctrl_constructor);

JS(js_kbd_ctrl_constructor) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    
    KbdController *kbd = (KbdController *)Factory<Controller>::new_instance( "KeyboardController" );
    if (!kbd)
        return JS_FALSE;
    
    // initialize with javascript context
    if(! kbd->init(global_environment) ) {
        error("failed initializing keyboard controller");
        delete kbd; return JS_FALSE;
    }
    
    // assign the real js object
    kbd->jsobj = obj;
    kbd->javascript = true;
    
    // assign instance into javascript object
    if( ! JS_SetPrivate(cx, obj, (void*)kbd) ) {
        error("failed assigning keyboard controller to javascript");
        delete kbd; return JS_FALSE;
    }
    
    *rval = OBJECT_TO_JSVAL(obj);
    return JS_TRUE;
}
