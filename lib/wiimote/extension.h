/*-------------------------------------------------------------

$Id: $

extension.h -- Wii Remote extension controllers

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


#ifndef __EXTENSION_H__
#define __EXTENSION_H__

/*!
 * \file extension.h 
 * 
 * \brief Wiimote extension parsers
 * 
 * This header defines functions to parse input from the Wiimote extension controllers.
 * Currently supported are the Nunchuk and the Classic Controller.
 */ 

#include "wiimote_internal.h"
#include "wiimote.h"

/*! \brief Classic Trigger 0..31
 * 
 * Trigger data, returned by the Classic Controller.
 * 0 is close to released, 31 is close to fully pressed. The actual controllers never go lower than about 3.
 */
typedef s8 wm_trigger_t;

/*!
 * \name Nunchuk button definitions
 * 
 * The buttons available on the Nunchuk. These values are ORed together and form a
 * wm_buttons_t. The undefined bits should be ignored. See wm_buttons_t.
 */
/*@{*/
#define WM_NUNCHUK_BTN_Z    0x0001
#define WM_NUNCHUK_BTN_C    0x0002
#define WM_NUNCHUK_BTN_MASK 0x0003
/*@}*/


/*!
 * \name Classic Controller button definitions
 * 
 * The buttons available on the Classic Controller. These values are ORed together and form a
 * wm_buttons_t. The undefined bits should be ignored. See wm_buttons_t.
 */
/*@{*/
#define WM_CLASSIC_BTN_UP     0x0001
#define WM_CLASSIC_BTN_LEFT   0x0002
#define WM_CLASSIC_BTN_ZR     0x0004
#define WM_CLASSIC_BTN_X      0x0008
#define WM_CLASSIC_BTN_A      0x0010
#define WM_CLASSIC_BTN_Y      0x0020
#define WM_CLASSIC_BTN_B      0x0040
#define WM_CLASSIC_BTN_ZL     0x0080
#define WM_CLASSIC_BTN_RTRIG  0x0200
#define WM_CLASSIC_BTN_PLUS   0x0400
#define WM_CLASSIC_BTN_HOME   0x0800
#define WM_CLASSIC_BTN_MINUS  0x1000
#define WM_CLASSIC_BTN_LTRIG  0x2000
#define WM_CLASSIC_BTN_DOWN   0x4000
#define WM_CLASSIC_BTN_RIGHT  0x8000
#define WM_CLASSIC_BTN_MASK   0xfeff
/*@}*/

/*!
 * \name Extension type IDs
 * 
 * The type identifiers for extensions. See wm_extension_t.type_id. 
 */
/*@{*/
/*! \brief No extension plugged in */
#define WM_EXT_NONE        0xFF00
/*! \brief Bad extension data (cable loose) */
#define WM_EXT_LOOSE       0xFFFF
/*! \brief Nunchuk plugged in */
#define WM_EXT_NUNCHUK     0x0000
/*! \brief Classic Controller plugged in */
#define WM_EXT_CLASSIC     0x0101
/*@}*/

/*! \brief Analog stick
 * 
 * Represents the position of an analog stick
 */
typedef struct {
	/*! \brief X coordinate, -128..+127
	 * 
	 * X coordinate of the analog stick. Positive is to the right.
	 * Note that the resolution of the stick may be less than the
	 * full 8 bits.
	 */
	s8 x;
	/*! \brief Y coordinate, -128..+127
	 * 
	 * Y coordinate of the analog stick. Positive is up.
	 * Note that the resolution of the stick may be less than the
	 * full 8 bits.
	 */
	s8 y;
} wm_stick_t;

/*! \brief Nunchuk data
 * 
 * Holds the data from a Nunchuk report.
 */
typedef struct {
	/*! \brief Acceleration data (same as from the Wiimote) */
	wm_accel_t accel;
	/*! \brief Buttons that are pressed (see WM_NUNCHUK_BTN_*) */
	wm_buttons_t buttons;
	/*! \brief Position of the analog stick */
	wm_stick_t stick;
} wm_nunchuk_t;

/*! \brief Classic Controller data
 * 
 * Holds the data from a Classic Controller report.
 */
typedef struct {
	/*! \brief Buttons that are pressed (see WM_CLASSIC_BTN_*) */
	wm_buttons_t buttons;
	/*! \brief Position of the left analog stick */
	wm_stick_t lstick;
	/*! \brief Position of the right analog stick */
	wm_stick_t rstick;
	/*! \brief Position of the left trigger */
	wm_trigger_t ltrig;
	/*! \brief Position of the right trigger */
	wm_trigger_t rtrig;
} wm_classic_t;

/*! \brief Decrypt and unpack Nunchuk data
 * 
 * Decrypt and unpack the Nnunchuk data from a wm_extension_t structure.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param data pointer to the extension data. See wiimote_inputdata() and wm_input_t.extension
 * \param nunchuk pointer to a wm_nunchuk_t structure that will be filled in by the function
 * \return negative if error, zero or positive if successful.
 */
int wiimote_decode_nunchuk(wm_wiimote_t *wiimote, wm_extension_t *data, wm_nunchuk_t *nunchuk);

/*! \brief Decrypt and unpack Classic Controller data
 * 
 * Decrypt and unpack the Classic Controller data from a wm_extension_t structure.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param data pointer to the extension data. See wiimote_inputdata() and wm_input_t.extension
 * \param classic pointer to a wm_classic_t structure that will be filled in by the function
 * \return negative if error, zero or positive if successful.
 */
int wiimote_decode_classic(wm_wiimote_t *wiimote, wm_extension_t *data, wm_classic_t *classic);

#endif
