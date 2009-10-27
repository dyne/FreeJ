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

#define WII_FLAGS CWIID_FLAG_MESG_IFC
// | CWIID_FLAG_NONBLOCK

/////// Javascript WiiController
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
      wii->open(addr);
    } else {
      wii->open();
    }

    return JS_TRUE;
}

JS(js_wii_ctrl_close) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  wii->close();

  return JS_TRUE;
}

JS(js_wii_ctrl_battery) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  double value = wii->battery();
  return JS_NewNumberValue(cx, value, rval);
}

JS(js_wii_ctrl_x) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  double value = wii->x();
  return JS_NewNumberValue(cx, value, rval);
}

JS(js_wii_ctrl_y) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  double value = wii->y();
  return JS_NewNumberValue(cx, value, rval);
}

JS(js_wii_ctrl_z) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  double value = wii->z();
  return JS_NewNumberValue(cx, value, rval);
}

JS(js_wii_ctrl_actaccel) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);

	*rval = JSVAL_FALSE;
  if (argc == 1) {
    JSBool state;
    JS_ValueToBoolean(cx, argv[0], &state);
    *rval = BOOLEAN_TO_JSVAL(wii->set_accel_report(state));
  } else {
    *rval = BOOLEAN_TO_JSVAL(wii->get_accel_report());
  }
	return JS_TRUE;
}

JS(js_wii_ctrl_ir) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);

	*rval = JSVAL_FALSE;
  if (argc == 1) {
    JSBool state;
    JS_ValueToBoolean(cx, argv[0], &state);
    *rval = BOOLEAN_TO_JSVAL(wii->set_ir_report(state));
  } else {
    *rval = BOOLEAN_TO_JSVAL(wii->get_ir_report());
  }
	return JS_TRUE;
}

JS(js_wii_ctrl_actbutt) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);

	*rval = JSVAL_FALSE;
  if (argc == 1) {
    JSBool state;
    JS_ValueToBoolean(cx, argv[0], &state);
    *rval = BOOLEAN_TO_JSVAL(wii->set_button_report(state));
  } else {
    *rval = BOOLEAN_TO_JSVAL(wii->get_button_report());
  }
	return JS_TRUE;
}

JS(js_wii_ctrl_rumble) {
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);

  if (argc == 1) {
    JSBool state;
    JS_ValueToBoolean(cx, argv[0], &state);
    *rval = BOOLEAN_TO_JSVAL(wii->set_rumble(state));
  } else {
    *rval = BOOLEAN_TO_JSVAL(wii->get_rumble());
  }

	return JS_TRUE;
}

JS(js_wii_ctrl_actleds) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  *rval = JSVAL_FALSE;
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);

  unsigned int led = 0;
  JSBool state = false;

  if (argc == 2) {
    JS_ValueToUint16(cx, argv[0], (uint16_t *)&led);
    JS_ValueToBoolean(cx, argv[1], &state);
    *rval = BOOLEAN_TO_JSVAL(wii->set_led(led, state));
  } else if (argc == 1) {
    JS_ValueToUint16(cx, argv[0], (uint16_t *)&led);
    *rval = BOOLEAN_TO_JSVAL(wii->get_led(led));
  }

  return JS_TRUE;
}

JS(js_wii_ctrl_dump) {
  func("%u:%s:%s argc: %u",__LINE__,__FILE__,__FUNCTION__, argc);
  WiiController *wii = (WiiController *)JS_GetPrivate(cx, obj);
  if(!wii) JS_ERROR("Wii core data is NULL");

  wii->dump();

  return JS_TRUE;
}

DECLARE_CLASS_GC("WiiController", js_wii_ctrl_class, js_wii_ctrl_constructor, js_ctrl_gc);

JSFunctionSpec js_wii_ctrl_methods[] = {
  {"open",           js_wii_ctrl_open,       1},
  {"close",          js_wii_ctrl_close,      0},
  {"battery",        js_wii_ctrl_battery,    0},
  {"x",              js_wii_ctrl_x,          0},
  {"y",              js_wii_ctrl_y,          0},
  {"z",              js_wii_ctrl_z,          0},
  {"toggle_accel",   js_wii_ctrl_actaccel,   0},
  {"toggle_ir",      js_wii_ctrl_ir,         0},
  {"toggle_buttons", js_wii_ctrl_actbutt,    0},
  {"toggle_rumble",  js_wii_ctrl_rumble,     0},
  {"toggle_led",     js_wii_ctrl_actleds,    2},
  {"dump",           js_wii_ctrl_dump,       0},

  {0}
};

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

WiiController::WiiController() :Controller() {

	_wii_event_connect = false;
	_wii_event_ir = false;
	_wii_event_connect_err = false;
	initialized = false;
	_newbutt = _oldbutt = 0;
  _x = _y = _z = 0;

	set_name("WiiCtrl");
}

WiiController::~WiiController() {
	close();
}

