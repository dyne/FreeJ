#!/usr/local/bin/freej -j
// freej script example by Mr.Goil
// this one can tile a seamless image and pan a plane with it
// $Id: $

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

