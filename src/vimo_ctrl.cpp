/*  FreeJ - serial Video Mouse jogger controller
 *
 *  (c) Copyright 2008 Christoph Rudorff <goil@dyne.org>
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
 *
 */


#include <callbacks_js.h>
#include <jsparser_data.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/file.h>
#if defined (HAVE_DARWIN) || defined (HAVE_FREEBSD)
#include <sys/stat.h>
#endif

#include <vimo_ctrl.h>

/* buttons:
btn_rev, 1,
btn_rec, 2,
btn_fwd, 4,
btn_pause, 8,
btn_stop, 16,
btn_play, 32,
btn_esc,  64,
btn_player,   1|16,
btn_recorder, 4|16
*/
// data packet: ... 0xaa (0x07 0x03 ww kk cc) 0xaa ...
union ViMoData {
#ifdef WORDS_BIGENDIAN
	struct {
		unsigned
			h :8,	// header 0x03

			i :2,	// inner wheel
			o :4,	// outer whell
			  :2,	// pad

			k :7,	// button key
			  :1,	// pad

			c :8; 	// crc (?)
	} bits;
#else
	struct {
		unsigned
			h :8,

			  :2,
			o :4,
			i :2,

			  :1,
			k :7,

			c :8;
	} bits;
#endif
	unsigned char data[4];
	unsigned int w;
};
// right: 32013 32 20 01 13 // 20 01 32 13
// left:  31023 31 10 02 23 // 10 31 02 23
//static const unsigned char wi_right[] = { 2, 0, 3, 1 };
//static const unsigned char wi_left[]  = { 1, 3, 0, 2 };
static const unsigned int wi_right[] = { 0x1320, 0x3201, 0x0132, 0x2013 };
static const unsigned int wi_left[]  = { 0x2310, 0x0231, 0x3102, 0x1023 };
static const int o_wheel_speed[] = {-5, 5, -6, 6, -4, 4, -7, 7, -2, 2, -1, 1, -3, 3, 0, 0 };
static const unsigned char magic[] = {0x02, 0x0a, 0x0c, 0x0a};

ViMoController::ViMoController() 
  :Controller() {
	func("%s this=%p",__PRETTY_FUNCTION__, this);
	initialized = active = false;
	jsenv = NULL;
	jsobj = NULL;
	filename = NULL;
	fd = 0;
	set_name("Video Mouse");
	vmd = (ViMoData*)malloc(2*sizeof(ViMoData));
	vmd_old = vmd+1;
	// NULL setting: k: 00 wi: 03 wo: 0f
#ifdef WORDS_BIGENDIAN
	vmd_old->w = vmd->w = 0x03fc0000;
#else
	vmd_old->w = vmd->w = 0x0000fc03;
#endif
	wi_hist = 0;
	wi_dir = 0;
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
	free(vmd);
}

JS(js_vimo_ctrl_constructor);
DECLARE_CLASS_GC("ViMoController", js_vimo_ctrl_class, NULL, js_ctrl_gc);

JSFunctionSpec js_vimo_ctrl_methods[] = {
	{"open",	js_vimo_open,	0},
	{"close",	js_vimo_close,	0},
	{0}
};



bool ViMoController::open() {
	struct termios options;
	struct stat filestat;

	if (!filename) {
		error("%s: no filename!", __PRETTY_FUNCTION__);
		return 0;
	}
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
	filename = strdup(newname);
	return open();
}

void ViMoController::close() {
	if (fd)
		::close(fd);
	fd = 0;
}

void ViMoController::button(unsigned int button, bool state, unsigned int mask, unsigned int old_mask) {
	JSCall("button", 4, "ubuu", button, state, mask, old_mask);
}

void ViMoController::inner_wheel(int direction, unsigned int history) {
	JSCall("wheel_i", 2, "iu", direction, history);
}

void ViMoController::outer_wheel(int speed, int old_speed) {
	JSCall("wheel_o", 2, "ii", speed, old_speed);
}

