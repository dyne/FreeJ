// freej example script by jaromil
// this simply draws i kind of star
// it also shows the usage of keyboard controller
// press 'q' to quit while running

x = 100;
y = 100;
s = 30 / 0.383;
s2 = s;

PI = 3.141592654;
c = PI * 2;
o = -PI / 2;

function drawStar(lay, s_mul, s2_mul) {
    s = s_mul / 0.383;
    s2 = s / s2_mul;

    var cx;
    for(cx=0; cx<10; cx++) {
	k = cx/10;
	kn = k+0.1;
	
	x1 =   x+s * Math.cos(o + k*c);
	y1 =   y+s * Math.sin(o + k*c);
	x2 =   x+s2 * Math.cos(o + kn*c);
	y2 =   y+s2 * Math.sin(o + kn*c);
 	x1 = Math.floor(x1);
 	y1 = Math.floor(y1);
 	x2 = Math.floor(x2);
 	y2 = Math.floor(y2);

//	debug("drawing star line:" + x1 + "," + y1 + " " + x2 + "," + y2);
// 	lay.line( x.toPrecision()+s.toPrecision() * Math.cos(o + k*c),
// 		  y+s * Math.sin(o + k*c),
// 		  x+s2 * Math.cos(o + kn*c),
// 		  y+s2 * Math.sin(o + kn*c)  );
	lay.aaline(x1, y1, x2, y2);
    }
}

running = true;
kbd = new KeyboardController();
kbd.pressed_esc = function() { running = false; }
register_controller( kbd );

geo = new GeometryLayer();
geo.color(255,255,255,255);
geo.set_blit("alpha");
geo.set_blit_value(0.1);
geo.start();
geo.activate(true);
add_layer(geo);

drawStar(geo,30,1);

srand();

cc = 1;

while(running) {
    cc += 0.05;

    geo.color(rand()%255, rand()%255, rand()%255, 0xff);
    drawStar(geo, 30, Math.sin(cc));

    if(cc>4.0) {
	cc = 1;
	srand();
    }

    run(0.01);
}

quit();
