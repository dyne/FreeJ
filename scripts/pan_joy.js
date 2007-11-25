#!/usr/local/bin/freej -j
/*  FreeJ example scripts
 *  (c) Copyright 2007 Christoph Rudorff aka MrGoil <goil@dyne.org>
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
// freej script example by Mr.Goil
// this one can tile a seamless image and pan a plane with it
// $Id: $

w=800; h=600;
set_resolution(w, h);

tile_x=128;
tile_y=128;

max_m=Math.ceil(w/tile_x) * 2;
max_n=Math.ceil(h/tile_y) * 2;

curr_img=0;
//images = scandir("./*.jpg");
images  = new Array("1steps063.jpg");

imgs = null;
function init() {
    imgs = new Array();
    for (m=0; m<max_m; m++) {
        imgs[m] = new Array();
        for (n=0; n<max_n; n++) {
            try {
                i = new ImageLayer(tile_x,tile_y);
                i.open(images[curr_img]);
                add_layer(i);
                i.set_position(tile_x*m, tile_y*n);
                i.activate();
                imgs[m][n] = i;
                echo ("size " + i.get_width() + "x" + i.get_height() );
            } catch (e) {
                echo("Failed to open "  + e);
                //echo("Failed to open " + images[curr_img] + ": " + e);
                if (++curr_img<images.length-1)
                    continue;
            }
        }
    }
}
init();

try {
    jc = new JoystickController();
    register_controller(jc);
    jc.axismotion = function (which, axis, value) {
        value = value/100000; //equilaze
        if(!last_js[which]) {
            last_js[which] = new Array();
        }
        last_js[which][axis] = value;
    }

    jc.button = function (which, button, state) {
        if(state==1)
            switch (button) {
                case 6:
                    prev_img();
                break;

                case 7:
                    next_img();
                break;
        }
    }
} catch (e) {
    echo("mh, no joystick?! " + e);
    quit();
}

last_js = new Array();

function axismotion(which, axis, value) {
// mh, on macbookporn, the hd joystick is funny ... and must be inverted
//    if (which==0) return;
    if (which==0) value=-value;
    switch(axis) {
        case 0:
            sx -= value;
            break;
        case 1:
            sy -= value;
            break;
/*        case 3:
            g=data[1];
            g.sx += value;
            break;
        case 4:
            g=data[1];
            g.sy += value;
            break;
*/
    }
    max_speed();
}
max_s=100;
function max_speed () {
    if (sx > max_s) sx=max_s;
    if (sx < -max_s) sx=-max_s;
    if (sy > max_s) sy=max_s;
    if (sy < -max_s) sy=-max_s;
}

kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }
kbd.released_a = next_img;
kbd.released_z = prev_img;

function next_img() {
    //curr_img++;
    if (++curr_img>images.length-1)
        curr_img=0;
    //init();
    clean_up();
}

function prev_img () {
    //curr_img++;
    if (--curr_img<0)
        curr_img=images.length-1;
    //init();
    clean_up();
}

function clean_up() {
	for (m=0; m<max_m; m++) {
		for (n=0; n<max_n; n++) {
			var i = imgs[m][n];
            i.open(images[curr_img]);
        }
    }
}

dx=dy=0;
sx=sy=1;
bx = max_m * tile_x / 2;
by = max_n * tile_y / 2;

tr=new TriggerController();
register_controller(tr);
tr.frame = function() {
	for (m=0; m<max_m; m++) {
		for (n=0; n<max_n; n++) {
			i = imgs[m][n];
			i.set_position ( tile_x*m - dx, tile_y*n - dy);
//echo ( "set " + m  + "," + n + "to" + (tile_x*m - dx) + " " + (tile_y*n - dy) );
		}
	}
	dx += Math.floor(sx);
	dy += Math.floor(sy);
	dx = dx % bx
	dy = dy % by;
	if (dx < 0)
		dx += bx;
	if (dy < 0)
		dy += by;
    for (which=0; which <= last_js.length; which++) {
//echo("whch " + which);
        if (last_js[which]) {
            for (axis=0; axis <= (last_js[which]).length; axis++) {
//echo("axis " + axis);
                value = last_js[which][axis];
                if ((value) && (value !=0)) {
//echo("val " + last_js[which][axis]);
                       axismotion(which, axis, value); 
               }
            }
        }
    }
}


init();
