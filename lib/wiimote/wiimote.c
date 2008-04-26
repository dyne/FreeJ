/*-------------------------------------------------------------

$Id: $

wiimote.c -- Wii Remote interface

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

#include <stdio.h>
#include <string.h>

#include <wiimote.h>
#include <bluetooth.h>
#include <extension.h>
#include <wiimote_internal.h>

#define HIDP_SET_REPORT 0x52
#define HIDP_REPORT_IN 0xA1

#define WM_ORPT_RUMBLE  0x10
#define WM_ORPT_LEDS    0x11
#define WM_ORPT_MODE    0x12
#define WM_ORPT_IREN    0x13
#define WM_ORPT_SPKEN   0x14
#define WM_ORPT_STATUS  0x15
#define WM_ORPT_WMEM    0x16
#define WM_ORPT_RMEM    0x17
#define WM_ORPT_SPKDAT  0x18
#define WM_ORPT_SPKMUTE 0x19
#define WM_ORPT_IREN2   0x1a

#define WM_IRPT_STATUS  0x20
#define WM_IRPT_RMEM    0x21
#define WM_IRPT_WMEM    0x22


#define WM_SW_ON 0x04
#define WM_SW_OFF 0x00

#define WM_RUMBLE 0x01

#define WM_TS_WANT_READ 0x01
#define WM_TS_WANT_WRITE 0x02
#define WM_TS_WANT_STATUS 0x04

const u8 wm_ir_sens_std[11] = {0x02, 0x00, 0x00, 0x71, 0x01, 0x00, 0xaa, 0x00, 0x64, 0x63, 0x03};

int wiimote_init(wm_wiimote_t *wiimote, wm_conn_t connection)
{
	DEBUG("Wiimote: init\n");
	wiimote->connection = connection;
	// Wiimote defaults
	wiimote->mode = WM_MODE_BUTTONS;
	wiimote->continuous = 0;
	wiimote->rumble = 0;
	wiimote->leds = 0; // technically unset / blinking, but we can't do that
	wiimote->have_intl_buffer = 0;
	memset(&wiimote->last_input,0,sizeof(wm_input_t));
	wiimote->ext_key = 0;
	wiimote->ext_id = WM_EXT_NONE;
	wiimote->ir_mode = WM_IR_OFF;
	wiimote->status.flags = 0;
	wiimote->status.leds = 0;
	wiimote->status.battery_level = 0;
	memcpy(wiimote->cur_sens, wm_ir_sens_std, 11);
	return wiimote_update_status(wiimote, 1);
}

int wiimote_mode(wm_wiimote_t *wiimote, u8 mode, int continuous)
{
	int res;
	DEBUG("Wiimote: Set MODE 0x%02x %s\n",mode,continuous?"CONTINUOUS":"SPORADIC");
	u8 cmd[] = {HIDP_SET_REPORT, WM_ORPT_MODE, 0, mode};
	if( continuous ) cmd[2] |= WM_SW_ON;
	if( wiimote->rumble ) cmd[2] |= WM_RUMBLE;
	res = wiimote_command(&wiimote->connection, sizeof(cmd), cmd);
	if( res < 0 ) return res;
	wiimote->mode = mode;
	wiimote->continuous = continuous;
	return 0;
}

int wiimote_simple_report(wm_wiimote_t *wiimote, u8 report, u8 value)
{
	int res;
	u8 cmd[] = {HIDP_SET_REPORT, report, value};
	if( wiimote->rumble ) cmd[2] |= WM_RUMBLE;
	res = wiimote_command(&wiimote->connection, sizeof(cmd), cmd);
	if( res < 0 ) return res;
	return 0;
}

int wiimote_set_leds(wm_wiimote_t *wiimote, u8 leds)
{
	int res;
	DEBUG("Wiimote: Set LEDs 0x%02x\n",leds);
	res = wiimote_simple_report(wiimote, WM_ORPT_LEDS, leds & 0xF0);
	if( res < 0 ) return res;
	wiimote->leds = leds & 0xF0;
	return 0;
}

int wiimote_update_status(wm_wiimote_t *wiimote, int blocking)
{
	int res;
	DEBUG("Wiimote: Request STATUS\n");
	res = wiimote_simple_report(wiimote, WM_ORPT_STATUS, 0x00);
	if( res < 0 ) return res;
	if( blocking ) {
		wiimote->transfer_status |= WM_TS_WANT_STATUS;
		while(wiimote->transfer_status & WM_TS_WANT_STATUS) {
			res = wiimote_block(wiimote);
			if( res < 0 ) return res;
			res = wiimote_process_reports(wiimote);
			if( res < 0 ) return res;
		}
	}
	return 0;
}

int wiimote_block(wm_wiimote_t *wiimote)
{
	return wiimote_block_for_report(&wiimote->connection);
}

int wiimote_process_reports(wm_wiimote_t *wiimote)
{
	u8 data[32];
	int count = 0;
	int res;
	int iroff;
	int i;
	int data_report = 0;
	
	while(1) {
		res = wiimote_get_report(&wiimote->connection, 32, data);
		if( res < 0 ) return res;
		if( res == 0 ) break;
		
		if( data[0] != HIDP_REPORT_IN ) {
			DEBUG("Wiimote: unknown input packet command 0x%02x. Ignoring.",data[0]);
			continue;
		}
		
		// =========================== INPUT REPORTS ===========================
		// Handle buttons and accelerometer
		if( data[1] >= WM_MODE_BUTTONS && data[1] <= WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6 ) {
			wiimote->last_input.types = WM_DATA_BUTTONS;
			wiimote->last_input.buttons = (data[2]<<8 | data[3]) & WM_BTN_MASK;
			// Handle accelerometer
			if( data[1] == WM_MODE_BUTTONS_ACCEL
						 || data[1] == WM_MODE_BUTTONS_ACCEL_IREXT
						 || data[1] == WM_MODE_BUTTONS_ACCEL_EXT16
						 || data[1] == WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6 ) {
				wiimote->last_input.types |= WM_DATA_ACCEL;
				wiimote->last_input.accel.x = (data[4] - 0x80) << 2;
				wiimote->last_input.accel.y = (data[5] - 0x80) << 2;
				wiimote->last_input.accel.z = (data[6] - 0x80) << 2;
			}
			data_report = 1;
		}
		// Handle extension data
		if( wiimote->status.flags & WM_STATUS_EXT_CONNECTED) {
			switch( data[1] ) {
				case WM_MODE_BUTTONS_EXT8:
					wiimote->last_input.types |= WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[4], 8);
					wiimote->last_input.extension.length = 8;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
				case WM_MODE_BUTTONS_EXT19:
					wiimote->last_input.types |= WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[4], 19);
					wiimote->last_input.extension.length = 19;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
				case WM_MODE_BUTTONS_ACCEL_EXT16:
					wiimote->last_input.types |= WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[7], 16);
					wiimote->last_input.extension.length = 16;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
				case WM_MODE_BUTTONS_IRBASIC_EXT9:
					wiimote->last_input.types |= WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[14], 9);
					wiimote->last_input.extension.length = 9;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
				case WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6:
					wiimote->last_input.types |= WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[17], 6);
					wiimote->last_input.extension.length = 6;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
				case WM_MODE_EXT21:
					wiimote->last_input.types = WM_DATA_EXT;
					memcpy(wiimote->last_input.extension.data, &data[2], 21);
					wiimote->last_input.extension.length = 21;
					data_report = 1;
					wiimote->last_input.extension.type_id = wiimote->ext_id;
					break;
			}
		}
		
		iroff = 7;
		
		// Handle IR data
		switch( data[1] ) {
			case WM_MODE_BUTTONS_ACCEL_IREXT:
				wiimote->last_input.types |= WM_DATA_IR;
				wiimote->last_input.ir.mode = WM_IR_EXTENDED;
				wiimote->last_input.ir.count = 0;
				for( i=0; i<4; i++) {
					wiimote->last_input.ir.point[i].x = data[iroff] | ((data[iroff + 2] & 0x30) << 4);
					wiimote->last_input.ir.point[i].y = data[iroff + 1] | ((data[iroff + 2] & 0xc0) << 2);
					wiimote->last_input.ir.point[i].size = data[iroff + 2] & 0x0F;
					wiimote->last_input.ir.point[i].valid = data[iroff + 2] != 0xFF;
					if( wiimote->last_input.ir.point[i].valid ) wiimote->last_input.ir.count++;
					iroff += 3;
				}
				break;
			case WM_MODE_BUTTONS_IRBASIC_EXT9:
				iroff = 4;
				// fallthrough
			case WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6:
				wiimote->last_input.types |= WM_DATA_IR;
				wiimote->last_input.ir.mode = WM_IR_BASIC;
				wiimote->last_input.ir.count = 0;
				for( i=0; i<4; i+=2) {
					wiimote->last_input.ir.point[i].x = data[iroff] | ((data[iroff + 2] & 0x30) << 4);
					wiimote->last_input.ir.point[i].y = data[iroff + 1] | ((data[iroff + 2] & 0xc0) << 2);
					wiimote->last_input.ir.point[i].size = 0x02;
					wiimote->last_input.ir.point[i+1].x = data[iroff + 3] | ((data[iroff + 2] & 0x03) << 8);
					wiimote->last_input.ir.point[i+1].y = data[iroff + 4] | ((data[iroff + 2] & 0x0c) << 6);
					wiimote->last_input.ir.point[i+1].size = 0x02;
					wiimote->last_input.ir.point[i].valid = (data[iroff + 2] & 0xF0) != 0xF0;
					wiimote->last_input.ir.point[i+1].valid = (data[iroff + 2] & 0x0F) != 0x0F;
					if( wiimote->last_input.ir.point[i].valid ) wiimote->last_input.ir.count++;
					if( wiimote->last_input.ir.point[i+1].valid ) wiimote->last_input.ir.count++;
					iroff += 5;
				}
				break;
		}
		
		// The weird interleaved modes
		if( data[1] == WM_MODE_BUTTONS_ACCEL_IRFULL_B) {
			wiimote->last_input.types = WM_DATA_BUTTONS;
			data_report = 1;
			wiimote->last_input.buttons = (data[2]<<8 | data[3]) & WM_BTN_MASK;
			if( wiimote->have_intl_buffer ) {
				u8 tmp;
				DEBUG("Wiimote: Received interleaved FULL report: FULL IR data not yet supported\n");
				wiimote->last_input.ir.mode = WM_IR_FULL;
				wiimote->last_input.ir.count = 0;
				wiimote->last_input.ir.point[0].valid = 0;
				wiimote->last_input.ir.point[1].valid = 0;
				wiimote->last_input.ir.point[2].valid = 0;
				wiimote->last_input.ir.point[3].valid = 0;
				wiimote->last_input.types |= WM_DATA_ACCEL | WM_DATA_IR;
				wiimote->last_input.accel.x = (wiimote->intl_buffer[2] - 0x80)<<2;
				wiimote->last_input.accel.y = (data[4] - 0x80)<<2;
				tmp = (data[2]>>5) & 0x03;
				tmp |= (data[3]>>3) & 0x0c;
				tmp |= (wiimote->intl_buffer[0]>>1) & 0x30;
				tmp |= (wiimote->intl_buffer[1]<<1) & 0xc0;
				wiimote->last_input.accel.z = (tmp - 0x80)<<2;
			}
		}
		
		if( data[1] == WM_MODE_BUTTONS_ACCEL_IRFULL_A) {
			wiimote->last_input.types = WM_DATA_BUTTONS;
			data_report = 1;
			wiimote->last_input.buttons = (data[2]<<8 | data[3]) & WM_BTN_MASK;
			memcpy(wiimote->intl_buffer, &data[2], 21);
			wiimote->have_intl_buffer = 1;
		} else if(data_report) {
			wiimote->have_intl_buffer = 0;
		}

		// =========================== OTHER REPORTS ===========================
		if( !data_report ) {
			u8 old_status_flags;
			switch( data[1] ) {
				case WM_IRPT_STATUS:
					DEBUG("Wiimote: processed STATUS report, flags 0x%02x\n", data[4]);
					old_status_flags = wiimote->status.flags;
					//if an extension was connected, initialize it
					if( !(data[4] && WM_STATUS_EXT_CONNECTED) ) wiimote->ext_id = WM_EXT_NONE;
					wiimote->status.leds = data[4] & WM_LED_MASK;
					wiimote->status.flags = data[4] & WM_STATUS_MASK;
					wiimote->status.battery_level = data[7];
					wiimote->transfer_status &= ~WM_TS_WANT_STATUS;
					if( !(old_status_flags & WM_STATUS_EXT_CONNECTED) && (data[4] & (WM_STATUS_EXT_CONNECTED)))
						wiimote_init_extension(wiimote, WM_KEY_NULL);
					if( (wiimote->status.flags ^ old_status_flags) & WM_STATUS_EXT_CONNECTED)
						//update mode and continue streaming when extension status changes
						wiimote_mode(wiimote, wiimote->mode, wiimote->continuous); 
					break;
				case WM_IRPT_WMEM:
					//TODO: handle result
					DEBUG("Wiimote: processed WRITE STATUS report\n");
					wiimote->transfer_status &= ~WM_TS_WANT_WRITE;
					break;
				case WM_IRPT_RMEM:
					DEBUG("Wiimote: processed READ STATUS report, flags 0x%02x\n",data[4]);
					//TODO: handle errors and long transfers
					memcpy(wiimote->transfer_data, &data[7], 16);
					wiimote->transfer_status &= ~WM_TS_WANT_READ;
					wiimote->last_input.types = WM_DATA_BUTTONS;
					data_report = 1;
					wiimote->last_input.buttons = (data[2]<<8 | data[3]) & WM_BTN_MASK;
					break;
				default:
					DEBUG("Wiimote: unsupported data report type 0x%02x, ignoring\n", data[1]);
					break;
			}
		
		}
		if( data_report )
			DEBUG("Wiimote: processed data report of type 0x%02x, valid data flags 0x%02x\n", data[1], wiimote->last_input.types);
		count++;
	}
	return count;
}

int wiimote_rumble(wm_wiimote_t *wiimote, int rumble)
{
	int res;
	if( (!wiimote->rumble) != (!rumble) ) {
		DEBUG("Wiimote: Set rumble %s\n",rumble?"ON":"OFF");
		wiimote->rumble = rumble;
		// this will automatically set rumble
		res = wiimote_simple_report(wiimote, WM_ORPT_RUMBLE, 0x00);
		if( res < 0 ) return res;
	}
	return 0;
}

int wiimote_read(wm_wiimote_t *wiimote, u8 device, u16 address, u16 length, u8 *data, int blocking)
{
	int res;
	DEBUG("Wiimote: READ dev 0x%02x addr 0x%04x length %d\n", device, address, length);
	//TODO: handle long transfers
	if(length > 16) {
		DEBUG("Wiimote: READ length too large (%d > 16)\n", length);
		return -1;
	}
	u8 cmd[8] = {HIDP_SET_REPORT, WM_ORPT_RMEM,
		(device == WM_DEVICE_MEMORY) ? WM_SW_OFF : WM_SW_ON,
		device, address >> 8, address & 0xFF, length >> 8, length & 0xFF};
	if( wiimote->rumble ) cmd[2] |= WM_RUMBLE;
	res = wiimote_command(&wiimote->connection, sizeof(cmd), cmd);
	if( res < 0 ) return res;
	wiimote->transfer_status |= WM_TS_WANT_READ;
	wiimote->transfer_data = data;
	if( blocking ) {
		while(wiimote->transfer_status & WM_TS_WANT_READ) {
			res = wiimote_block(wiimote);
			if( res < 0 ) return res;
			res = wiimote_process_reports(wiimote);
			if( res < 0 ) return res;
		}
		return 1;
	}
	return 0;

}

int wiimote_write(wm_wiimote_t *wiimote, u8 device, u16 address, u8 length, const u8 *data, int blocking)
{
	int res;
	DEBUG("Wiimote: WRITE dev 0x%02x addr 0x%04x length %d\n", device, address, length);
	if(length > 16) {
		DEBUG("Wiimote: WRITE length too large (%d > 16)\n", length);
		return -1;
	}
	u8 cmd[23] = {HIDP_SET_REPORT, WM_ORPT_WMEM,
		 (device == WM_DEVICE_MEMORY) ? WM_SW_OFF : WM_SW_ON,
		 device, address >> 8, address & 0xFF, length};
	memcpy(&cmd[7], data, length);
	if( wiimote->rumble ) cmd[2] |= WM_RUMBLE;
	res = wiimote_command(&wiimote->connection, sizeof(cmd), cmd);
	if( res < 0 ) return res;
	wiimote->transfer_status = WM_TS_WANT_WRITE;
	if( blocking ) {
		while(wiimote->transfer_status & WM_TS_WANT_WRITE) {
			res = wiimote_block(wiimote);
			if( res < 0 ) return res;
			res = wiimote_process_reports(wiimote);
			if( res < 0 ) return res;
		}
		return 1;
	}
	return 0;

}

int wiimote_set_ir(wm_wiimote_t *wiimote, u8 mode)
{
	int res;
	if( mode == WM_IR_OFF ) {
		u8 data[4] = {0,0,0,0};
		DEBUG("Wiimote: Turning OFF IR\n");
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x30, 4, data, 1);
		if(res < 0) return res;
		res = wiimote_simple_report(wiimote, WM_ORPT_IREN2, WM_SW_OFF);
		if(res < 0) return res;
		res = wiimote_simple_report(wiimote, WM_ORPT_IREN, WM_SW_OFF);
		if(res < 0) return res;
		wiimote->ir_mode = mode;
	} else {
		u8 data[9];
		if( wiimote->ir_mode == WM_IR_OFF ) {
			DEBUG("Wiimote: Turning ON IR\n");
			
			res = wiimote_simple_report(wiimote, WM_ORPT_IREN, WM_SW_ON);
			if(res < 0) return res;
			res = wiimote_simple_report(wiimote, WM_ORPT_IREN2, WM_SW_ON);
			if(res < 0) return res;
		}
		DEBUG("Wiimote: Set IR MODE %d\n", mode);
		
		data[0] = 0x08;
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x30, 1, data, 1);
		if(res < 0) return res;
		
		memcpy(data, &wiimote->cur_sens[0], 9);
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x00, 9, data, 1);
		if(res < 0) return res;
		
		memcpy(data, &wiimote->cur_sens[9], 2);
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x1a, 2, data, 1);
		if(res < 0) return res;
		
		data[0] = mode;
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x33, 1, data, 1);
		if(res < 0) return res;
		
		wiimote->ir_mode = mode;
	}
	return 0;
}

int wiimote_set_ir_sens(wm_wiimote_t *wiimote, u8 *sens)
{
	int res;
	DEBUG("Wiimote: Set IR SENSITIVITY\n");
	memcpy(wiimote->cur_sens, sens, 11);
	if( wiimote->ir_mode != WM_IR_OFF ) {
		u8 data[9];
		
		data[0] = 0x01;
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x30, 1, data, 1);
		if(res < 0) return res;
		
		memcpy(data, &wiimote->cur_sens[0], 9);
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x00, 9, data, 1);
		if(res < 0) return res;
		
		memcpy(data, &wiimote->cur_sens[9], 2);
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x1a, 2, data, 1);
		if(res < 0) return res;
		
		data[0] = wiimote->ir_mode;
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x33, 1, data, 1);
		if(res < 0) return res;
		
		data[0] = 0x08;
		res = wiimote_write(wiimote, WM_DEVICE_IR, 0x30, 1, data, 1);
		if(res < 0) return res;
		
		return 1;
	}
	return 0;
}

int wiimote_init_extension(wm_wiimote_t *wiimote, u32 key)
{
	int res;
	u8 id[2];
	if(key != WM_KEY_NULL) {
		DEBUG("Wiimote: Only NULL key is supported (got 0x%08x)\n",key);
		return -1;
	}
	res = wiimote_write(wiimote, WM_DEVICE_EXTENSION, 0x40, 4, (u8*)&key, 1);
	if(res < 0) return res;
	res = wiimote_read(wiimote, WM_DEVICE_EXTENSION, 0xFE, 2, id, 1);
	if(res < 0) return res;
	DEBUG("Wiimote: Got Extension ID 0x%02x%02x\n",id[0],id[1]);
	if( id[0] == 0x00 && id[1] == 0x00) {
		wiimote->ext_id = WM_EXT_LOOSE;
	} else {
		id[0] = ( id[0] ^ 0x17 ) + 0x17;
		id[1] = ( id[1] ^ 0x17 ) + 0x17;
		wiimote->ext_id = (id[0] << 8) | id[1];
	}
	return 0;
}

int wiimote_decrypt_extension(wm_wiimote_t *wiimote, wm_extension_t *data)
{
	int i;
	if(wiimote->ext_key != WM_KEY_NULL) {
		DEBUG("Wiimote: Only NULL key is supported (got 0x%08x)\n",wiimote->ext_key);
		return -1;
	}
	for(i=0; i<data->length; i++) {
		data->data[i] = (data->data[i] ^ 0x17) + 0x17;
	}
	return 0;

}
