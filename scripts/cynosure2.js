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
Javascript Version of the xscreensaver Cynosure
*/


// global values
i           = 0; // iterations
timeLeft    = 0;
curColor    = 0;
curBase     = curColor;

// find my library
inc_ok = false;
["./", "scripts/"].forEach ( function inc(path) {
	if (!inc_ok) {
		try {
			include(path + "param.js");
			inc_ok = true;
		} catch (e) {
			echo("CY incl err: " + e);
		}
	}
} );

if (!inc_ok) {
	echo("cant find my libs ..");
	exit();
}

// define interactive parameters
param = new Array();
// Param(obj, name, in_min, in_max, out_min, out_max, out_start_value)
param[0] = new Param(this, "shadowWidth", 0, 125, 0, 128,  2);
param[1] = new Param(this, "elevation",   0, 125, 0,  64, 10);
param[2] = new Param(this, "sway",        0, 125, 0, 127, 30);
param[3] = new Param(this, "tweak",       0, 125, 1,  32,  20);
param[4] = new Param(this, "gridSize",    0, 125, 2, 127,  30);
param[4].frac=2;
param[5] = new Param(this, "iterations",  0, 125, 0, 125, 100);
param[6] = new Param(this, "MINCELLSIZE", 0, 125,16, 400,  64); // width
param[7] = new Param(this, "MINRECTSIZE", 0, 125, 0, 128,  24);
param[8] = new Param(this, "THRESHOLD",   0, 125, 1, 128,  20);

// assign paras to midi[channel][function]
midi_action = new Array();    // channel
midi_action[0] = new Array(); // function
midi_action[0][2] = param[0];
midi_action[0][3] = param[1];
midi_action[0][4] = param[2];
midi_action[1] = new Array();
midi_action[1][2] = param[3];
midi_action[1][3] = param[4];
midi_action[1][4] = param[5];
midi_action[2] = new Array();
midi_action[2][2] = param[6];
midi_action[2][3] = param[7];
midi_action[2][4] = param[8];

// on screen display
osd = new TextLayer();
osd.print(" "); // BUG!
add_layer(osd);
osd.up();

colors = new Array(0xa03000ff, 0x006000ff, 0x336000ff, 0x00ff00ff, 0xc20000ff, 0x0000c0ff, 0xc0c0c0ff, 0x30f0f0ff, 0x00ff90ff);
ncolors = colors.length - 1;
//width=1024; height=768;
//width=400; height=300;
width=640; height=480;
width=400; height=300;

geo = new GeometryLayer(width, height);
//geo = new GeometryLayer(100,100);
add_layer(geo);

trigger = new TriggerController();
register_controller(trigger);
trigger.frame = function() { Draw(); };

kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }
kbd.released_x = function() { Draw(); }

parm_sel = 0;
kbd.pressed_up = function() {
    parm_sel = Math.min(param.length-1, parm_sel+1);
    param[parm_sel].print(osd);
}
kbd.pressed_down = function() {
    parm_sel = Math.max(0, parm_sel-1);
    param[parm_sel].print(osd);
}
kbd.pressed_left = function() {
    param[parm_sel].step(-1);
    param[parm_sel].print(osd);
}
kbd.pressed_right = function() {
    param[parm_sel].step(1);
    param[parm_sel].print(osd);
}
    

mc = new MidiController();
register_controller(mc);
ret = mc.connect_from(0, 20, 0);

mc.event_ctrl = function (ch, func, value) {
    var a = midi_action[ch];
    if (a)
        a = a[func];
    if (a) {
        a.setValue(value);
        a.print(osd);
    }
}

function Draw() {
    for (j=0; j<10; j++) {
        if ((iterations > 0) && (++i >= iterations)) {
            i = 0; // clearScreen
            geo.rectangle_fill(0,0,width,height,0x000000FF);
        } 
        paint();
    }
}

