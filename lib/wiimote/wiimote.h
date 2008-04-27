/*-------------------------------------------------------------

$Id: $

wiimote.h -- Wii Remote interface

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

#ifndef __WIIMOTE_H__
#define __WIIMOTE_H__

/*!
 * \file wiimote.h 
 * 
 * \brief Wiimote interface
 * 
 * This header defines most of the functions and structures that are used by
 * the program to access the data returned by the Wiimote, as well as to control
 * it. This includes initializing the remote, changing operating modes, turning
 * peripherals on and off, and sending and receiving data and status
 */ 

#include <wiimote_internal.h>
#include <bluetooth.h>

/*!
 * \brief Wiimote or extension buttons
 * 
 * A generic type used to encapsulate a set of buttons (currently up to 16)
 * present on a physical device (either the Wiimote or one of its extensions).
 * Bits that are set indicate buttons that are pressed. Unused bits may be
 * present and should be ignored.
 */
typedef u16 wm_buttons_t;

/**
 * \name Wiimote button definitions
 * 
 * The buttons available on the Wiimote itself, excluding the Power button
 * which is special. These values are ORed together and form a wm_buttons_t.
 * The undefined bits should be ignored. See wm_buttons_t.
 */
/*@{*/
#define WM_BTN_TWO   0x0001
#define WM_BTN_ONE   0x0002
#define WM_BTN_B     0x0004
#define WM_BTN_A     0x0008
#define WM_BTN_MINUS 0x0010
#define WM_BTN_HOME  0x0080
#define WM_BTN_LEFT  0x0100
#define WM_BTN_RIGHT 0x0200
#define WM_BTN_DOWN  0x0400
#define WM_BTN_UP    0x0800
#define WM_BTN_PLUS  0x1000
#define WM_BTN_MASK  0x1f9f
/*@}*/

/*!
 * \name Infrared sensor modes
 * 
 * The Wiimote's IR sensor has three modes of operation, plus a disabled mode.
 * Each mode returns a different set of data to the host.
 */
/*@{*/
/*! \brief IR sensor off
 * 
 * Shuts down the IR sensor. This conserves battery power.
 */
#define WM_IR_OFF      0
/*! \brief Basic IR data
 * 
 * The IR sensor returns only object coordinates.
 */
#define WM_IR_BASIC    1
/*! \brief Extended IR data
 * 
 * The IR sensor returns object coordinates and a
 * rough size value for each object.
 */
#define WM_IR_EXTENDED 3
/*! \brief Full IR data (UNSUPPORTED)
 * 
 * The IR sensor returns more information about each
 * object. This mode is not yet supported.
 */
#define WM_IR_FULL     5
/*@}*/

/*!
 * \name Wiimote reporting modes
 * 
 * The Wiimote returns input data packed into a single report.
 * There are several report modes, and each mode combines data
 * from several peripherals in different ways. There is no
 * mode better than the rest; each has its own limitations.
 * 
 * When used together with IR data, the mode MUST match the IR
 * mode. Even though WM_IR_BASIC data would fit into the space
 * allocated for IR in an IREXT reporting mode, this will not
 * work.
 * 
 * Note that there is no Extended IR mode with extension data, which is why
 * the dot sizes are fixed in the Wii Settings sensitivity adjustment screen
 * if there is a Nunchuk or a Classic Controller plugged in.
 */
/*@{*/
/*! \brief Buttons only
 * 
 * Reports only the state of the Wiimote's buttons
*/
#define WM_MODE_BUTTONS                     0x30
/*! \brief Buttons and accelerometer
 * 
 * Reports the button state and the three-axis acceleration.
 */
#define WM_MODE_BUTTONS_ACCEL               0x31
/*! \brief Buttons and 8 extension bytes
 * 
 * Reports the button state and 8 bytes from the extension.
 */
#define WM_MODE_BUTTONS_EXT8                0x32
/*! \brief Buttons, accelerometer, and Extended IR
 * 
 * Reports the button state, the acceleration, and Extended IR data.
 */
#define WM_MODE_BUTTONS_ACCEL_IREXT         0x33
/*! \brief Buttons and 19 extension bytes
 * 
 * Reports the button state and 19 bytes from the extension.
 */
#define WM_MODE_BUTTONS_EXT19               0x34
/*! \brief Buttons, accelerometer, and 16 extension bytes
 * 
 * Reports the button state, the acceleration, and 16 bytes from the extension.
 */
#define WM_MODE_BUTTONS_ACCEL_EXT16         0x35
/*! \brief Buttons, Basic IR, and 9 extension bytes
 * 
 * Reports the button state, Basic IR data, and 9 extension bytes.
 */
