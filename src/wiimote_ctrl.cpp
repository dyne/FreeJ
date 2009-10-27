/*  FreeJ WiiMote controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
 *
 * based on libcwiid by L. Donnie Smith
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

#include <config.h>
#ifdef WITH_CWIID

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wiimote_ctrl.h>

#include <context.h>
#include <jutils.h>

#include <callbacks_js.h>
#include <jsparser_data.h>

#define toggle_bit(set,val,bit) \
	(set ? val | bit : val & ~bit)

#define GET_PRIVATE(item_class, item) \
item_class *item = static_cast<item_class *> (JS_GetPrivate(cx,obj)); \
if(!item) { \
  error("%u:%s:%s :: JS core data is NULL", \
  __LINE__,__FILE__,__FUNCTION__); \
  return JS_FALSE; \
}

#define JS_FUNC_CALL(fun, item_class, value) \
JS(fun) { \
  *rval = JSVAL_TRUE; \
  GET_PRIVATE(item_class, item) \
\
  if(!item->value ) { \
    warning("cannot value %s %s", item->name, ""#value); \
    *rval = JSVAL_FALSE; \
  } \
  return JS_TRUE; \
}

#define JS_FUNC_TOGGLE_BIT(fun, BIT, value) \
JS(fun) { \
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc); \
	GET_PRIVATE(WiiController, wii); \
\
	*rval = JSVAL_FALSE; \
\
	if (wii->initialized) { \
		wii->update_state(); \
		cwiid_state state; \
		cwiid_get_state(wii->wiimote, &state); \
		*rval = BOOLEAN_TO_JSVAL(state.value & BIT); \
		if (argc == 1) { \
			JSBool newstate; \
			JS_ValueToBoolean(cx, argv[0], &newstate); \
			cwiid_set_rpt_mode( \
				wii->wiimote, \
				(newstate ? \
					state.value | BIT : \
					state.value & ~BIT \
				) \
			); \
		} \
	} \
	return JS_TRUE; \
}

#define PROP_ATTRIB_RO JSPROP_ENUMERATE | JSPROP_READONLY | JSPROP_PERMANENT | JSPROP_SHARED
#define PROP_ATTRIB_RW JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_SHARED

#define JS_PROP_OP(fun) \
JSBool fun(JSContext *cx, JSObject *obj, jsval id, jsval *vp)

#define JS_PROP_GET_NUM(fun, item_class, value) \
JS_PROP_OP(fun) { \
  func("JS_PROP_GET_NUM %s %p",__PRETTY_FUNCTION__, obj); \
  *vp = JSVAL_VOID; \
    GET_PRIVATE(item_class, item); \
\
  double value = double(item->value); \
  return JS_NewNumberValue(cx, value, vp); \
}

/////// Javascript WiiController
JS(js_wii_ctrl_constructor);

DECLARE_CLASS_GC("WiiController", js_wii_ctrl_class, js_wii_ctrl_constructor, js_ctrl_gc);

JS(js_wii_ctrl_open);
JS_FUNC_CALL(js_wii_ctrl_close, WiiController, close());
JS(js_wii_ctrl_actaccel);
JS(js_wii_ctrl_ir);
JS(js_wii_ctrl_actbutt);
JS(js_wii_ctrl_rumble);
JS(js_wii_ctrl_actleds);
JS_FUNC_CALL(js_wii_ctrl_dump, WiiController, print_state());

JSFunctionSpec js_wii_ctrl_methods[] = {
  {"open",           js_wii_ctrl_open,       1},
  {"close",          js_wii_ctrl_close,      0},
  {"toggle_accel",   js_wii_ctrl_actaccel,   0},
  {"toggle_ir",      js_wii_ctrl_ir,         0},
  {"toggle_buttons", js_wii_ctrl_actbutt,    0},
  {"toggle_rumble",  js_wii_ctrl_rumble,     0},
  {"toggle_led",     js_wii_ctrl_actleds,    1},
  {"dump",           js_wii_ctrl_dump,       0},

  {0}
};

//JS_PROP_GET_NUM(js_wii_ctrl_battery, WiiController, get_battery );
JS_PROP_OP(js_wii_ctrl_battery) {
	*vp = JSVAL_VOID;
    GET_PRIVATE(WiiController, wii);

	if (wii->initialized) {
		double value = wii->get_battery();
		return JS_NewNumberValue(cx, value, vp);
	} else {
		return JSVAL_FALSE;
	}
}

JS_PROP_GET_NUM(js_wii_ctrl_x, WiiController, x);
JS_PROP_GET_NUM(js_wii_ctrl_y, WiiController, y);
JS_PROP_GET_NUM(js_wii_ctrl_z, WiiController, z);

JSPropertySpec js_wii_ctrl_props[] = {
	{"battery",     0, PROP_ATTRIB_RO, js_wii_ctrl_battery, NULL},
	{"x",           1, PROP_ATTRIB_RO, js_wii_ctrl_x,       NULL},
	{"y",           2, PROP_ATTRIB_RO, js_wii_ctrl_y,       NULL},
	{"z",           3, PROP_ATTRIB_RO, js_wii_ctrl_z,       NULL},
	{0}
};

JS(js_wii_ctrl_constructor) {
  func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
  char excp_msg[MAX_ERR_MSG + 1];
  
  WiiController *wii = new WiiController();
  // initialize with javascript context
  if(! wii->init(global_environment) ) {
    sprintf(excp_msg, "failed initializing WiiMote controller");
    goto error;
  }

  // assign instance into javascript object
  if( ! JS_SetPrivate(cx, obj, (void*)wii) ) {
    sprintf(excp_msg, "failed assigning WiiMote controller to javascript");
    goto error;
  }

  // assign the real js object
  wii->jsobj = obj;
  wii->javascript = true;
  
  notice("WiiMote controller attached");

  *rval = OBJECT_TO_JSVAL(obj);
  return JS_TRUE;

 error:
  JS_ReportErrorNumber(cx, JSFreej_GetErrorMessage, NULL,
		       JSSMSG_FJ_CANT_CREATE, __func__, excp_msg);
  //  cx->newborn[GCX_OBJECT] = NULL;
  delete wii; return JS_FALSE;
}

JS(js_wii_ctrl_open) {
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

JS_FUNC_TOGGLE_BIT(js_wii_ctrl_actaccel, CWIID_RPT_ACC, rpt_mode)
JS_FUNC_TOGGLE_BIT(js_wii_ctrl_ir, CWIID_RPT_IR, rpt_mode)
JS_FUNC_TOGGLE_BIT(js_wii_ctrl_actbutt,  CWIID_RPT_BTN, rpt_mode)

JS(js_wii_ctrl_rumble) {
	GET_PRIVATE(WiiController, wii);

	*rval = JSVAL_FALSE;

	if (wii->initialized) {
		wii->update_state();
		cwiid_state state;
		cwiid_get_state(wii->wiimote, &state);
		*rval = BOOLEAN_TO_JSVAL(state.rumble);
		if (argc == 1) {
			JSBool newstate;
			JS_ValueToBoolean(cx, argv[0], &newstate);
			cwiid_set_rumble(wii->wiimote, newstate);
		}
	}
	return JS_TRUE;
}

//JS_NATIVE(js_wii_ctrl_dump) {
//	*rval = JSVAL_FALSE;
//	GET_PRIVATE(WiiController, wii);
//
//	cwiid_state state;
//	cwiid_get_state(wii->wiimote, &state);
//	wii->print_state();
//
//	return JS_TRUE;
//}

double WiiController::get_battery() {
	update_state();
	return (double)(100.0 * state.battery / CWIID_BATTERY_MAX);
}

JS(js_wii_ctrl_actleds) {
	func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
	*rval = JSVAL_FALSE;
	GET_PRIVATE(WiiController, wii);
	if (wii && wii->initialized) {
		wii->update_state();
		JS_NewNumberValue(cx, wii->state.led, rval);
		if (argc == 1) {
			uint16_t led_new;
			JS_ValueToUint16(cx, argv[0], &led_new);
			cwiid_set_led(wii->wiimote, led_new);
		}
	}
	return JS_TRUE;
}

void cwiid_callback(cwiid_wiimote_t *wii, int mesg_count,
                    union cwiid_mesg mesg[], struct timespec *timestamp) {
	WiiController* Wii = static_cast<WiiController *>((void*)(cwiid_get_data(wii)));

	for (int i=0; i < mesg_count; i++) {
		cwiid_mesg msg = mesg[i];
		if(mesg[i].type == CWIID_MESG_ACC) {
			Wii->accel(
				mesg[i].acc_mesg.acc[CWIID_X],
				mesg[i].acc_mesg.acc[CWIID_Y],
				mesg[i].acc_mesg.acc[CWIID_Z]
			);
		} else
		if(msg.type == CWIID_MESG_IR) {
			Wii->ir(&msg.ir_mesg);
		} else
		if(mesg[i].type == CWIID_MESG_BTN)
			Wii->button( mesg[i].btn_mesg.buttons );
		else
		if(mesg[i].type == CWIID_MESG_ERROR)
			Wii->error_event(mesg[i].error_mesg.error);
		else
			error("WII unh. message %i", mesg[i].type);
	}

}

void WiiController::error_event(cwiid_error err) {
  func("%s : %i", __PRETTY_FUNCTION__, err);

  initialized = false;
  cwiid_close(wiimote);
  JSCall("error", 1, "u", err);
}



void WiiController::accel(uint8_t wx, uint8_t wy, uint8_t wz) {
	nx = wx;
	ny = wy;
	nz = wz;
}

void WiiController::button(uint16_t buttons) {
	newbutt = buttons;
}

WiiController::WiiController()
:Controller() {

	wii_event_connect = false;
	wii_event_ir = false;
	wii_event_connect_err = false;
	initialized = false;
	newbutt = oldbutt = 0;

	set_name("WiiCtrl");
}

WiiController::~WiiController() {
	close();
}

bool WiiController::close() {
	//stop(); TODO: cancel thread
	if (initialized) {
		cwiid_close(wiimote);
	}
	initialized = false;
	return true;
}

#define WII_FLAGS CWIID_FLAG_MESG_IFC
// | CWIID_FLAG_NONBLOCK

bool WiiController::activate(bool state) {
	bool old = active;
	active = state;

	if (initialized) {
		if (active)
			cwiid_enable(wiimote, WII_FLAGS);
		else
			cwiid_disable(wiimote, WII_FLAGS);
	}

	return old;
}

bool WiiController::connect(char *hwaddr) {
  // if argument is NULL look for any wiimote
  if (!is_running()) {
    if(hwaddr == NULL) {
    const char *anyaddr = "00:00:00:00:00:00";
      str2ba(anyaddr, &bdaddr);
    }	else {
      str2ba(hwaddr,&bdaddr);
    }
		start();
  }
  if (initialized) {
    close();
  }
  return 1;
}

void WiiController::run() {
  
  notice("Detecting WiiMote (press A+B on it to handshake)");
  
  wiimote = cwiid_open(&bdaddr, WII_FLAGS);
  if(!wiimote) {
    error("unable to connect to WiiMote");
	wii_event_connect_err = true;
    return;
  } else
    act("WiiMote connected");
  
  cwiid_set_data(wiimote, (void*)this);
  if (cwiid_set_mesg_callback(wiimote, cwiid_callback)) {
    error("unable to set wiimote message callback");
    cwiid_close(wiimote);
	wii_event_connect_err = true;
    return;
  }

  // set filename to bdaddr
  //char addr_str[18];
  //ba2str(&bdaddr, addr_str);
  //set_filename(addr_str);
  
  wii_event_connect = true;
  initialized = true;
  activate(true);
}

int WiiController::poll() {
	return dispatch();
}

void WiiController::ir(cwiid_ir_mesg* msg) {
	ir_data = *msg;
	wii_event_ir = true;
}

int WiiController::dispatch() {
  if (is_running()) return 0; // connecting thread is running

  if (wii_event_ir) {
    for (int n = 0; n < CWIID_IR_SRC_COUNT; n++) {
      if (ir_data.src[n].valid) {
        JSCall("ir", 4, "iuui",
                n, ir_data.src[n].pos[CWIID_X], ir_data.src[n].pos[CWIID_Y],
                ir_data.src[n].size);
      }
    }
    wii_event_ir = false;
  }
	if (wii_event_connect) {
		JSCall("connect", 1, "b", 1);
		wii_event_connect = false;
	}
	if (wii_event_connect_err) {
    JSCall("error", 1, "u", CWIID_ERROR_COMM);
		wii_event_connect_err = false;
	}

	if( (nx ^ x) || (ny ^ y) || (nz ^ z) ) {
		x = nx; y = ny; z = nz;
		if (jsobj) {
      JSCall("acceleration", 3, "uuu", x, y, z);
		}
	}
// button(<int> button, <int> state, <int> mask, <int> old_mask)
    uint16_t butt_diff = newbutt ^ oldbutt;
	if (butt_diff) {
		for (uint16_t k = 1 << 15; k != 0; k = k >> 1 ) {
			if (k & butt_diff) {
				JSCall("button", 4, "ubuu",
                  k, ((k & newbutt) > 0), newbutt, oldbutt);
			}
		}
		oldbutt = newbutt;
	}
//	if (!res) {
//		error("deactivating %s [%s]", name, filename);
//		activate(false);
//	}
	return 1;
}

int WiiController::update_state() {
	cwiid_get_state(wiimote, &state);
	return 0;
}

int WiiController::print_state() {
	int i;
	int valid_source = 0;

	if (!initialized) {
		error("WII: not connected, no data to dump");
		return 0;
	}

	update_state();

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
	case CWIID_EXT_BALANCE:
    act("Balance: --");
		break;
	case CWIID_EXT_MOTIONPLUS:
    act("Motionplus: --");
		break;
	}
	return 1;
}

#endif