int WiiController::dispatch() {
  if (is_running()) return 0; // connecting thread is running

  if (_wii_event_ir) {
    for (int n = 0; n < CWIID_IR_SRC_COUNT; n++) {
      if (_ir_data.src[n].valid) {
        JSCall("ir", 4, "iuui",
                n, _ir_data.src[n].pos[CWIID_X], _ir_data.src[n].pos[CWIID_Y],
                _ir_data.src[n].size);
      }
    }
    _wii_event_ir = false;
  }
	if (_wii_event_connect) {
		JSCall("connect", 1, "b", 1);
		_wii_event_connect = false;
	}
	if (_wii_event_connect_err) {
    JSCall("error", 1, "u", CWIID_ERROR_COMM);
		_wii_event_connect_err = false;
	}

	if( (_nx ^ _x) || (_ny ^ _y) || (_nz ^ _z) ) {
		_x = _nx; _y = _ny; _z = _nz;
		if (jsobj) {
      JSCall("acceleration", 3, "uuu", _x, _y, _z);
		}
	}
// button(<int> button, <int> state, <int> mask, <int> old_mask)
    uint16_t butt_diff = _newbutt ^ _oldbutt;
	if (butt_diff) {
		for (uint16_t k = 1 << 15; k != 0; k = k >> 1 ) {
			if (k & butt_diff) {
				JSCall("button", 4, "ubuu",
                  k, ((k & _newbutt) > 0), _newbutt, _oldbutt);
			}
		}
		_oldbutt = _newbutt;
	}
//	if (!res) {
//		error("deactivating %s [%s]", name, filename);
//		activate(false);
//	}
	return 1;
}

int WiiController::poll() {
	return dispatch();
}

void WiiController::run() {
  
  notice("Detecting WiiMote (press A+B on it to handshake)");
  
  _wiimote = cwiid_open(&_bdaddr, WII_FLAGS);
  if(!_wiimote) {
    error("unable to connect to WiiMote");
	_wii_event_connect_err = true;
    return;
  } else
    act("WiiMote connected");
  
  cwiid_set_data(_wiimote, (void*)this);
  if (cwiid_set_mesg_callback(_wiimote, cwiid_callback)) {
    error("unable to set wiimote message callback");
    cwiid_close(_wiimote);
	_wii_event_connect_err = true;
    return;
  }

  _wii_event_connect = true;
  initialized = true;
  activate(true);
}

bool WiiController::open(const char *hwaddr) {
  if (initialized) {
    close();
  }
  if (!is_running()) {
    str2ba(hwaddr,&_bdaddr);
		start();
  }
  return 1;
}

bool WiiController::open() {
  // look for any wiimote
  const char *anyaddr = "00:00:00:00:00:00";
  return open(anyaddr);
}

bool WiiController::close() {
	//stop(); TODO: cancel thread
	if (initialized) {
		cwiid_close(_wiimote);
	}
	initialized = false;
	return true;
}

void WiiController::accel(uint8_t wx, uint8_t wy, uint8_t wz) {
	_nx = wx;
	_ny = wy;
	_nz = wz;
}

bool WiiController::get_accel_report() {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  return (wiistate.rpt_mode & CWIID_RPT_ACC);
}

bool WiiController::set_accel_report(bool state) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  bool oldstate = (wiistate.rpt_mode & CWIID_RPT_ACC);
  if (state) {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode | CWIID_RPT_ACC);
  } else {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode & ~CWIID_RPT_ACC);
  }
  return oldstate;
}

void WiiController::ir(cwiid_ir_mesg* msg) {
	_ir_data = *msg;
	_wii_event_ir = true;
}

bool WiiController::get_ir_report() {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  return (wiistate.rpt_mode & CWIID_RPT_IR);
}

bool WiiController::set_ir_report(bool state) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  bool oldstate = (wiistate.rpt_mode & CWIID_RPT_IR);
  if (state) {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode | CWIID_RPT_IR);
  } else {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode & ~CWIID_RPT_IR);
  }
  return oldstate;
}

void WiiController::button(uint16_t buttons) {
	_newbutt = buttons;
}

bool WiiController::get_button_report() {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  return (wiistate.rpt_mode & CWIID_RPT_BTN);
}

bool WiiController::set_button_report(bool state) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  bool oldstate = (wiistate.rpt_mode & CWIID_RPT_BTN);
  if (state) {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode | CWIID_RPT_BTN);
  } else {
    cwiid_set_rpt_mode(_wiimote, wiistate.rpt_mode & ~CWIID_RPT_BTN);
  }
  return oldstate;
}

void WiiController::error_event(cwiid_error err) {
  func("%s : %i", __PRETTY_FUNCTION__, err);

  initialized = false;
  cwiid_close(_wiimote);
  JSCall("error", 1, "u", err);
}

bool WiiController::activate(bool state) {
	bool old = active;
	active = state;

	if (initialized) {
		if (active)
			cwiid_enable(_wiimote, WII_FLAGS);
		else
			cwiid_disable(_wiimote, WII_FLAGS);
	}

	return old;
}

bool WiiController::get_rumble() {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  return wiistate.rumble;
}

bool WiiController::set_rumble(bool state) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  bool oldstate = wiistate.rumble;
  cwiid_set_rumble(_wiimote, state);
  return oldstate;
}

