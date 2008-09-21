
/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author  Mr Goil, Jaromil
    @version 0.9
*/

///////////////////////////////////////////////////
/// Controller VIRTUAL PARENT

/** 
    Virtual Controller 
    @constructor
    @class This class is the parent class of all Controller and should 
    not be used directly.

	<p>
	General notes on all controllers:
	<ul>
		<li>They will only be active as long as their js object is alive. If it runs out of scope or is deleted, the controller will be removed on the next gc() call.
		<li>If the callback function returns 'true', the event is 'handled', dispatch ends.
		<li>If the callback function returns 'false' or nothing, the pending event will be requeued to the next controller, if any.
		<li>If a callback failed because of any script errors, the controller will be deactivated *). Use {@link #activate activate(true)} to reenable.
	</ul>
	*) not true for Keyboardcontroller and MidiController
	</p>

*/
function Controller() { };

/** de/activate the controller
	<p>If called with one parameter, it sets the state and returns the old state. Without parameters, it just returns the current status.</p>

	@author MrGoil
    @returns last state
    @type bool
	@param {bool} new state
*/
Controller.prototype.activate = function activate(new state) { };

/** get the name of the controller
    (not implemented yet)
    @returns controller name
    @type String
*/
Controller.prototype.get_name = function get_name() { };

/** The Midi Controller constructor creates a midi controller 
    @class The Midi Controller holds callbacks to javascript on midi events.
    Assign functions to the callback to handle events:
    <div class="example">Example:

    mc = new MidiController();
    register_controller(mc);
    mc.event_ctrl = function (ch, param, value) {
        echo("midi event ctrl called: " + ch + ", " + param + ", " + value); 
        // do something
        return true;
    }
    </div>
    You want to use a tool like aconnect/aconnectgui to wire the midi devices.
    You may return true or false in the event handlers to indicate further processing or not. However, this is not implemented properly, yet.

    @author Mr Goil
    @constructor
    @base Controller
    @returns a new midi controller
*/
function MidiController() { };
MidiController.prototype = new Controller();

/** Callback on midi event_ctl.
    Usually this is triggered when moving a sliders or button.
    @return true if event is handled otherwise false
    @type bool
    @param{int} ch channel
    @param{int} param parameter number
    @param{int} value value
*/
MidiController.prototype.event_ctrl = function (ch, param, value) { }

/** Callback on midi event_pitch

    A pitch slider is like event_ctl but they have '0' zero in the middle.
    in the middle position.
    @return true if event is handled otherwise false
    @type bool
    @param{int} ch channel
    @param{int} param parameter number
    @param{int} value range -8192 to 8192, steps of 128
*/
MidiController.prototype.event_pitch = function (ch, param, value) { }

/** Callback on midi event_noteon

    @return true if event is handled otherwise false
    @type bool
    @param{int} ch channel
    @param{int} note note
    @param{int} vel velocity
*/
MidiController.prototype.event_noteon = function (ch, note, vel) { }

/** Callback on midi event_noteoff

    @return true if event is handled otherwise false
    @type bool
    @param{int} ch channel
    @param{int} note note
    @param{int} vel velocity
*/
MidiController.prototype.event_noteoff = function (ch, note, vel) { }

/** Callback on midi event_pgmchange

    @return true if event is handled otherwise false
    @type bool
    @param{int} ch channel
    @param{int} param parameter number
    @param{int} value value
*/
MidiController.prototype.event_pgmchange = function (ch, param, value) { }

/** connect an midi output port with freej input.
    connect_to() isn't implemented, yet ... ;)
    @type int
    @return status 0 on success, -16 Device or resource busy, -22 Invalid argument
    @param {int} myport freej port number, usually 0
    @param {int} dest_client lookup the input source with "aconnect -li"
    @param {int} dest_port port number of the input client
*/
function connect_from(myport, dest_client, dest_port) { };
MidiController.prototype.connect_from = connect_from;

//////////////////////////////
// Keyboard Controller

/** KeyboardController constructor creates a keyboard controller 
    @class The KeyboardController holds callbacks to javascript.
    Assign functions to the callback to handle events:
    <div class="example">Example:

    kbd = new KeyboardController();
    register_controller( kbd );

    kbd.pressed_ctrl_q = function() { quit(); }
    </div>

    @author jaromil
    @constructor
    @base Controller
    @returns a new keyboard controller
*/
function KeyboardController() { };
KeyboardController.prototype = new Controller();

