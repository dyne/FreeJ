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

#define toggle_bit(bf,b)	\
	(bf) = ((bf) & b)		\
	       ? ((bf) & ~(b))	\
	       : ((bf) | (b))


/////// Javascript WiiController
JS(js_wii_ctrl_constructor);

DECLARE_CLASS("WiiController", js_wii_ctrl_class, js_wii_ctrl_constructor);

JS(js_wii_ctrl_connect);
JS(js_wii_ctrl_actaccel);
JS(js_wii_ctrl_actbutt);
JS(js_wii_ctrl_rumble);
JS(js_wii_ctrl_actleds);

JSFunctionSpec js_wii_ctrl_methods[] = {
  {"connect",        js_wii_ctrl_connect,    1},
  {"toggle_accel",   js_wii_ctrl_actaccel,   0},
  {"toggle_buttons", js_wii_ctrl_actbutt,    0},
  {"toggle_rumble",  js_wii_ctrl_rumble,     0},
  {"toggle_led",     js_wii_ctrl_actleds,    1},

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

    if(argc>0) {
      char *addr;
      JS_ARG_STRING(addr,0);
      wii->connect(addr);
    } else
      wii->connect(NULL);
    
    return JS_TRUE;
}
JS(js_wii_ctrl_actaccel) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  char rpt_mode = 0;

  toggle_bit(rpt_mode, CWIID_RPT_ACC);
  cwiid_set_rpt_mode(wii->wiimote, rpt_mode);

  return JS_TRUE;
}

JS(js_wii_ctrl_actbutt) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  char rpt_mode = 0;

  toggle_bit(rpt_mode, CWIID_RPT_BTN);
  cwiid_set_rpt_mode(wii->wiimote, rpt_mode);

  return JS_TRUE;
}

JS(js_wii_ctrl_rumble) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  char rumble = 0;

  toggle_bit(rumble, 1);
  cwiid_set_rumble(wii->wiimote, rumble);

  return JS_TRUE;
}

JS(js_wii_ctrl_actleds) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);

  JS_CHECK_ARGC(1);

  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  char led_state = 0;

  JS_ARG_NUMBER(led,0);
  if     (led==1) toggle_bit(led_state, CWIID_LED1_ON);
  else if(led==2) toggle_bit(led_state, CWIID_LED2_ON);
  else if(led==3) toggle_bit(led_state, CWIID_LED3_ON);
  else if(led==4) toggle_bit(led_state, CWIID_LED4_ON);
  else error("there are only 4 leds on the wiimote");

  return JS_TRUE;
}

WiiController *tmp; // ARG. this is not reentrant because C sucks.
                    // PLEASE add a void *user_data in the cwiid_wiimote struct!!!!
void cwiid_callback(cwiid_wiimote_t *wii, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
    
  tmp->accel(     mesg[mesg_count-1].acc_mesg.acc[CWIID_X],
		  mesg[mesg_count-1].acc_mesg.acc[CWIID_Y],
		  mesg[mesg_count-1].acc_mesg.acc[CWIID_Z]   );

}

void WiiController::accel(uint8_t nx, uint8_t ny, uint8_t nz) {
  // simple threshold
  int thresh = 1;
  if ( (x-nx > thresh) || (nx-x > thresh) ) x = nx;
  if ( (y-ny > thresh) || (ny-y > thresh) ) y = ny;
  if ( (z-nz > thresh) || (nz-z > thresh) ) z = nz;

}

WiiController::WiiController()
:Controller() {
  
  tmp = this; // this shouldn't be here, when cwiid callback gets fixed

  set_name("WiiCtrl");
}

WiiController::~WiiController() {

  cwiid_close(wiimote);
  
}

