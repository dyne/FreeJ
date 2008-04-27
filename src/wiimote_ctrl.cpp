/*  FreeJ WiiMote controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *
 * based on libwiimote by Hector Martin (marcan)
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
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
 * "$Id: $"
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <config.h>

#include <wiimote_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h>
#include <jsparser_data.h>


/////// Javascript WiiController
JS(js_wii_ctrl_constructor);

DECLARE_CLASS_GC("WiiController", js_wii_ctrl_class, js_wii_ctrl_constructor, js_ctrl_gc);

JS(js_wii_ctrl_connect);

JSFunctionSpec js_wii_ctrl_methods[] = {
  {"connect",      js_wii_ctrl_connect,      0},
  {0}
};

JS(js_wii_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];
  
  WiiController *wii = new WiiController();
  // initialize with javascript context
  if(! wii->init(cx, obj) ) {
    sprintf(excp_msg, "failed initializing WiiMote controller");
    goto error;
  }

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)wii) ) {
    sprintf(excp_msg, "failed assigning WiiMote controller to javascript");
    goto error;
  }

  notice("WiiMote controller attached");

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  //  cx->newborn[GCX_OBJECT] = NULL;
  delete wii; return JS_FALSE;
}

JS(js_wii_ctrl_connect) {
    func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
    WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
    if(!wii) JS_ERROR("Wii core data is NULL");
    
    wii->connect(NULL);

    return JS_TRUE;
}

WiiController::WiiController()
:Controller() {
  
  set_name("WiiCtrl");
}

WiiController::~WiiController() {

  // need to find out how to cleanup?
  // no mention about it in libwii
  //
  // disconnect!!! ftw !!!
  
}

bool WiiController::connect(char *hwaddr) {
  int res;

  // if argument is NULL look for any wiimote
  if(hwaddr == NULL) wiimote_bdaddr = *BDADDR_ANY;
  else str2ba(hwaddr,&wiimote_bdaddr);
  
  act("Detecting WiiMote (press A+B on it to handshake)");
  
  res = wiimote_connect(wiimote_bdaddr, &connection);
  if(res == 0) act("WiiMote connected");
  
  wiimote_init(&wiimote, connection);
  wiimote_mode(&wiimote, WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6, 1);
  wiimote_set_ir(&wiimote, WM_IR_BASIC);
  wiimote_set_leds(&wiimote, WM_LED_2|WM_LED_3);

}

bool WiiController::init(JSContext *env, JSObject *obj) {

  jsenv = env;
  jsobj = obj;

  initialized = true;
  return(true);
  
}

int WiiController::poll() {
	// check if there are pending commands
	int res;
	res = wiimote_process_reports(&wiimote);

	if(res<0) { 
		error("error processing wiimote reports: deactivating controller");
		active = false;
	}
	else if(res>0) { // there are input reports
		dispatch();
	} 
	else return 0;
}

int WiiController::dispatch() {
  int res;
  char funcname[512];
  char keyname[512];

  // gather input data structure
  input = wiimote_inputdata(&wiimote);

  // buttons
  if( input->types & WM_DATA_BUTTONS ) {

    memset(keyname, 0, sizeof(char)<<9);  // *512
    memset(funcname, 0, sizeof(char)<<9); // *512
    
    if( input->buttons & WM_BTN_ONE )    JSCall("button_1", 0, NULL, &res);
    if( input->buttons & WM_BTN_TWO )    JSCall("button_2",  0, NULL, &res);
    if( input->buttons & WM_BTN_A )      JSCall("button_A", 0, NULL, &res);
    if( input->buttons & WM_BTN_B )      JSCall("button_B", 0, NULL, &res);
    if( input->buttons & WM_BTN_MINUS )  JSCall("button_minus", 0, NULL, &res);
    if( input->buttons & WM_BTN_PLUS )   JSCall("button_plus", 0, NULL, &res);
    if( input->buttons & WM_BTN_HOME )   JSCall("button_home", 0, NULL, &res);
    if( input->buttons & WM_BTN_LEFT )   JSCall("button_left", 0, NULL, &res);
    if( input->buttons & WM_BTN_RIGHT )  JSCall("button_right", 0, NULL, &res);
    if( input->buttons & WM_BTN_UP )     JSCall("button_up", 0, NULL, &res);
    if( input->buttons & WM_BTN_DOWN )   JSCall("button_down", 0, NULL, &res);

  }
  
  // accelerometer
  if( input->types & WM_DATA_ACCEL ) {

    Controller::JSCall("acceleration", 3, "uuu",
		       input->accel.x, input->accel.y, input->accel.z );

  }

  if( input->types & WM_DATA_EXT ) {
    switch(input->extension.type_id) {
    case WM_EXT_NONE: break;
    case WM_EXT_LOOSE: warning("WiiMote extension is loose"); break;
    case WM_EXT_NUNCHUK:
      wiimote_decode_nunchuk(&wiimote, &input->extension, &nunchuk);
      Controller::JSCall("nunchuk_acceleration", 3, "uuu",
			 nunchuk.accel.x, nunchuk.accel.y, nunchuk.accel.z);
      Controller::JSCall("nunchuk_stick", 2, "uu",
			 nunchuk.stick.x, nunchuk.stick.y);
      if( nunchuk.buttons & WM_NUNCHUK_BTN_Z )
	JSCall("nunchuk_button_Z", 0, NULL, &res);
      if( nunchuk.buttons & WM_NUNCHUK_BTN_C )
	JSCall("nunchuk_button_C", 0, NULL, &res);

      break;
    default: break;
    }
  }

}

