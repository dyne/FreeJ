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
//| CWIID_FLAG_NONBLOCK

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

void wiicontroller_cwiid_callback(cwiid_wiimote_t *dev, int count,
                                  union cwiid_mesg msgs[],
                                  struct timespec *timestamp) {
  WiiController* wii = (WiiController *)cwiid_get_data(dev);

  for (int i=0; i < count; i++) {
    cwiid_mesg msg = msgs[i];

    switch(msg.type) {
      case CWIID_MESG_ACC:
        wii->cwiid_update_acc((double)msg.acc_mesg.acc[CWIID_X],
                              (double)msg.acc_mesg.acc[CWIID_Y],
                              (double)msg.acc_mesg.acc[CWIID_Z]);
        break;
      case CWIID_MESG_IR:
        for (int n = 0; n < CWIID_IR_SRC_COUNT; n++) {
          if (msg.ir_mesg.src[n].valid) {
            wii->cwiid_update_ir(n, msg.ir_mesg.src[n].pos[CWIID_X],
                                 msg.ir_mesg.src[n].pos[CWIID_Y],
                                 msg.ir_mesg.src[n].size);
          }
        }
        break;
      case CWIID_MESG_BTN:
        wii->cwiid_update_btn(msg.btn_mesg.buttons);
        break;
      case CWIID_MESG_ERROR:
        wii->cwiid_update_err((WiiController::WiiError)msg.error_mesg.error);
        break;
      default:
        error("%s unhandled message type %i", __PRETTY_FUNCTION__, msg.type);
    }
  }
}

WiiController::WiiController() {

  _opener = new ThreadedClosureQueue();
  _events_queue = new ClosureQueue();

  _device = NULL;

  _buttons = 0;
  _x = _y = _z = 0;

  set_name("WiiCtrl");
}

WiiController::~WiiController() {
	close();
  delete _opener;
  delete _events_queue;
}

int WiiController::dispatch() {
  _events_queue->do_jobs();
  return 1;
}

int WiiController::poll() {
	return dispatch();
}

bool WiiController::_get_report(unsigned int type) {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
  return (wiistate.rpt_mode & type);
}

bool WiiController::_set_report(bool state, unsigned int type) {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
  bool oldstate = (wiistate.rpt_mode & type);
  if (state) {
    cwiid_set_rpt_mode(_device, wiistate.rpt_mode | type);
  } else {
    cwiid_set_rpt_mode(_device, wiistate.rpt_mode & ~type);
  }
  return oldstate;
}

void WiiController::_post_open_device(cwiid_wiimote_t *dev) {
  // set device
  if (_device) close();
  _device = dev;

  // calibration
  if (cwiid_get_acc_cal(_device, CWIID_EXT_NONE, &_calib)) {
    error("unable to get WiiMote calibration data");
    goto error_state;
  }

  // callback settings
  if (cwiid_set_data(dev, (void*)this)) {
    error("unable to set WiiMote private data");
    goto error_state;
  }
  if (cwiid_set_mesg_callback(dev, wiicontroller_cwiid_callback)) {
    error("unable to set WiiMote message callback");
    goto error_state;
  }

  act("WiiMote connected");
  connect_event();
  return;

error_state:
  close();
  error_event(ERROR_COMM);
}

void WiiController::_open_device(char *hwaddr) {
  
  bdaddr_t bdaddr;
  cwiid_wiimote_t *dev;

  str2ba(hwaddr,&bdaddr);
  free(hwaddr);

  notice("Detecting WiiMote (press 1+2 on it to handshake)");

  dev = cwiid_open(&bdaddr, WII_FLAGS);
  if(!dev) {
    error("unable to connect to WiiMote");
    _events_queue->add_job(NewClosure(this, &WiiController::error_event,
                                      ERROR_COMM));
    return;
  }

  _events_queue->add_job(NewClosure(this, &WiiController::_post_open_device,
                                    dev));
}

bool WiiController::open(const char *hwaddr) {
  char *tmp_hwaddr = strndup(hwaddr, 17); // len(hwaddr) = 17
  _opener->add_job(NewClosure(this, &WiiController::_open_device, tmp_hwaddr));
  return true;
}