#define WM_MODE_BUTTONS_IRBASIC_EXT9        0x36
/*! \brief Buttons, Acceleration, Basic IR, and 6 extension bytes
 * 
 * Reports the button state, Acceleration, Basic IR data, and 6 extension bytes.
 * This is the only mode that combines all four features from the Wiimote, and it
 * is the mode used by games such as Twilight Princess which use them.
 */
#define WM_MODE_BUTTONS_ACCEL_IRBASIC_EXT6  0x37
/*! \brief 21 extension bytes
 * 
 * Reports only 21 bytes of extension data. This would presumably be used by some
 * extension peripheral designed to be used only by itself, essentially using 
 * the Wiimote only as a Bluetooth interface. Something big, since 21 bytes is a 
 * lot of data.
 */
#define WM_MODE_EXT21                       0x3d
/*! \brief Interleaved reports: buttons, accelerometer, and Full IR
 * 
 * This is a strange mode that halves the data rate by splitting data into two
 * separate reports that are then interleaved. This doesn't include extension data,
 * but it provides a lot of IR data (Full mode) in a format that hasn't been
 * reverse engineered yet. We do know that it is simply more information about
 * each dot though -- it still tracks only four dots and there is no pixel data
 * or anything around those lines. This mode isn't supported at the moment.
 */
#define WM_MODE_BUTTONS_ACCEL_IRFULL        0x3e
#define WM_MODE_BUTTONS_ACCEL_IRFULL_A      0x3e
#define WM_MODE_BUTTONS_ACCEL_IRFULL_B      0x3f
/*@}*/

/*!
 * \name Wiimote data validity flags
 * 
 * These flags are used to indicate which data is valid (up to date)
 * in the returned data structure. In other words, these flags
 * indicate the data types that were returned in the last packet
 * from the Wiimote. This mechanism exists because, under certain
 * circumstances, access to normal data is suspended and only a subset
 * is available. For example, when reading or writing memory, only
 * button data is available until the write or read completes.
 * 
 * The application should only process new data that is marked as new
 * by these flags. However, the old data is still available. For example,
 * reading the IR data during a memory read will return the IR data
 * last acquired before the read, until the read completes and new data
 * is available.
 * 
 * These flags are ORed and returned in the types member of wm_input_t.
 */
/*@{*/
/*! \brief Buttons data is available */
#define WM_DATA_BUTTONS    0x01
/*! \brief IR data is available */
#define WM_DATA_IR         0x02
/*! \brief Extension data is available
 * 
 * Extension data is available. Note that this will not be set even if
 * there is extension data if there is no extension controller
 * physically plugged in. This however does not guarantee the validity of
 * the data - make sure you check for the right controller type in
 * wm_extension_t.
 */
#define WM_DATA_EXT        0x04
/*! \brief Acceleration data is available */
#define WM_DATA_ACCEL      0x08
/*@}*/

/*!
 * \name Wiimote LEDs
 * 
 * These flags are ORed to define which player LEDs are to be turned
 * on in the Wiimote. See wiimote_set_leds().
 */
/*@{*/
#define WM_LED_1 0x10
#define WM_LED_2 0x20
#define WM_LED_3 0x40
#define WM_LED_4 0x80
/*! \brief LED Mask
 * 
 * Represents all LEDs lit.
 */
#define WM_LED_MASK 0xf0
/*@}*/

/*!
 * \name Wiimote status flags
 * 
 * These flags are returned by the wiimote to indicate the status
 * of several internal functions. See wm_status_t.flags.
 */
/*@{*/
/*! \brief Extension connected
 * 
 * This flag indicates that an extension is physically connected.
 * 
 * Note that this only means that the "connection detect" pin
 * is connected, not that there is a valid data connection. Dirt,
 * a loose connection, etc can prevent an extension from working 
 * even though this flag may be set.
 */
#define WM_STATUS_EXT_CONNECTED 0x02
/*! \brief Speaker is enabled
 * 
 * This flag indicates that the speaker is enabled. The speaker
 * is not yet supported in this library.
 */
#define WM_STATUS_SPK_ENABLED   0x04
/*! \brief Continuous reporting enabled
 * 
 * This flag indicates that the wiimote is in continuous reporting
 * mode. This means that it will report data at its nominal rate
 * whether the data changes or not.
 */
#define WM_STATUS_CONTINUOUS    0x08
/*! \brief Status mask
 * 
 * Represents all possible status flags.
 */
#define WM_STATUS_MASK          0x0e
/*@}*/

