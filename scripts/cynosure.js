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

curColor    = 0;
curBase     = curColor;
shadowWidth = 2 ; //get_integer_resource ("shadowWidth", "Integer");
elevation   = 10; //get_integer_resource ("elevation", "Integer");
sway        = 30; //get_integer_resource ("sway", "Integer");
tweak       = 20; //get_integer_resource ("tweak", "Integer");
gridSize    = 10; //get_integer_resource ("gridSize", "Integer");
timeLeft    = 0;
ncolors     = 8; //get_integer_resource ("colors", "Colors");
iterations  = 100; //get_integer_resource ("iterations", "Iterations");
MINCELLSIZE = 64;
MINRECTSIZE = 24;
THRESHOLD = 100;

i = 0;

colors = new Array(0xa03000ff, 0x006000ff, 0x336000ff, 0x00ff00ff, 0xc20000ff, 0x0000c0ff, 0xc0c0c0ff, 0x30f0f0ff, 0x00ff90ff);
width=200; height=600;
width=1024; height=768;
width=800; height=800;
width=400; height=300;

geo = new GeometryLayer();
add_layer(geo);

trigger = new TriggerController();
register_controller(trigger);
trigger.frame = function() { Draw(); };

kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }
kbd.released_x = function() { Draw(); }

kbd.released_p = function() {
    echo("ok, I shot myself now");
    try{rem_controller(trigger);}catch(e){echo("tg nope: "+e);}
    try{rem_layer(geo);}catch(e){echo("la nope: "+e);}
    try{rem_controller(mc);}catch(e){echo("mc nope: "+e);}
    try{rem_controller(kbd);}catch(e){echo("kb nope: "+e);}
    delete geo;
    delete mc;
    delete kbd;
    delete trigger;
    gc(); // kbd-js not cleared within this call!
    echo("Bullet arrived.");
}


mc = new MidiController(); // ('descr')
register_controller(mc);
ret = mc.connect_from(0, 20, 0);

mc.event_ctrl = function (ch, func, value) {
    switch (ch) {
        case 0:
            switch (func) {
                case 2:
                    shadowWidth = value;
                    break;
                case 3:
                    elevation = value;
                    break;
                case 4:
                    sway = value;
                    break;
            }
            break;
        case 1:
            switch (func) {
                case 2:
                    tweak = value+1;
                    break;
                case 3:
                    gridSize = value/4 +1;
                    break;
                case 4:
                    iterations = value*2;
            }
            break;
        case 2:
            switch (func) {
                case 2:
                    MINRECTSIZE = value;
                    break;
                case 3:
                    MINCELLSIZE = value;
                    break;
                case 4:
                    THRESHOLD = value;
                    break;
            }
    }
}

function integer(i) {
	if (i>0) 
		return Math.floor(i)
	else
		return Math.ceil(i)
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
      cellsHigh  = width / cellWidth;
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
	return integer(Math.random() * 0xffffffff);
}


function genConstrainedColor(base, tweak) {
    var i = integer(1 + (random() % tweak));
    if (random() & 1)
      i = -i;
    i = (base + i) % ncolors;
    while (i < 0)
      i += ncolors;
    return integer(i);
}

function c_tweak(base, tweak) {
    var ranTweak = integer(random() % (2 * tweak));
    var n = (base + (ranTweak - tweak));
    if (n < 0) n = -n;
    return integer(n < 255 ? n : 255);
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
      return integer(random() % ncolors);
    } else {
      curBase = genConstrainedColor(curColor, tweak);
      return integer(curBase);
    }

}


echo("cyno script end");