bool WiiController::open() {
  // look for any wiimote
  const char *anyaddr = "00:00:00:00:00:00";
  return open(anyaddr);
}

bool WiiController::close() {
  if (_device) {
    cwiid_close(_device);
    _device = NULL;
    return true;
  } else {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
}

bool WiiController::activate(bool state) {

	if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }

  if (state) {
    return (cwiid_enable(_device, WII_FLAGS) == 0);
  } else {
    return (cwiid_disable(_device, WII_FLAGS) == 0);
  }

}

void WiiController::connect_event() {
  JSCall("connect", 1, "b", 1);
}

void WiiController::disconnect_event() {
  JSCall("disconnect", 1, "b", 1);
}

void WiiController::error_event(WiiError err) {
  JSCall("error", 1, "u", err);
}

void WiiController::accel_event(double x, double y, double z) {
  JSCall("acceleration", 3, "ddd", x, y, z);
}

void WiiController::ir_event(unsigned int source, unsigned int x,
                             unsigned int y, unsigned int size) {
  JSCall("ir", 4, "iuui", source, x, y, size);
}

void WiiController::button_event(unsigned int button, bool state,
                                 unsigned int mask, unsigned int old_mask) {
  JSCall("button", 4, "ubuu", button, state, mask, old_mask);
}

bool WiiController::get_rumble() {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
  return wiistate.rumble;
}

bool WiiController::set_rumble(bool state) {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
  bool oldstate = wiistate.rumble;
  cwiid_set_rumble(_device, state);
  return oldstate;
}

bool WiiController::get_led(unsigned int led) {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
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
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return false;
  }
  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);
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
  cwiid_set_led(_device, new_led);
  return old_state;
}

double WiiController::battery() {
  if (!_device) {
    error("%s controller not connected", __PRETTY_FUNCTION__);
    return 0.0;
  } else {
    cwiid_state wiistate;
    cwiid_get_state(_device, &wiistate);
    return (double)(100.0 * wiistate.battery / CWIID_BATTERY_MAX);
  }
}

void WiiController::cwiid_update_acc(double x, double y, double z) {
  _events_queue->add_job(NewClosure(this,
                                    &WiiController::_cwiid_sync_update_acc,
                                    x, y, z));
}

void WiiController::_cwiid_sync_update_acc(double x, double y, double z) {
  double new_x, new_y, new_z;

  // TODO(shammash):
  //  - add rotation support: roll, pitch, yaw
  //    (wminput/plugins/acc/acc.c:process_acc())
  //  - consider using thresholds
  new_x = (x - _calib.zero[CWIID_X]) /
          (_calib.one[CWIID_X] - _calib.zero[CWIID_X]);
  new_y = (y - _calib.zero[CWIID_Y]) /
          (_calib.one[CWIID_Y] - _calib.zero[CWIID_Y]);
  new_z = (z - _calib.zero[CWIID_Z]) /
          (_calib.one[CWIID_Z] - _calib.zero[CWIID_Z]);
  if( (new_x != _x) || (new_y != _y) || (new_z != _z) ) {
    _x = new_x; _y = new_y; _z = new_z;
    accel_event(_x, _y, _z);
  }
}

void WiiController::cwiid_update_ir(unsigned int source, unsigned int x,
                                    unsigned int y, unsigned int size) {
  _events_queue->add_job(NewClosure(this,
                                    &WiiController::_cwiid_sync_update_ir,
                                    source, x, y, size));
}

void WiiController::_cwiid_sync_update_ir(unsigned int source, unsigned int x,
                                     unsigned int y, unsigned int size) {
  // TODO(shammash): consider using IR to calc yaw
  ir_event(source, x, y, size);
}

void WiiController::cwiid_update_btn(uint16_t buttons) {
  _events_queue->add_job(NewClosure(this,
                                    &WiiController::_cwiid_sync_update_btn,
                                    buttons));
}