bool WiiController::connect(char *hwaddr) {
  
  // if argument is NULL look for any wiimote
  if(hwaddr == NULL) bdaddr = *BDADDR_ANY;
  else str2ba(hwaddr,&bdaddr);
  
  notice("Detecting WiiMote (press A+B on it to handshake)");
  
  wiimote = cwiid_open(&bdaddr, 0);
  if(!wiimote) {
    error("unable to connect to WiiMote");
    return false;
  } else
    act("WiiMote connected");
  
  cwiid_set_data(wiimote, (void*)this);
  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
    error("unable to set wiimote message callback");
    cwiid_close(wiimote);
    return 0;
  }

  // activate acceleration by default (todo switches)
 
  // for more activation switches see wmdemo.c in cwiid
  unsigned char rpt_mode = 0;

  toggle_bit(rpt_mode, CWIID_RPT_ACC);
  cwiid_set_rpt_mode(wiimote, rpt_mode);

  cwiid_enable(wiimote, CWIID_FLAG_MESG_IFC); // enable messages
  
  return 1;
}

bool WiiController::init(JSContext *env, JSObject *obj) {

  jsenv = env;
  jsobj = obj;

  initialized = true;
  return(true);
  
}

int WiiController::poll() {

  if(active)
    dispatch();

    return 1;
}

int WiiController::dispatch() {
  //  int res;
  //  char funcname[512];
  //  char keyname[512];

  Controller::JSCall("acceleration", 3, "uuu", x, y, z );

  return 1;
}

int WiiController::print_state() {
	int i;
	int valid_source = 0;

	act("Report Mode:");
	if (state.rpt_mode & CWIID_RPT_STATUS) act(" STATUS");
	if (state.rpt_mode & CWIID_RPT_BTN) act(" BTN");
	if (state.rpt_mode & CWIID_RPT_ACC) act(" ACC");
	if (state.rpt_mode & CWIID_RPT_IR) act(" IR");
	if (state.rpt_mode & CWIID_RPT_NUNCHUK) act(" NUNCHUK");
	if (state.rpt_mode & CWIID_RPT_CLASSIC) act(" CLASSIC");
	
	act("Active LEDs:");
	if (state.led & CWIID_LED1_ON) act(" 1");
	if (state.led & CWIID_LED2_ON) act(" 2");
	if (state.led & CWIID_LED3_ON) act(" 3");
	if (state.led & CWIID_LED4_ON) act(" 4");

	act("Rumble: %s", state.rumble ? "On" : "Off");

	act("Battery: %d%%",
	       (int)(100.0 * state.battery / CWIID_BATTERY_MAX));

	act("Buttons: %X", state.buttons);

	act("Acc: x=%d y=%d z=%d", state.acc[CWIID_X], state.acc[CWIID_Y],
	       state.acc[CWIID_Z]);

	act("IR: ");
	for (i = 0; i < CWIID_IR_SRC_COUNT; i++) {
		if (state.ir_src[i].valid) {
			valid_source = 1;
			act("(%d,%d) ", state.ir_src[i].pos[CWIID_X],
			                   state.ir_src[i].pos[CWIID_Y]);
		}
	}
	if (!valid_source) {
		act("no sources detected");
	}

	switch (state.ext_type) {
	case CWIID_EXT_NONE:
		act("No extension");
		break;
	case CWIID_EXT_UNKNOWN:
		act("Unknown extension attached");
		break;
	case CWIID_EXT_NUNCHUK:
		act("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
		       "acc.z=%d", state.ext.nunchuk.buttons,
		       state.ext.nunchuk.stick[CWIID_X],
		       state.ext.nunchuk.stick[CWIID_Y],
		       state.ext.nunchuk.acc[CWIID_X],
		       state.ext.nunchuk.acc[CWIID_Y],
		       state.ext.nunchuk.acc[CWIID_Z]);
		break;
	case CWIID_EXT_CLASSIC:
		act("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
		       "l=%d r=%d", state.ext.classic.buttons,
		       state.ext.classic.l_stick[CWIID_X],
		       state.ext.classic.l_stick[CWIID_Y],
		       state.ext.classic.r_stick[CWIID_X],
		       state.ext.classic.r_stick[CWIID_Y],
		       state.ext.classic.l, state.ext.classic.r);
		break;
	}
	return 1;
}
