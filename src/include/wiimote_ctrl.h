/*  FreeJ WiiMote controller
 *  (c) Copyright 2008 Denis Rojo <jaromil@dyne.org>
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

#ifndef __WII_CTRL_H__
#define __WII_CTRL_H__

#include <config.h>
#ifdef WITH_CWIID

#include <controller.h>
#include <closure.h>

extern "C" {
#include <cwiid.h>
}

class WiiController: public Controller {

 public:

   // see cwiid_error
   enum WiiError {
     ERROR_NONE,
     ERROR_DISCONNECT,
     ERROR_COMM,
   };

  WiiController();
  virtual ~WiiController();

  int dispatch();
  int poll();

  bool open();
  bool open(const char *hwaddr);
  bool close();

  bool activate(bool state);

  virtual void connect_event();
  virtual void disconnect_event();
  virtual void error_event(WiiError err);

  virtual void accel_event(double x, double y, double z);
  bool get_accel_report() { return _get_report(CWIID_RPT_ACC); }
  bool set_accel_report(bool state) {
    return _set_report(state, CWIID_RPT_ACC);
  }

  virtual void ir_event(unsigned int source, unsigned int x, unsigned int y,
                        unsigned int size);
  bool get_ir_report() { return _get_report(CWIID_RPT_IR); }
  bool set_ir_report(bool state) {
    return _set_report(state, CWIID_RPT_IR);
  }

  virtual void button_event(unsigned int button, bool state,
                            unsigned int mask, unsigned int old_mask);
  bool get_button_report() { return _get_report(CWIID_RPT_BTN); }
  bool set_button_report(bool state) {
    return _set_report(state, CWIID_RPT_BTN);
  }

  bool get_rumble();
  bool set_rumble(bool state);

  bool get_led(unsigned int led);
  bool set_led(unsigned int led, bool state);

  double battery();
  double x() { return _x;}
  double y() { return _y;}
  double z() { return _z;}

  // used by cwiid callback to update controller state:
  void cwiid_update_acc(double x, double y, double z);
  void cwiid_update_ir(unsigned int source, unsigned int x, unsigned int y,
                       unsigned int size);
  void cwiid_update_btn(uint16_t buttons);
  void cwiid_update_err(WiiError err);

  int dump(); // debug

 private:

  ThreadedClosureQueue *_opener; // queue blocking open connections
  ClosureQueue *_events_queue; // events handled at dispatch() time

  void _open_device(char *hwaddr); // blocking open
  void _post_open_device(cwiid_wiimote_t *dev); // synchronous setup,
                                                // after blocking open

  // these process and update the status in a synchronized way:
  void _cwiid_sync_update_acc(double x, double y, double z);
  void _cwiid_sync_update_ir(unsigned int source, unsigned int x,
                             unsigned int y, unsigned int size);
  void _cwiid_sync_update_btn(uint16_t buttons);
  void _cwiid_sync_update_err(WiiError err);

  bool _set_report(bool state, unsigned int type);
  bool _get_report(unsigned int type);

  double  _x,  _y,  _z;

  uint16_t _buttons;

  // TODO: nunchuk_state and classic_state extensions

  cwiid_wiimote_t  *_device;
  struct acc_cal _calib;

};

#endif // WITH_CWIID

#endif
