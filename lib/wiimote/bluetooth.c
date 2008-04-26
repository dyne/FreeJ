/*-------------------------------------------------------------

$Id: $

bluetooth.c -- Wii Remote bluetooth backend

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

#include <bluez.h>
#include <wiimote_internal.h>
#include <bluetooth.h>
#include <l2cap.h>

#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>

#define L2CAP_PSM_HIDP_CTRL 0x11
#define L2CAP_PSM_HIDP_INTR 0x13

int wiimote_connect(bdaddr_t address, wm_conn_t *connection)
{
	struct sockaddr_l2 addr_cmd = { 0 };
	struct sockaddr_l2 addr_rpt = { 0 };
	char s_addr[18];
	int status;
	
	ba2str(&address,s_addr);
	DEBUG("Wiimote: Connecting to %s...\n",s_addr);
	
	connection->address = address;
	
	addr_cmd.l2_family = AF_BLUETOOTH;
	addr_cmd.l2_psm = htobs(L2CAP_PSM_HIDP_CTRL);
	addr_cmd.l2_bdaddr = address;
	addr_rpt.l2_family = AF_BLUETOOTH;
	addr_rpt.l2_psm = htobs(L2CAP_PSM_HIDP_INTR);
	addr_rpt.l2_bdaddr = address;
	
	connection->s_cmd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	DEBUG("Wiimote: Allocated command socket %d.\n",connection->s_cmd);
	status = connect(connection->s_cmd, (struct sockaddr *)&addr_cmd, sizeof(addr_cmd));
	if( status != 0 ) {
		DEBUG("Wiimote: Command connection failed (%d).\n",status);
		return status;
	}
	DEBUG("Wiimote: Connected to command socket.\n");
	connection->s_rpt = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	DEBUG("Wiimote: Allocated report socket %d.\n",connection->s_rpt);
	status = connect(connection->s_rpt, (struct sockaddr *)&addr_rpt, sizeof(addr_rpt));
	if( status != 0 ) {
		DEBUG("Wiimote: Report connection failed (%d).\n",status);
		close(connection->s_cmd);
		return status;
	}
	DEBUG("Wiimote: Connected to report socket.\n");
	DEBUG("Wiimote: Connection successful.\n");
	return 0;
}

int wiimote_block_for_report(wm_conn_t *connection) {
	struct pollfd pfd;
	int res;

	pfd.fd = connection->s_rpt;
	pfd.events = POLLIN;
	res = poll(&pfd,1,-1);
	if (res != 1) {
		DEBUG("Wiimote: poll call failed: %d\n",res);
		return -1;
	}
	return 0;
}

int wiimote_get_report(wm_conn_t *connection, int maxlen, u8 *data)
{
	struct pollfd pfd;
	int res;

	pfd.fd = connection->s_rpt;
	pfd.events = POLLIN;
	res = poll(&pfd,1,0);
	
	if (res == 0) {
		return 0; // no reports
	} else if (res < 0) {
		DEBUG("Wiimote: poll call failed: %d\n",res);
		return -1;
	}
	
	res = recv(connection->s_rpt, data, maxlen, 0);
	if( res < 2 ) { // a valid report contains at least two bytes
		DEBUG("Wiimote: error receiving report: %d (max %d)\n",res,maxlen);
		return -1;
	}
	//DEBUG("Wiimote: Report received (ID 0x%02x, len %d)\n", data[1], res);
	return 1;
}

int wiimote_command(wm_conn_t *connection, int length, const u8 *data)
{
	int res;
	//DEBUG("Wiimote: Command on report 0x%02x\n", data[1]);
	
	res = write(connection->s_cmd, data, length);
	if ( res != length ) {
		DEBUG("Wiimote: Command write failed: %d (expected %d)\n", res, length);
		return -1;
	}
	return 0;
}