int ViMoController::dispatch() {
	//char *data = vmd->data;
	//func("dis: %02x %02x %02x", data[1], data[2], data[3]);

	vmd->bits.k = ~vmd->bits.k; // button keys are inverted

	// button.(button, state, mask, mask_old)
	unsigned char key_diff = vmd->bits.k ^ vmd_old->bits.k;
	if (key_diff) {
		for (unsigned char k = 1 << 7 ; k != 0; k = k >> 1) {
			if (k & key_diff) {
				button(k, (k&vmd->bits.k), vmd->bits.k, vmd_old->bits.k);
			}
		}
	}

	// inner wheel
	// .wheel_i(dir,hist) pos: <= 2 0 1 /3/ 2 0 1 =>, left:-1, right:1
	unsigned char wi_diff = vmd->bits.i ^ vmd_old->bits.i;
	if (wi_diff) {
		wi_hist = (wi_hist << 4) | vmd->bits.i;

		if (wi_right[vmd->bits.i] == (wi_hist & 0xffff))
				wi_dir++;
		else
		if (wi_left [vmd->bits.i] == (wi_hist & 0xffff))
				wi_dir--;

//func("wi: %02x mv: %s (%i) %08x", wi_diff, (wi_dir > 0 ? "right" : "left"), wi_dir, wi_hist);
		if (vmd->bits.i==3) { // wheel is on a lock position
			wi_dir = (wi_dir > 0 ? 1 : -1);
			inner_wheel(wi_dir, wi_hist);
		}
	}

	// outer wheel
	// .wo(speed, speed_old)
	unsigned char wo_diff = vmd->bits.o ^ vmd_old->bits.o;
	if (wo_diff) {
		int s = o_wheel_speed[vmd->bits.o];
		int so = o_wheel_speed[vmd_old->bits.o];
		func("wo: %02x -> speed: %i old: %i", wo_diff, s, so);
		outer_wheel(s, so);
	}

	vmd_old->w = vmd->w;
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
	unsigned char ch;
	unsigned char *data = vmd->data;
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
			if (read_pos == sizeof(ViMoData)) { // packet complete
				read_pos = 0;
				if (data[0] == 0x03) {
					dispatch();
				} else {
					func("%s invalid data packet (%s): %08x",
						__PRETTY_FUNCTION__, filename, vmd->w
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
		active = false;
		return 0;
	}
	return 0;
}


// activate is removed from controller
// bool active is operated directly
// this solves a problem with swig and virtual inheritance...
// hopefully removing the flush data here won't hurt!
// (jrml & shammash)

// bool ViMoController::activate(bool state) {
// 	// also called on register_controller
// 	// flush data
// 	if (fd)
// 		tcflush(fd, TCIOFLUSH);
// 	return Controller::activate(state);
// }

JS(js_vimo_open) {
	ViMoController *vmc = (ViMoController *)JS_GetPrivate(cx, obj);
	if (!vmc) {
		error("%s core data NULL", __PRETTY_FUNCTION__);
		return JS_FALSE;
	}
	if (argc == 0) {
		return JS_NewNumberValue(cx, vmc->open(), rval);
	}
	if (argc == 1) {
		char *filename;
		JS_ARG_STRING(filename, 0);
		return JS_NewNumberValue(cx, vmc->open(filename), rval);
	}
	JS_ERROR("Wrong number of arguments");
}

JS(js_vimo_close) {
	ViMoController *vmc = (ViMoController *)JS_GetPrivate(cx, obj);
	if (!vmc) {
		error("%s core data NULL", __PRETTY_FUNCTION__);
		return JS_FALSE;
	}
	vmc->close();
	return JS_TRUE;
}

JS(js_vimo_ctrl_constructor) {
	func("%u:%s:%s",__LINE__,__FILE__,__FUNCTION__);

	ViMoController *vimo = new ViMoController();

	// initialize with javascript context
	if(! vimo->init( env ) ) {
		error("failed initializing ViMo controller");
		delete vimo; return JS_FALSE;
	}
	if (argc == 1) {
		char *filename;
		JS_ARG_STRING(filename, 0);
		if(!vimo->open(filename)) {
			error("failed initializing ViMo controller");
			delete vimo; return JS_FALSE;
		}
	}

	// assign instance into javascript object
	if( ! JS_SetPrivate(cx, obj, (void*)vimo) ) {
		error("failed assigning ViMo controller to javascript");
		delete vimo; return JS_FALSE;
	}

	// assign the real js object
	vimo->jsobj = obj;
	vimo->javascript = true;


	*rval = OBJECT_TO_JSVAL(obj);
	return JS_TRUE;
}
