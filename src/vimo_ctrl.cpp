/*  FreeJ
 *  (c) Copyright 2006 Denis Roio aka jaromil <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 3 of the License,
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
 * serial VideoMouse Controller by MrGoil (c) 2008
 *
 */


#include <callbacks_js.h>
#include <jsparser_data.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/file.h>

#include <vimo_ctrl.h>

ViMoController::ViMoController() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	initialized = active = false;
	jsenv = NULL;
	jsobj = NULL;
	filename = NULL;
	fd = 0;
	set_name("Video Mouse");
	// NULL setting: k: 00 wi: 03 wo: 0f 
	vmd_old.w = vmd.w = 0x03fc0000;
}

ViMoController::~ViMoController() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	rem();
	close();
	if (jsobj)
		JS_SetPrivate(jsenv, jsobj, NULL);
	jsobj = NULL;
	if (filename)
		free(filename);
}

JS(js_vimo_ctrl_constructor);
DECLARE_CLASS_GC("ViMoController", js_vimo_ctrl_class, NULL, js_ctrl_gc);

JSFunctionSpec js_vimo_ctrl_methods[] = { 
	{"open",	js_vimo_open,	0},
	{"close",	js_vimo_close,	0},
	{0} 
};

bool ViMoController::init(JSContext *env, JSObject *obj) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);
	jsenv = env;
	jsobj = obj;

	initialized = true;
	return(true);
}

static const char magic[] = {0x02, 0x0a, 0x0c, 0x0a};