function paint() {
    var cellsWide, cellsHigh, cellWidth, cellHeight;
//debug ("paint");
    /* How many cells wide the grid is (equal to gridSize +/- (gridSize / 2))
     */
    cellsWide  = c_tweak(gridSize, gridSize / 2);
    /* How many cells high the grid is (equal to gridSize +/- (gridSize / 2))
     */
    cellsHigh  = c_tweak(gridSize, gridSize / 2);
    /* How wide each cell in the grid is */
    cellWidth  = width  / cellsWide;
    /* How tall each cell in the grid is */
    cellHeight = height / cellsHigh;

    /* Ensure that each cell is above a certain minimum size */

    if (cellWidth < MINCELLSIZE) {
        cellWidth  = MINCELLSIZE;
        cellsWide  = width / cellWidth;
    }

    if (cellHeight < MINCELLSIZE) {
        cellHeight = MINCELLSIZE;
        cellsHigh  = height / cellHeight;
    }
    /* fill the grid with randomly-generated cells */
    for(var i = 0; i < cellsHigh; i++) {
      /* Each row is a different color, randomly generated (but constrained) */
        var c = genNewColor();
        var fg_gc = colors[c];
//debug("pick color #" + c + " = " + fg_gc);
          //XSetForeground(dpy, fg_gc, colors[c].pixel);

        for(var j = 0; j < cellsWide; j++) {
            var curWidth, curHeight, curX, curY;

            /* Generate a random height for a rectangle and make sure that */
            /* it's above a certain minimum size */
            curHeight = random() % (cellHeight - shadowWidth);
            if (curHeight < MINRECTSIZE)
              curHeight = MINRECTSIZE;
            /* Generate a random width for a rectangle and make sure that
               it's above a certain minimum size */
            curWidth  = random() % (cellWidth  - shadowWidth);
            if (curWidth < MINRECTSIZE)
              curWidth = MINRECTSIZE;
            /* Figure out a random place to locate the rectangle within the
               cell */
            curY = (i * cellHeight) + (random() % ((cellHeight - curHeight) -
                                                        shadowWidth));
            curX = (j * cellWidth) +  (random() % ((cellWidth  - curWidth) -
                                                            shadowWidth));

            /* Draw the shadow */
            if (elevation > 0)
                geo.rectangle_fill(curX + elevation, curY + elevation,
                      curX + elevation + curWidth, 
                      curY + elevation + curHeight,
                      0x00000060);

            /* Draw the edge */
            if (shadowWidth > 0)
                geo.rectangle_fill(curX + shadowWidth, curY + shadowWidth,
                curX + shadowWidth + curWidth, 
                curY + shadowWidth + curHeight,
                0x000000ff);

            geo.rectangle_fill(curX, curY, curX+curWidth, curY+curHeight, fg_gc)

            /* Draw a 1-pixel black border around the rectangle */
            geo.rectangle(curX, curY, curX+curWidth, curY+curHeight, 0x00000000);
        }
    }
}

function random() {
    return (Math.random() * 0xffffffff).int();
}

function genConstrainedColor(base, tweak) {
    var i = 1 + (random() % tweak);
    if (random() & 1)
        i = -i;
    i = (base + i) % ncolors;
    while (i < 0)
        i += ncolors;
    return i;
}

function c_tweak(base, tweak) {
    var ranTweak = random() % (2 * tweak);
    var n = (base + (ranTweak - tweak));
    if (n < 0) n = -n;
    return Math.min(255, n);
}

function genNewColor() {
    /* These lines handle "sway", or the gradual random changing of */
    /* colors. After genNewColor() has been called a given number of */
    /* times (specified by a random permutation of the tweak variable), */
    /* take whatever color has been most recently randomly generated and */
    /* make it the new base color. */
    if (timeLeft == 0) {
        timeLeft = c_tweak(sway, sway / 3);
        curColor = curBase;
    } else {
        timeLeft--;
    }
     
    /* If a randomly generated number is less than the threshold value,
       produce a "sport" color value that is completely unrelated to the 
       current palette. */
    if (0 == (random() % THRESHOLD)) {
        return random() % ncolors;
    } else {
        curBase = genConstrainedColor(curColor, tweak);
        return curBase;
    }
}