/** All keyboard events are exported as .pressed_MOD_KEYSYM as in pressed_a pressed_b pressed_c or pressed_shift_a but also as .released_MOD_KEYSYM as in released_a ... see below for a full list of implemented MOD_KEYSYMs (see also the default keyboard.js in share/freej/scripts)
<div class="example">

kbd = new KeyboardController();
register_controller( kbd );

kbd.pressed_1 
kbd.pressed_2 
kbd.pressed_3 
kbd.pressed_4 
kbd.pressed_5 
kbd.pressed_6 
kbd.pressed_7 
kbd.pressed_8 
kbd.pressed_9 
kbd.pressed_0 

kbd.pressed_a 
kbd.pressed_b 
kbd.pressed_c 
kbd.pressed_d 
kbd.pressed_e 
kbd.pressed_f 
kbd.pressed_g 
kbd.pressed_h 
kbd.pressed_i 
kbd.pressed_j 
kbd.pressed_k 
kbd.pressed_l 
kbd.pressed_m 
kbd.pressed_n 
kbd.pressed_o 
kbd.pressed_p 
kbd.pressed_q 
kbd.pressed_r 
kbd.pressed_s 
kbd.pressed_t 
kbd.pressed_u 
kbd.pressed_v 
kbd.pressed_w 
kbd.pressed_x 
kbd.pressed_y 
kbd.pressed_z 

// more letter keys are available
// in combination with control, shift or alt keys
// define them as:
kbd.pressed_ctrl_a  
kbd.pressed_shift_b 
kbd.pressed_alt_c   
// .. and so on with other letters
// you can also combine ctrl+shift+alt for example:
kbd.pressed_ctrl_shift_alt_a 

// symbol keys:
kbd.pressed_up       
kbd.pressed_down     
kbd.pressed_insert   
kbd.pressed_home     
kbd.pressed_end      
kbd.pressed_pageup   
kbd.pressed_pagedown 

kbd.pressed_backspace 
kbd.pressed_tab       
kbd.pressed_return    
kbd.pressed_space     
kbd.pressed_less      
kbd.pressed_greater   
kbd.pressed_equals    

// numeric keypad keys:
kbd.pressed_num_1 
kbd.pressed_num_2 
kbd.pressed_num_3 
kbd.pressed_num_4 
kbd.pressed_num_5 
kbd.pressed_num_6 
kbd.pressed_num_7 
kbd.pressed_num_8 
kbd.pressed_num_9 
kbd.pressed_num_0 

kbd.pressed_num_period   
kbd.pressed_num_divide   
kbd.pressed_num_multiply 
kbd.pressed_num_minus    
kbd.pressed_num_plus     
kbd.pressed_num_enter    
kbd.pressed_num_equals   

// to quit we have default keys.
// never forget to define the quit key! ;^)
kbd.pressed_ctrl_q 
kbd.pressed_ctrl_c 
kbd.pressed_esc    
</div>
*/
KeyboardController.prototype.pressed_MOD_KEYSYM = function () { };


//////////////////////////////
// Joystick Controller

/** The JoystickController constructor creates a joystick controller 
    @class The Joystick Controller holds callbacks to javascript.
    Assign functions to the callback to handle events:
    <div class="example">Example:

    jc = new JoystickController();
    register_controller(jc);
    jc.axismotion = function (which, axis, value) {
        echo("joystick axis event: " + which + ", " + axis + ", " + value); 
        // do something
        return true;
    }
    </div>

    @constructor
    @base Controller
    @returns a new JoystickController
*/
function JoystickController() { };
JoystickController.prototype = new Controller();

/** Callback on moving a joystick axis

    @return true if event is handled otherwise false
    @type bool
    @param{int} which Joystick device index
    @param{int} axis Axis index
    @param{int} value axis position range usually -32767 to 32767
*/
JoystickController.prototype.axismotion = function (which, axis, value);

/** Callback on trackball motion event

    @return true if event is handled otherwise false
    @type bool
    @param{int} which The joystick device index
    @param{int} ball The joystick trackball index
    @param{int} xrel The relative motion in the X direction
    @param{int} yrel The relative motion in the Y direction

*/
JoystickController.prototype.ballmotion = function (which, ball, xrel, yrel);

/** Callback on joystick hat position change

    <div class="example">Positionvalues ('or'ed in the corners):
<pre>
     0x01
0x08 0x00 0x02
     0x04</pre></div>

    @return true if event is handled otherwise false
    @type bool
    @param{int} which The joystick device index
    @param{int} hat The joystick hat index
    @param{int} value The hat position value

*/
JoystickController.prototype.hatmotion = function (which, hat, value);