bool ViMoController::open() {
//	int curr_fl;
    struct termios options;
	struct stat filestat;

	if (!filename)
		return 0;
	if (fd)
		return 0;

	read_pos = 0;

	if (stat(filename, &filestat) == -1)
		goto error;

	if (!S_ISCHR(filestat.st_mode)) {
		error("%s is not a character device", filename);
		return 0;
	}

	fd = ::open(filename, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (fd == -1)
		goto error;

	if ( ::flock(fd, LOCK_EX | LOCK_NB) == -1)
		goto error_close;

	// set block
//	{
//	int curr_fl = fcntl(fd, F_GETFL);
//	if (curr_fl == -1)
//		goto error_close;
//	if (fcntl(fd, F_SETFL, curr_fl & ~O_NONBLOCK) == -1)
//		goto error_close;
//	}

	// set up serial port
    if (tcgetattr(fd, &options) == -1)
		goto error_close;

	cfmakeraw(&options);		// sets some defaults
	options.c_cc[VMIN] = 5;		// min chars/read complete
	options.c_cc[VTIME] = 10;	// read timeout
	options.c_cc[VSTART] = 0;	//
	options.c_cc[VSTOP] = 0; 	//
		
	cfsetspeed(&options, B19200);	// Set 19200 baud    
	options.c_cflag |= CS8;			// set 8bit
	options.c_cflag &= ~CRTSCTS;	// no flow
//	options.c_cflag |= CRTSCTS;	// flow
	if (tcsetattr(fd, TCSANOW, &options) == -1)
		goto error_close;

	// init Mouse
	tcflush(fd, TCIOFLUSH);
	if (write(fd, magic, 4)==-1) { // answer: 0xc0 0x01 0xc1
		error("%s sending magic failed", __PRETTY_FUNCTION__);
		goto error_close;
	}
	if (tcdrain(fd) == -1) {
		error("%s drain failed", __PRETTY_FUNCTION__);
		goto error_close;
	}

	// hope, all ok
	return 1;

error_close:
	::close(fd);
error:
	error("%s: %s %i %s", __PRETTY_FUNCTION__, filename, errno, strerror(errno));
	return 0;
}

bool ViMoController::open(char* newname) {
	if (fd)
		return 0;
	if (filename)
		free(filename);
	filename = strndup(newname, MAX_ERR_MSG);
	return open();
}

void ViMoController::close() {
	//stop();
	if (fd)
		::close(fd);
	fd = 0;
}

static const int o_wheel_speed[] = {-5, 5, -6, 6, -4, 4, -7, 7, -2, 2, -1, 1, -3, 3, 0, 0 };
unsigned int wi_hist=0; // wheel history

int ViMoController::dispatch() {
	//char *data = vmd.data;
	//func("dis: %02x %02x %02x", data[1], data[2], data[3]);
	vmd.bits.k = ~vmd.bits.k;

	// button.(button, state, mask, mask_old)
	char key_diff = vmd.bits.k ^ vmd_old.bits.k;
	if (key_diff) {
		for (char k = 1 << 7 ; k != 0; k = k >> 1) {
			if (k & key_diff) {
				JSCall("button", 4, "ubuu", k, (k&vmd.bits.k), vmd.bits.k, vmd_old.bits.k);
			}
		}
	}

	// inner wheel
	// .wi_lock(pos, speed) pos: <= 2 0 1 /3/ 2 0 1 =>, left:-1, right:1
	// .wi_fine(dir, speed) dir: 0< 1> speed 1,2,3
	char wi_diff = vmd.bits.i ^ vmd_old.bits.i;
	if (wi_diff) {
		wi_hist = (wi_hist << 4) | vmd.bits.i;
		char mv = vmd_old.bits.i | (vmd.bits.i << 4);
		if (vmd.bits.i==3) {
			func("wi: %02x mv: %02x %08x", wi_diff, mv, wi_hist);
			JSCall("wheel_i", 3, "uuu", vmd.bits.i, vmd_old.bits.i, wi_hist);
//double test = 99.345677;
//JSCall("wheel_i", 4, "diii", &test, (int*)&vmd.bits.i, (int*)&vmd_old.bits.i, &hist);
//JSCall("wheel_i", 3, "dic", test,  hist, hist);
		}
	}

	// outer wheel
	// .wo(speed, speed_old)
	char wo_diff = vmd.bits.o ^ vmd_old.bits.o;
	if (wo_diff) {
		int s = o_wheel_speed[vmd.bits.o];
		int so = o_wheel_speed[vmd_old.bits.o];
		func("wo: %02x -> speed: %i old: %i", wo_diff, s, so);
		JSCall("wheel_o", 2, "ii", s, so);
	}

	vmd_old.w = vmd.w;
	return 0;
}

int ViMoController::poll() {
	if (!initialized)
		return 0;
	if (!fd) // not open
		return 0;

	// []     0  1  2  3
	// pos    1  2  3  4
	// aa 07 03 3c fe 44 aa ...
	// we get one 0xaa every second
	// and 5 bytes data event:
	// 0,1: 0x07 0x03 burst
	// 2,3: wheel / keys
	// 4: crc (?!)
	char ch;
	char *data = vmd.data;
	int n = read(fd, &ch, 1);
	while (n > 0) {
		if (read_pos==0) {
			if (ch == 0xaa) { // empty sync 1/s
				n = read(fd, &ch, 1);
				continue;
			}
			// packet starts with 0x07 0x03
			if (ch == 0x07) {
				read_pos = 1;
			}
		} else {
			data[read_pos - 1] = ch;
			if (read_pos == sizeof(vmd)) { // packet complete
				read_pos = 0;
				if (data[0] == 0x03) {
					dispatch();
				} else {
					func("%s invalid data packet (%s): %08x",
						__PRETTY_FUNCTION__, filename, vmd.w
					);
				}
			} else {
				read_pos++;
			}
		}
		n = read(fd, &ch, 1);
	}

	if (n == -1) {
		if (errno == EAGAIN) { // no data, nothing to do
			return 0;
		}
		error("%s: %i %s", __PRETTY_FUNCTION__, errno, strerror(errno));
		activate(false);
		return 0;
	}
	return 0;
}

bool ViMoController::activate(bool state) {
	// also called on register_controller
	// flush data
	if (fd)
		tcflush(fd, TCIOFLUSH);
	return Controller::activate(state);
}

JS(js_vimo_open) {
}

JS(js_vimo_close) {
}

JS(js_vimo_ctrl_constructor) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

	ViMoController *mouse = new ViMoController();

	// initialize with javascript context
	if(! mouse->init(cx, obj) ) {
		error("failed initializing mouse controller");
		delete mouse; return JS_FALSE;
	}
	if (argc == 1) {
		char *filename;
		JS_ARG_STRING(filename, 0);
		if(!mouse->open(filename)) {
			error("failed initializing mouse controller");
			delete mouse; return JS_FALSE;
		}
	}

	// assign instance into javascript object
	if( ! JS_SetPrivate(cx, obj, (void*)mouse) ) {
		error("failed assigning mouse controller to javascript");
		delete mouse; return JS_FALSE;
	}

	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
