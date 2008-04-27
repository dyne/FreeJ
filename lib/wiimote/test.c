#include <stdio.h>
#include <bluetooth/bluetooth.h>

#include "wiimote.h"
#include "bluetooth.h"
#include "extension.h"

int main(int argc, char **argv)
{
	int res;
	wm_conn_t connection;
	wm_wiimote_t wiimote;

	// can be 		bdaddr = *BDADDR_ANY;
	
	char *wiimote_addr = "00:17:AB:33:37:65";
	bdaddr_t wiimote_bdaddr;
	
	printf("Connecting to Wiimote...\n");
	str2ba(wiimote_addr,&wiimote_bdaddr);
	
	res = wiimote_connect(wiimote_bdaddr,&connection);
	if( res == 0) printf("Connected!\n");
	
	wiimote_init(&wiimote, connection);
	
	wiimote_mode(&wiimote, WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6, 1);
	
	wiimote_set_ir(&wiimote, WM_IR_BASIC);
	
	wiimote_set_leds(&wiimote, WM_LED_2|WM_LED_3);
	
	printf("Press A to rumble!\n");
	
	while(1) {
		int rmbl = 0;
		wm_input_t *input;
		if( wiimote_block(&wiimote) < 0 ) break;
	    if( wiimote_process_reports(&wiimote) < 0 ) break;
		input = wiimote_inputdata(&wiimote);
		
		if( input->types & WM_DATA_BUTTONS ) {
			printf("Buttons pressed: ");
			if( input->buttons & WM_BTN_ONE ) printf("1 ");
			if( input->buttons & WM_BTN_TWO ) printf("2 ");
			if( input->buttons & WM_BTN_A ) printf("A ");
			if( input->buttons & WM_BTN_B ) printf("B ");
			if( input->buttons & WM_BTN_MINUS ) printf("- ");
			if( input->buttons & WM_BTN_PLUS ) printf("+ ");
			if( input->buttons & WM_BTN_HOME ) printf("H ");
			if( input->buttons & WM_BTN_LEFT ) printf("< ");
			if( input->buttons & WM_BTN_RIGHT ) printf("> ");
			if( input->buttons & WM_BTN_UP ) printf("/\\ ");
			if( input->buttons & WM_BTN_DOWN ) printf("\\/ ");
			printf("\n");
			rmbl = input->buttons & WM_BTN_A;
		}
		
		if( input->types & WM_DATA_ACCEL ) {
			printf("Acceleration: %5d %5d %5d\n", input->accel.x, input->accel.y, input->accel.z);
		}
		if( input->types & WM_DATA_IR ) {
			int i;
			printf("IR dots (%d): ",input->ir.count);
			for(i=0; i<4; i++) {
				if( input->ir.point[i].valid ) {
					printf("(%4d, %4d)[%2d] ", input->ir.point[i].x, input->ir.point[i].y, input->ir.point[i].size);
				} else {
					printf("                ");
				}
			}
			printf("\n");
		}
		if( input->types & WM_DATA_EXT ) {
			wm_nunchuk_t nunchuk;
			wm_classic_t classic;
			switch( input->extension.type_id ) {
				case WM_EXT_NONE:
					printf("No extension detected (?)\n");
					break;
				case WM_EXT_LOOSE:
					printf("Extension is loose\n");
					break;
				case WM_EXT_NUNCHUK:
					wiimote_decode_nunchuk(&wiimote, &input->extension, &nunchuk);
					
					printf("Nunchuk Acceleration: %5d %5d %5d\n", nunchuk.accel.x, nunchuk.accel.y, nunchuk.accel.z);
					printf("Nunchuk Stick: %4d %4d\n", nunchuk.stick.x, nunchuk.stick.y);
					printf("Nunchuk Buttons: ");
					if( nunchuk.buttons & WM_NUNCHUK_BTN_Z ) printf("Z ");
					if( nunchuk.buttons & WM_NUNCHUK_BTN_C ) printf("C ");
					printf("\n");
					break;
				case WM_EXT_CLASSIC:
					wiimote_decode_classic(&wiimote, &input->extension, &classic);
					
					printf("Classic Sticks: (%4d, %4d) (%4d, %4d)\n",
						   classic.lstick.x, classic.lstick.y, classic.rstick.x, classic.rstick.y);
					printf("Classic Triggers: %4d %4d\n", classic.ltrig, classic.rtrig);
					printf("Classic Buttons: ");
					if( classic.buttons & WM_CLASSIC_BTN_LEFT ) printf("< ");
					if( classic.buttons & WM_CLASSIC_BTN_RIGHT ) printf("> ");
					if( classic.buttons & WM_CLASSIC_BTN_UP ) printf("/\\ ");
					if( classic.buttons & WM_CLASSIC_BTN_DOWN ) printf("\\/ ");
					if( classic.buttons & WM_CLASSIC_BTN_A ) printf("A ");
					if( classic.buttons & WM_CLASSIC_BTN_B ) printf("B ");
					if( classic.buttons & WM_CLASSIC_BTN_X ) printf("X ");
					if( classic.buttons & WM_CLASSIC_BTN_Y ) printf("Y ");
					if( classic.buttons & WM_CLASSIC_BTN_PLUS ) printf("+ ");
					if( classic.buttons & WM_CLASSIC_BTN_MINUS ) printf("- ");
					if( classic.buttons & WM_CLASSIC_BTN_HOME ) printf("H ");
					if( classic.buttons & WM_CLASSIC_BTN_ZL ) printf("ZL ");
					if( classic.buttons & WM_CLASSIC_BTN_ZR ) printf("ZR ");
					if( classic.buttons & WM_CLASSIC_BTN_LTRIG ) printf("L ");
					if( classic.buttons & WM_CLASSIC_BTN_RTRIG ) printf("R ");
					printf("\n");
					rmbl |= classic.buttons & WM_CLASSIC_BTN_A;
					break;
				default:
					printf("Unsupported extension connected (0x%04X)\n",input->extension.type_id);
					break;
			}
		} else {
			printf("No extension connected.\n");
		}
		wiimote_rumble(&wiimote, rmbl);
	}
	return 0;
}