/*! \name R/W Devices
 * 
 * The wiimote includes several serial bus devices. Each device exposes
 * an address space that can be read and written. In addition, the Wiimote
 * includes an EEPROM memory chip that can also be read and written.
 * These defines are used to specify which address space or device is used
 * in a read or write operation. See wiimote_read() and wiimote_write().
 */
/*@{*/
/*! \brief EEPROM Memory
 * 
 * Read or write to or from the internal EEPROM memory. Note that some
 * calibration constants are stored here, so be careful when writing.
 */
#define WM_DEVICE_MEMORY        0x00
/*! \brief Speaker controller
 * 
 * Read or write to or from the registers of the speaker audio decoder / DAC.
 */
#define WM_DEVICE_SPEAKER       0xa2
/*! \brief Extension controller
 * 
 * Read or write to or from the connected extension controller. Note that
 * the controller data is encrypted when read, both via data reads and via
 * input reports. See wiimote_decrypt_extension().
 */
#define WM_DEVICE_EXTENSION     0xa4
/*! \brief IR Camera
 * 
 * Read or write to or from the infrared camera / tracking DSP.
 */
#define WM_DEVICE_IR            0xb0
/*@}*/

/*! \brief Default Extension Key
 * 
 * The extension data is encrypted in the wiimote. Currently, this library
 * only supports the null key (zero), which is defined here. See 
 * wiimote_init_extension().
 */
#define WM_KEY_NULL 0x00000000

/*! \brief IR Point
 * 
 * Represents a single IR point as tracked by the IR sensor.
 */
typedef struct {
	/*! \brief X coordinate, 0..1023
	 * 
	 * X coordinate in the sensor frame. Higher is to the right of the
	 * wiimote, lower is to the left.
	 */
	u16 x;
	/*! \brief Y coordinate, 0..768 
	 * 
	 * Y coordinate in the sensor frame. Higher is above the wiimote,
	 * lower is below the wiimote.
	 */
	u16 y;
	/*! \brief Perceived size, ~1..7
	 * 
	 * Approximate size of the dot as seen by the camera. Higher generally
	 * means closer to the source. This is only available in the Extended IR
	 * mode, otherwise it is fixed at 2.
	*/
	u8 size;
	/*! \brief Point is valid (detected)
	 * 
	 * Indicates that this slot is actively tracking a point.
	 * Data is only valid if this is true.
	*/
	int valid;
} wm_ir_point_t;

/*! \brief IR Data
 * 
 * Holds the infrared sensor data, including the four tracked points.
 * 
 * The IR sensor will track up to four points in its four "slots". When a new point
 * comes into view, it will use the first available slot. If a point vanishes from
 * view, an empty slot will be left in the middle. Therefore, when less than four
 * points are tracked, they will not necessarily be in the first slots. If more than
 * four points are tracked, behavior is erratic.
 */
typedef struct {
	/*! \brief IR mode
	 * 
	 * Current IR mode. One of the WM_IR_* constants.
	 */
	int mode;
	/*! \brief Number of tracked dots
	 * 
	 * Number of dots tracked by the camera, 0-4.
	 */
	int count;
	/*! \brief IR Dot data
	 * 
	 * Coordinates and information about each tracked dot. Valid dots may be
	 * situated in any of the four elements of this array. Use wm_ir_point_t.valid
	 * to find the valid slots.
	 */
	wm_ir_point_t point[4];
} wm_ir_t;

/*! \brief Acceleration data
 * 
 * Holds the accelerometer data from either the Wiimote or an extension.
 * 
 * The Wiimote and the Nunchuk extension both include a 10-bit triple-axis accelerometers.
 * The data represents acceleration in a rangeof +/- 3g.
 * 
 * Note that the reference frame is free fall. This means that, when standing still, the
 * wiimote experiences a normal acceleration of 1g upwards. When in free fall, the acceleration
 * is zero for all three axes (unless it is rotating).
 * 
 * Note: this library only returns values with 8 bits of precision for now. The range is still
 * 10 bits, but the two LSBs are zero.
 * 
 * In the following descriptions, the wiimote is assumed to be horizontal, pointing at an imaginary
 * screen, with the buttons side up.
 */
typedef struct {
	/*! \brief X acceleration, -512..+511
	 * 
	 * X acceleration, 10-bit. Positive means the Wiimote is accelerating to the left.
	 */
	s16 x;
	/*! \brief Y acceleration, -512..+511
	 * 
	 * Y acceleration, 10-bit. Positive means the Wiimote is accelerating away from the screen.
	 */
	s16 y;
	/*! \brief Z acceleration, -512..+511
	 * 
	 * Z acceleration, 10-bit. Positive means the Wiimote is accelerating upwards.
	 */
	s16 z;
} wm_accel_t;

