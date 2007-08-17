
/** This file is intended solely for being parsed by JSDoc
    to produce documentation for the FreeJ's Javascript API
    it is not a script you can run into FreeJ
    it is not intended to be an example of good JavaScript OO-programming,
    nor is it intended to fulfill any specific purpose apart from generating documentation

    @author  Mr Goil
    @version 0.9
*/

///////////////////////////////////////////////////
/// Controller VIRTUAL PARENT

/** 
    Virtual Controller 
    @class This class is the parent class of all Controller and should not
    @constructor
    be used directly.
*/
function Controller() { };

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
    @param{int} value value -127 to 127
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

/** All keyboard events are exported as .pressed_MOD_KEYSYM

    <div class="example">Examples:

    KeyboardController.pressed_num_1
    kbd.pressed_ctrl_a  = function() { }
    kbd.pressed_ctrl_shift_alt_a = function() { }
    </div>
    Mh, still need to lurk through the code for mode documentation ...
*/
KeyboardController.prototype.pressed_MOD_KEYSYM = function () { };


