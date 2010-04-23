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

/* XXX - this is exactly the same code we have in trigger_ctrl.cpp ... 
         we should try to avoid duplicating code around */
JS(js_kbd_ctrl_constructor) {
    func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
    
    KbdController *kbd = (KbdController *)Factory<Controller>::get_instance( "KeyboardController" );
    if (!kbd)
        return JS_FALSE;
    
    JS_BeginRequest(cx);
    // initialize with javascript context
    if (!kbd->initialized) {
        if(! kbd->init(global_environment) ) {
            error("failed initializing keyboard controller");
            JS_EndRequest(cx);
            return JS_FALSE;
        }
        // mark that this controller was initialized by javascript
        kbd->javascript = true;        
    }

    // assign instance into javascript object
    if( !JS_SetPrivate(cx, obj, (void*)kbd) ) {
        error("failed assigning kbd controller to javascript");
        JS_EndRequest(cx);  
        return JS_FALSE;
    }
    
    *rval = OBJECT_TO_JSVAL(obj);
    kbd->add_listener(cx, obj);
    JS_EndRequest(cx);  
    return JS_TRUE;
}