/*! \brief Extension data
 * 
 * Holds a generic report of extension data bytes. These bytes may be encrypted. The data
 * must be decrypted and unpacked before it can be used.
 */
typedef struct {
	/*! \brief Data length 0..21
	 * 
	 * Number of bytes of data present in this report, up to 21
	 */
	int length;
	/*! \brief Extension controller type ID
	 * 
	 * Type ID of the controller. One of the WM_EXT_* constants.
	 */
	int type_id;
	/*! \brief Extension data
	 * 
	 * Raw data bytes from the controller. Possibly encrypted.
	 */
	u8 data[21];
} wm_extension_t;

/*! \brief Wiimote input data report
 * 
 * Holds all the data returned by the Wiimote in a single report.
 * When a new report comes in, the data that is available in it is
 * updated. Any data not present in further reports maintains the
 * old values.
 */
typedef struct {
	/*! \brief Last updated data types
	 * 
	 * The data types that were present in the last report and therefore
	 * updated. Normally, this will include all data types reported in the
	 * selected mode (See wiimote_mode()). However, under certain circumstances,
	 * some data may be omitted. This data will retain the old value until it is
	 * updated.
	 * 
	 * This is the logical OR of one or more WM_DATA_* constants.
	 */
	int types;
	/*! \brief Buttons data */
	wm_buttons_t buttons;
	/*! \brief IR data */
	wm_ir_t ir;
	/*! \brief Accelerometer data */
	wm_accel_t accel;
	/*! \brief Extension data */
	wm_extension_t extension;
} wm_input_t;

/*! \brief Wiimote status information
 * 
 * Holds the data returned by the Wiimote in a status report.
 * Status reports are returned by the Wiimote after a wiimote_update_status()
 * call and when certain events such as extension connections or disconnections occur.
 */
typedef struct {
	/*! \brief State of the LEDs
	 * 
	 * The logical OR of WM_LED_* representing the status of the LEDs on the wiimote.
	 */
	u8 leds;
	/*! \brief Status flags
	 * 
	 * The logical OR of one or more WM_STATUS_* flags indicating the state of several features of the Wiimote.
	 */
	u8 flags;
	/*! \brief Battery level
	 * 
	 * The voltage of the battery. Scaling unknown.
	 */
	u8 battery_level;
} wm_status_t;

/*! \brief Wiimote information structure
 * 
 * Holds all the information needed to manage a connection to a Wiimote.
 * In general, the user should never need to access the members of this
 * structure directly. This is passed to most Wiimote library functions.
 */
typedef struct wm_wiimote {
	wm_conn_t connection;
	u8 mode;
	int continuous;
	int rumble;
	u8 leds;
	wm_input_t last_input;
	wm_status_t status;
	int have_intl_buffer;
	u8 intl_buffer[21];
	u32 ext_key;
	u16 ext_id;
	u8 ir_mode;
	u8 cur_sens[11];
	int transfer_status;
	u8 *transfer_data;
} wm_wiimote_t;

/*! \brief Standard Wiimote sensitivity
 * 
 * The wiimote sensitivity setting is described by an
 * 11-byte vector. This "standard" sensitivity is provided
 * as a constant and can be passed to wiimote_set_ir_sens().
 */
extern const u8 wm_ir_sens_std[11];

/*! \brief Standard Wiimote sensitivity
 * 
 * Convenience alias for wm_ir_sens_std
 */
#define WM_IR_SENS_STD wm_ir_sens_std

/*! \brief Get latest wm_input_t data from wm_wiimote_t
 * 
 * This macro returns a pointer to the wm_input_t structure from a wm_wiimote_t structure,
 * containing the latest data for the input functions of the Wiimote.
 * 
 * \param wiimote wm_wiimote_t structure representing the connection to the Wiimote.
 */
#define wiimote_inputdata(wiimote) (&((wiimote)->last_input))

/*! \brief Get latest wm_status_t data from wm_wiimote_t
 * 
 * This macro returns a pointer to the wm_status_t structure from a wm_wiimote_t structure,
 * containing the latest status information from the Wiimote.
 * 
 * \param wiimote wm_wiimote_t structure representing the connection to the Wiimote.
 */
#define wiimote_status(wiimote) (&((wiimote)->status))

/*! \brief Initialize the wiimote structure
 * 
 * After connecting to the Wiimote via the backend, use this function first to initialize
 * a wm_wiimote_t structure and the Wiimote.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure to be filled in
 * \param connection pointer to the wm_conn_t structure that represents the connection to the Wiimote
 * 
 * \return negative if error, zero or positive if successful.
 */
