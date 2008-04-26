/*-------------------------------------------------------------

$Id: $

extension.c -- Wii Remote extension controllers

Copyright (C) 2007
Hector Martin (marcan)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.      The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.      Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.      This notice may not be removed or altered from any source
distribution.

$Log: $

-------------------------------------------------------------*/

#include "wiimote_internal.h"
#include "wiimote.h"
#include "extension.h"

int wiimote_decode_nunchuk(wm_wiimote_t *wiimote, wm_extension_t *data, wm_nunchuk_t *nunchuk) {
	
	int res;
	wm_extension_t extdata = *data;
	res = wiimote_decrypt_extension(wiimote, &extdata);
	if( res < 0 ) return res;
	
	nunchuk->stick.x = (extdata.data[0] - 0x80);
	nunchuk->stick.y = (extdata.data[1] - 0x80);
	nunchuk->accel.x = (extdata.data[2] - 0x80) << 2;
	nunchuk->accel.y = (extdata.data[3] - 0x80) << 2;
	nunchuk->accel.z = (extdata.data[4] - 0x80) << 2;
	nunchuk->buttons = ~extdata.data[5] & WM_NUNCHUK_BTN_MASK;
	return 1;
}

int wiimote_decode_classic(wm_wiimote_t *wiimote, wm_extension_t *data, wm_classic_t *classic) {
	int res;
	wm_extension_t extdata = *data;
	res = wiimote_decrypt_extension(wiimote, &extdata);
	if( res < 0 ) return res;
	
	classic->lstick.x = ((extdata.data[0] & 0x3f) - 0x20) << 2;
	classic->lstick.y = ((extdata.data[1] & 0x3f) - 0x20) << 2;
	classic->rstick.x = ((((extdata.data[0] & 0xc0) >> 3) | ((extdata.data[1] & 0xc0) >> 5) |
			 ((extdata.data[2] & 0x80) >> 7)) - 0x10) << 3;
	classic->rstick.y = ((extdata.data[2] & 0x1f) - 0x10) << 3;
	classic->ltrig = ((extdata.data[2] & 0x60) >> 2) | ((extdata.data[3] & 0xe0) >> 5);
	classic->rtrig = (extdata.data[3] & 0x1f);
	classic->buttons = ~((extdata.data[4] << 8) | extdata.data[5]) & WM_CLASSIC_BTN_MASK;
	return 1;	
}