bool WiiController::get_led(unsigned int led) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  switch(led) {
    case 1:
      return (wiistate.led & CWIID_LED1_ON);
    case 2:
      return (wiistate.led & CWIID_LED2_ON);
    case 3:
      return (wiistate.led & CWIID_LED3_ON);
    case 4:
      return (wiistate.led & CWIID_LED4_ON);
    default:
      error("%s led %d outside range (1-4)", __PRETTY_FUNCTION__, led);
      return false;
  }
}

bool WiiController::set_led(unsigned int led, bool state) {
	if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return false;
  }
  update_state();
  cwiid_state wiistate;
  cwiid_get_state(_wiimote, &wiistate);
  uint16_t new_led = wiistate.led;
  bool old_state = false;
  switch(led) {
    case 1:
      old_state = wiistate.led & CWIID_LED1_ON;
      new_led = (state ? (wiistate.led | CWIID_LED1_ON) :
                          (wiistate.led & ~CWIID_LED1_ON));
      break;
    case 2:
      old_state = wiistate.led & CWIID_LED2_ON;
      new_led = (state ? (wiistate.led | CWIID_LED2_ON) :
                          (wiistate.led & ~CWIID_LED2_ON));
      break;
    case 3:
      old_state = wiistate.led & CWIID_LED3_ON;
      new_led = (state ? (wiistate.led | CWIID_LED3_ON) :
                          (wiistate.led & ~CWIID_LED3_ON));
      break;
    case 4:
      old_state = wiistate.led & CWIID_LED4_ON;
      new_led = (state ? (wiistate.led | CWIID_LED4_ON) :
                          (wiistate.led & ~CWIID_LED4_ON));
      break;
    default:
      error("%s led %d outside range (1-4)", __PRETTY_FUNCTION__, led);
  }
  cwiid_set_led(_wiimote, new_led);
  return old_state;
}

double WiiController::battery() {
  if (!initialized) {
    error("%s controller not initialized", __PRETTY_FUNCTION__);
    return 0.0;
  } else {
    update_state();
    return (double)(100.0 * _state.battery / CWIID_BATTERY_MAX);
  }
}

int WiiController::update_state() {
	cwiid_get_state(_wiimote, &_state);
	return 0;
}

int WiiController::dump() {
	int i;
	int valid_source = 0;

	if (!initialized) {
		error("WII: not connected, no data to dump");
		return 0;
	}

	update_state();

	act("Report Mode:");
	if (_state.rpt_mode & CWIID_RPT_STATUS) act(" STATUS");
	if (_state.rpt_mode & CWIID_RPT_BTN) act(" BTN");
	if (_state.rpt_mode & CWIID_RPT_ACC) act(" ACC");
	if (_state.rpt_mode & CWIID_RPT_IR) act(" IR");
	if (_state.rpt_mode & CWIID_RPT_NUNCHUK) act(" NUNCHUK");
	if (_state.rpt_mode & CWIID_RPT_CLASSIC) act(" CLASSIC");
	
	act("Active LEDs:");
	if (_state.led & CWIID_LED1_ON) act(" 1");
	if (_state.led & CWIID_LED2_ON) act(" 2");
	if (_state.led & CWIID_LED3_ON) act(" 3");
	if (_state.led & CWIID_LED4_ON) act(" 4");

	act("Rumble: %s", _state.rumble ? "On" : "Off");

	act("Battery: %d%%",
	       (int)(100.0 * _state.battery / CWIID_BATTERY_MAX));

	act("Buttons: %X", _state.buttons);

	act("Acc: x=%d y=%d z=%d", _state.acc[CWIID_X], _state.acc[CWIID_Y],
	       _state.acc[CWIID_Z]);

	act("IR: ");
	for (i = 0; i < CWIID_IR_SRC_COUNT; i++) {
		if (_state.ir_src[i].valid) {
			valid_source = 1;
			act("(%d,%d) ", _state.ir_src[i].pos[CWIID_X],
			                   _state.ir_src[i].pos[CWIID_Y]);
		}
	}
	if (!valid_source) {
		act("no sources detected");
	}

	switch (_state.ext_type) {
	case CWIID_EXT_NONE:
		act("No extension");
		break;
	case CWIID_EXT_UNKNOWN:
		act("Unknown extension attached");
		break;
	case CWIID_EXT_NUNCHUK:
		act("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
		       "acc.z=%d", _state.ext.nunchuk.buttons,
		       _state.ext.nunchuk.stick[CWIID_X],
		       _state.ext.nunchuk.stick[CWIID_Y],
		       _state.ext.nunchuk.acc[CWIID_X],
		       _state.ext.nunchuk.acc[CWIID_Y],
		       _state.ext.nunchuk.acc[CWIID_Z]);
		break;
	case CWIID_EXT_CLASSIC:
		act("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
		       "l=%d r=%d", _state.ext.classic.buttons,
		       _state.ext.classic.l_stick[CWIID_X],
		       _state.ext.classic.l_stick[CWIID_Y],
		       _state.ext.classic.r_stick[CWIID_X],
		       _state.ext.classic.r_stick[CWIID_Y],
		       _state.ext.classic.l, _state.ext.classic.r);
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
