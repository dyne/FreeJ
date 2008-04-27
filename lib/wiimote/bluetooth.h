/*-------------------------------------------------------------

$Id: $

bluetooth.h -- Wii Remote bluetooth backend

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

#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#include <bluez.h>
#include "wiimote_internal.h"

typedef struct {
	bdaddr_t address;
	int s_cmd;
	int s_rpt;
} wm_conn_t;

int wiimote_connect(bdaddr_t address, wm_conn_t *connection);
int wiimote_block_for_report(wm_conn_t *connection);
int wiimote_get_report(wm_conn_t *connection, int maxlen, u8 *data);
int wiimote_command(wm_conn_t *connection, int length, const u8 *data);

#endif
