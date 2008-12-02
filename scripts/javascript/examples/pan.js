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


include("keyboard.js");

w=400; h=300;

tile_x=128;
tile_y=128;

max_m=Math.ceil(w/tile_x) * 2;
max_n=Math.ceil(h/tile_y) * 2;

imgs = new Array();
for (m=0; m<max_m; m++) {
	imgs[m] = new Array()
	for (n=0; n<max_n; n++) {
		i = new ImageLayer(tile_x,tile_y);
		i.open("1steps063.jpg");
		add_layer(i);
		i.set_position(tile_x*m, tile_y*n);
		imgs[m][n] = i;
	}
}

run(1);


dx=dy=0;
bx = max_m * tile_x / 2;
by = max_n * tile_y / 2;
while (1) {
	for (m=0; m<max_m; m++) {
		for (n=0; n<max_n; n++) {
			i = imgs[m][n];
			i.set_position ( tile_x*m - dx, tile_y*n - dy);
		}
	}
	run(0.01);
	dx++;
	dy++;
	dx = dx % bx
	dy = dy % by;
}