/** Callback on button event

    @return true if event is handled otherwise false
    @type bool
    @param{int} which The joystick device index
    @param{int} button The joystick button index
    @param{int} state 0=up 1=down

*/
JoystickController.prototype.button = function (which, button, state);

//////////////////////////////
// Open Sound Controller

/** The  OscController   constructor  creates  a   scriptable  network
 * listener for OSC protocol commands
 @class The OscController holds a  list of defined methods scripted to
 execute actions  when they  are called from  a OSC client  across the
 network.
 <div class="example">Define new methods, their name will be exposed as OSC methods:
 <pre>
 // open an OSC listener on port 9696
 // please note the port is a number in quotes
osc = new OscController("9696");

 // create the callback function that will be called
osc.test_callback =  function test_callback(i,f,s) {
     echo("OSC remote call to my test function");
     echo("int argument is: " + i);
     echo("float argument is: " + f);
     echo("string argument is: " + s);
     return true;
};

 // assign my test_callback to an OSC method, declaring the arguments that will
 // be passed in a string sequence: i -> int32 , f -> float , s -> string
osc.add_method("/test","ifs","test_callback");

 // starts the osc listener thread
osc.start();
 // to stop it during execution use osc.stop();

 // and don't forget to register the controller
register_controller(osc);

 // now you can call the method /test with arguments, for example:
 // using the OSC message "/test,123,6.66,hello_world" on port 9696
 
 </pre></div>

 @author Steve Harris (liblo), Jaromil
 @base Controller
 @returns a new OscController
*/
function OscController() { };
OscController.prototype = new Controller();

/** Add a method that will be called from OSC
    @param{string} osc_method The name of the OSC method to be served
    @param{string} prototype  A string listing the arguments accepted: i=int, f=float, s=string
    @param{string} js_method  The javascript function that will be called and passed the arguments
*/
OscController.prototype.add_method = function(osc_method, prototype, js_method);

//////////////////////////////
// Trigger Controller

/** The TriggerController constructor creates a trigger controller 
    @class The Trigger Controller holds callbacks to javascript. You can use the frame callback to process various stuff, instead using run(). 

    @author MrGoil
    @constructor
    @base Controller
    @returns a new TriggerController
*/
function TriggerController() { };
TriggerController.prototype = new Controller();

/** This will be called each frame. 
    Don't waste too much time here!

    @type void
*/
TriggerController.prototype.frame = function ();

//////////////////////////////
// Mouse Controller

/** The MouseController constructor creates a controller which receives mousebutton and mousemotion events. It dispatches them to {@link #motion .motion()} and {@link #button .button()}.
	@class Get mouse button and motion events of the SDL output window. The Mouse Controller holds callbacks to javascript.
	@author MrGoil
	@constructor
	@base Controller
	@returns a new MouseController
*/
function MouseController() { };
MouseController.prototype = new Controller();

/** This will be called on mouse button up and down.

	@type bool callback
	@return return true, when event was handled, false to requeue event
	@param{int} button number from 1 up to ...
	@param{int} state 0=up 1=down
	@param{int} x where in the viewport it happened
	@param{int} y
*/
MouseController.prototype.button = function (button, state, x, y);

/** This will be called when mouse is moving over the viewport.
	If the input is grabbed, then the mouse will give relative motion events even when the cursor reaches the edge fo the screen. This is currently only implemented on Windows and Linux/Unix-a-likes.

	@type bool callback
	@return return true, when event was handled, false to requeue event
	@param{int} buttonmask mousebutton bitmap
	@param{int} x where in the viewport it happened
	@param{int} y
	@param{int} xrel Relative motion in the X/Y direction
	@param{int} yrel
*/
MouseController.prototype.motion = function (buttonmask, x, y, xrel, yrel);

/** Grabbing means that the mouse is confined to the application window.
	Hides the Cursor.

	<p>Just in case you fuck up with it, the grab will be released when:
    <ul>
		<li>the controller object is deleted
		<li>the controller is {@link GLOBALS#rem_controller removed}
		<li>script errors in the callback function
		<li>{@link Controller#activate activate}(false)
	</ul></p>
	@see #motion
	@type bool
	@return last state or current state when called w/o parameter
	@param{bool} state true to grab the mouse, false to release
*/
MouseController.prototype.grab = function (state);



//////////////////////////////
// Video Mouse Controller

