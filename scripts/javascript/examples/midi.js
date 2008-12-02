/*  FreeJ example scripts
 *  (c) Copyright 2005 Christoph Rudorff aka MrGoil <goil@dyne.org>
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
 */

/*
Example usage of MidiController() by Mr Goil
*/


kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }

// put something on the stage
i = new ImageLayer(100,100);
i.open("scripts/1steps063.jpg");
add_layer(i);

x = 0;
y = 0;

mc = new MidiController();
register_controller(mc);

// use 'aconnect -li' or aconnectgui to determine port values
ret = mc.connect_from(0, 20, 0);
echo("connect: " + ret);
ret = mc.connect_from(0, 20, 0);     // just test:
echo("connect: " + ret);             // -16 device busy
ret = mc.connect_from(0, 123, 456);  // illegal
echo("connect: " + ret);             // -22 Invalid argument

// assign callbacks
mc.event_ctrl = function (ch, func, value) {
    midi_event("event_ctrl", ch, func, value);
}

mc.event_pitch = function (ch, func, value) {
    midi_event("event_pitch", ch, func, value);
}

mc.event_noteon = function (ch, func, value) {
    midi_event("event_noteon", ch, func, value);
}

mc.event_noteoff = function (ch, func, value) {
    midi_event("event_noteoff", ch, func, value);
}

mc.event_pgmchange = function (ch, func, value) {
    midi_event("event_pgm", ch, func, value);
}

function midi_event(what, ch, func, value) {
    echo("midi js " + what +  " called " + ch + ", " + func + ", " + value); 
    if ( func % 2 )
        x = value;
    else
        y = value;
    i.set_position(x,y);
}


/*
[chris@hirnlego ~/Software/FREEJ/brain]$ aconnect -li
client 0: 'System' [type=Kernel]
    0 'Timer           '
    1 'Announce        '
        Verbinde mit: 15:0, 130:0, 131:0
client 14: 'Midi Through' [type=Kernel]
    0 'Midi Through Port-0'
client 20: 'UC-33 USB MIDI Controller' [type=Kernel]
    0 'UC-33 USB MIDI Controller MIDI '
    1 'UC-33 USB MIDI Controller MIDI '

[chris@hirnlego ~/Software/FREEJ/brain]$ aconnect -lo
client 14: 'Midi Through' [type=Kernel]
    0 'Midi Through Port-0'
client 20: 'UC-33 USB MIDI Controller' [type=Kernel]
    0 'UC-33 USB MIDI Controller MIDI '
client 128: 'freej MidiController' [type=Benutzer]
    0 'MIDI IN         '
    1 'MIDI IN 2       '
client 129: 'freej MidiController' [type=Benutzer]
    0 'MIDI IN         '
    1 'MIDI IN 2       '
*/