void WiiController::_cwiid_sync_update_btn(uint16_t buttons) {
  uint16_t butt_diff = buttons ^ _buttons;
  if (butt_diff) {
    for (uint16_t k = 1 << 15; k != 0; k = k >> 1 ) {
      if (k & butt_diff) {
        button_event(k, ((k & buttons) > 0), buttons, _buttons);
      }
    }
    _buttons = buttons;
  }
}

void WiiController::cwiid_update_err(WiiError err) {
  _events_queue->add_job(NewClosure(this,
                                    &WiiController::_cwiid_sync_update_err,
                                    err));
}

void WiiController::_cwiid_sync_update_err(WiiError err) {
  if (err == ERROR_DISCONNECT) {
    disconnect_event();
  } else {
    error_event(err);
  }
}

int WiiController::dump() {
	int i;
	int valid_source = 0;

	if (!_device) {
		error("WII: not connected, no data to dump");
		return 0;
	}

  cwiid_state wiistate;
  cwiid_get_state(_device, &wiistate);

	act("Report Mode:");
	if (wiistate.rpt_mode & CWIID_RPT_STATUS) act(" STATUS");
	if (wiistate.rpt_mode & CWIID_RPT_BTN) act(" BTN");
	if (wiistate.rpt_mode & CWIID_RPT_ACC) act(" ACC");
	if (wiistate.rpt_mode & CWIID_RPT_IR) act(" IR");
	if (wiistate.rpt_mode & CWIID_RPT_NUNCHUK) act(" NUNCHUK");
	if (wiistate.rpt_mode & CWIID_RPT_CLASSIC) act(" CLASSIC");
	
	act("Active LEDs:");
	if (wiistate.led & CWIID_LED1_ON) act(" 1");
	if (wiistate.led & CWIID_LED2_ON) act(" 2");
	if (wiistate.led & CWIID_LED3_ON) act(" 3");
	if (wiistate.led & CWIID_LED4_ON) act(" 4");

	act("Rumble: %s", wiistate.rumble ? "On" : "Off");

	act("Battery: %d%%",
	       (int)(100.0 * wiistate.battery / CWIID_BATTERY_MAX));

	act("Buttons: %X", wiistate.buttons);

	act("Acc: x=%d y=%d z=%d", wiistate.acc[CWIID_X], wiistate.acc[CWIID_Y],
	       wiistate.acc[CWIID_Z]);

	act("IR: ");
	for (i = 0; i < CWIID_IR_SRC_COUNT; i++) {
		if (wiistate.ir_src[i].valid) {
			valid_source = 1;
			act("(%d,%d) ", wiistate.ir_src[i].pos[CWIID_X],
			                   wiistate.ir_src[i].pos[CWIID_Y]);
		}
	}
	if (!valid_source) {
		act("no sources detected");
	}

	switch (wiistate.ext_type) {
	case CWIID_EXT_NONE:
		act("No extension");
		break;
	case CWIID_EXT_UNKNOWN:
		act("Unknown extension attached");
		break;
	case CWIID_EXT_NUNCHUK:
		act("Nunchuk: btns=%.2X stick=(%d,%d) acc.x=%d acc.y=%d "
		       "acc.z=%d", wiistate.ext.nunchuk.buttons,
		       wiistate.ext.nunchuk.stick[CWIID_X],
		       wiistate.ext.nunchuk.stick[CWIID_Y],
		       wiistate.ext.nunchuk.acc[CWIID_X],
		       wiistate.ext.nunchuk.acc[CWIID_Y],
		       wiistate.ext.nunchuk.acc[CWIID_Z]);
		break;
	case CWIID_EXT_CLASSIC:
		act("Classic: btns=%.4X l_stick=(%d,%d) r_stick=(%d,%d) "
		       "l=%d r=%d", wiistate.ext.classic.buttons,
		       wiistate.ext.classic.l_stick[CWIID_X],
		       wiistate.ext.classic.l_stick[CWIID_Y],
		       wiistate.ext.classic.r_stick[CWIID_X],
		       wiistate.ext.classic.r_stick[CWIID_Y],
		       wiistate.ext.classic.l, wiistate.ext.classic.r);
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