/** The ViMoController constructor creates a controller which holds the callbacks.
	There's an example script "vimo.js".
	@class 
	<p>The ViMoController is for this fancy serial Video Mouse device:</p>
	<img src="images/gse2-500.jpg">
	<p>
	I had no specs for hacking this driver. One mouse had a floppy disc in it's package labeled "WinEdit-Version 1.1". I ran that in qemu and sniffed the line. You may ask if this driver supports the IR sensor or the connectors ... mh, WinEdit came up with a popup: "Sorry, this feature is not implemented yet."<br>
	Check eBay or your attic!
	</p>
	@author MrGoil
	@constructor
	@base Controller
	@returns a new ViMoController
	@param{string} filename e.g. "/dev/ttyS0". If you set the device filename here, the constructer calls open(filename).
	You can skip the filename and do open(filename).
*/
function ViMoController(filename) { };
ViMoController.prototype = new Controller();

/** This will be called on mouse button up and down.
	Each button generates one event.
    <div class="example">button number values (masks are the 'or'ed status):
1  rev
2  cut
4  fwd
8  pause
16 stop
32 play
64 cancel
</div>

	@type bool callback 
	@return return whatever ... we don't care
	@param{int} button number value
	@param{int} state 0=up 1=down
	@param{int} mask current button bitmask
	@param{int} old_mask old button bitmask
*/
ViMoController.prototype.button = function (button, state, mask, old_mask);

/** This will be called when turning the outer wheel.

	@type bool callback 
	@return return whatever ... we don't care
	@param{int} speed new position range -7 to 7, 0 is the middle. Don't be confused: the wheel seems to have two '0' positions.
	@param{int} old_speed previous value
*/
ViMoController.prototype.wheel_o = function (speed, old_speed);

/** This will be called when turning the inner wheel at a locked position.
	It's not easy to determiate the direction if you move it too fast.
	The device returns a 3 on a lock position and between 3->2->0->1 to the right, 3->1->0->2 to the left.

	@type bool callback 
	@return return whatever ... we don't care
	@param{int} direction -1=left, +1=right
	@param{uint} history bitmapped history, for debugging or whatever. Left oldest, right octet current position. Each octet is one position, range 0 - 3.
*/
ViMoController.prototype.wheel_i = function (direction, history);

/** opens device and locks it. If a filename is set, open() works without parameter, too.

	@type int
	@return 0 fail 1 ok
	@param{string} filename name of the serial device where the mouse is attached
*/
ViMoController.prototype.open = function (filename);

/** closes device and releases lock.

	@type void
*/
ViMoController.prototype.close = function ();


//////////////////////////////
// Wii Controller

/** Nintendo Wii remote control.
	<p>The WiiController constructor creates a controller which holds the callbacks.
	There's an example script "wiimote.js".</p>
	@class 
	<p><b><i>This documentation and the Wii is still under construction.</i></b>However, this controller is already working.</p>
	<p>The WiiController connects via bluetooth to a Nintendo wii remote controller.</p>
	<img src="images/wii_ctrl.jpg">
	<p>
	</p>
	@author jaromil, chris
	@constructor
	@base Controller
	@returns a new WiiController
	@param{string} outdevice IDEA/TODO(?) select bluetooth host device, e.g. 'hci0', 'hci1'
	@type WiiController
*/
function WiiController(outdevice) { };
WiiController.prototype = new WiiController();

/** Callback on accelleration changes
	@type bool callback 
	@return return true, when event was handled, false to requeue event
	@param{int} x current x-axis value
	@param{int} y current y-axis value
	@param{int} z current z-axis value
*/
WiiController.prototype.acceleration = function (x, y, z);

/** Connect to wii. Press A+B buttons on the remote.
	Actually, this call blocks freej until the wii is connected.
	@type bool
	@return actually this call returns nothing in any cases ...
	@param{string} bdaddr (optional) wii remote device address, e.g. '00:19:1D:66:91:D3'
	Leave empty to scan and connect to any wii.
*/	
WiiController.prototype.connect = function (bdaddr);

/** toggle acceleration data on/off
	@type void
	@param{bool} state switch acceleration datastream. off by default.
*/
WiiController.prototype.toggle_accel = function (state);

/** toggle button data on/off
	@type void
	@param{bool} state switch button datastream. on by default.
*/
WiiController.prototype.toggle_buttons = function (state);

/** toggle rumble on/off
	@type void
	@param{bool} state switch rumble
*/
WiiController.prototype.toggle_rumble = function (state);

/** toggle led on/off
	@type void
	@param{int} state bitmask state TODO: FIXME
*/
WiiController.prototype.toggle_led = function (state);