int wiimote_init(wm_wiimote_t *wiimote, const wm_conn_t connection);

/*! \brief Set the reporting mode of the Wiimote
 * 
 * Configure the reporting mode of the wiimote. See the WM_MODE_* constants.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param mode reporting mode to use, from WM_MODE_*.
 * \param continuous if true, the Wiimote reports data at a constant rate. If false,
 *                   the wiimote only reports data when it changes.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_mode(wm_wiimote_t *wiimote, u8 mode, int continuous);

/*! \brief Block until more input is present in the Bluetooth buffer
 * 
 * Blocks until the Wiimote reports more data. This should be rarely used in games
 * (since then you'll probably use a vsync loop), but it can be used in PC demos to yield CPU time
 * between reports.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_block(wm_wiimote_t *wiimote);

/*! \brief Process all reports in the Bluetooth buffer
 * 
 * This function should be called periodically to update the input data
 * in the wm_wiimote_t structure and to handle any special events.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \return the number of reports that were processed, negative if an error occured.
 */
int wiimote_process_reports(wm_wiimote_t *wiimote);

/*! \brief Set the LEDs on the Wiimote
 * 
 * Set the state of the LEDs near the base of the Wiimote.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param leds logical OR of WM_LED_* constants representing which LEDs to turn on.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_set_leds(wm_wiimote_t *wiimote, u8 leds);

/*! \brief Set the state of the rumble motor on the Wiimote
 * 
 * Turn the rumble motor on or off. If this is done many times per second, the speed can be varied.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param rumble if true, turn on the runble motor.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_rumble(wm_wiimote_t *wiimote, int rumble);

/*! \brief Request a status data update from the Wiimote
 * 
 * Asks the Wiimote for an update of the wm_status_t data.
 * If blocking is true, the call will block and process reports until the new
 * data comes in.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param blocking if true, block and process reports until the status report comes in.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_update_status(wm_wiimote_t *wiimote, int blocking);

/*! \brief Writes to the wiimote's memory or peripheral address space
 * 
 * Perform a write to the wiimote's EEPROM or to the register space of one of the on-board
 * peripherals or the external controller.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param device device that the data will be written to. One of WM_DEVICE_*.
 * \param address address of the data (16 bits).
 * \param length length to write, 1..16.
 * \param data pointer to an vector of data bytes to write.
 * \param blocking if true, block until the data is written.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_write(wm_wiimote_t *wiimote, u8 device, u16 address, u8 length, const u8 *data, int blocking);

/*! \brief Reads from the wiimote's memory or peripheral address space
 * 
 * Perform a read from the wiimote's EEPROM or from the register space of one of the on-board
 * peripherals or the external controller.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param device device that the data will be read from. One of WM_DEVICE_*.
 * \param address address of the data (16 bits).
 * \param length length to read, 1..16.
 * \param data pointer to an vector of data bytes where the data read will be stored.
 * \param blocking if true, block until the data is read.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_read(wm_wiimote_t *wiimote, u8 device, u16 address, u16 length, u8 *data, int blocking);

/*! \brief Set the IR mode or turn IR off
 * 
 * Set the mode of the on-board IR camera / tracker, or turn it on/off.
 * The mode must match the selected reporting mode, or no data will be returned.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param mode IR mode. One of WM_IR_*.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_set_ir(wm_wiimote_t *wiimote, u8 mode);

/*! \brief Set the IR sensitivity array
 * 
 * Set the array of parameters that determines the sensitivity of the IR sensor.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param sens Pointer to an array of 11 bytes defining the sensitivity. Use WM_IR_SENS_STD if you've got nothing better.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_set_ir_sens(wm_wiimote_t *wiimote, u8 *sens);

/*! \brief Initialize the extension controller
 * 
 * Initialize the extension controller (if plugged in) and
 * set the encryption key. This is done automatically upon insertion,
 * but it may not work if the controller is inserted very slowly. In this case
 * a manual call to this function will fix things.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param key 32-bit encryption key. Use WM_KEY_NULL for now.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_init_extension(wm_wiimote_t *wiimote, u32 key);

/*! \brief Decrypt extension data
 * 
 * Decrypt the extension data in-place, using the previously specified
 * encryption key. Only WM_KEY_NULL is supported at the moment.
 * 
 * \param wiimote pointer to the wm_wiimote_t structure.
 * \param data pointer to wm_extension_t data that will be decrypted in-place.
 * \return negative if error, zero or positive if successful.
 */
int wiimote_decrypt_extension(wm_wiimote_t *wiimote, wm_extension_t *data);

#endif
