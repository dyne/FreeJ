// OSC controller interface
// buZz, Puik. started aug. 2009

// ment to do :
//  - widgets
//  - editing of interface?
//  - send OSC events on widget change

// startup with:
// # freej -cFj osc_controller_interface.js 

// eee fullscreen res ;)
W = 800;
H = 480;

include('widget_class.js');


scr = new Screen();
if (!scr.is_initialized()) {
    // screen hasn't been initialized yet, let's do it now
    scr.init(W,H);
    add_screen(scr);
}

MAX_WIDGETS = 10; // unused atm

widgets = new Array();

mousestate = 0;
plzredraw = 1;

geo = new GeometryLayer(W,H);
//geo.set_blit("ADD");
//geo.set_fps();
geo.color(255,255,255,255);
geo.activate(true);
scr.add_layer(geo);

kbd = new KeyboardController();
kbd.activate(true);
kbd.pressed_esc = function() { quit(); }
register_controller( kbd );

ms = new MouseController();
ms.activate(true);
ms.grab(true);
ms.button = function(b, s, x, y) {
	var i;
	// (button, state, x, y)
//    	echo("mousestate "+mousestate+" b"+b+" s"+s+" x"+x+" y"+y);
	this.grab(s);


	switch (mousestate) {
		// mousestate 0 = nothing
		// mousestate 1 = click
		// mousestate 2 = nothing + just clicked earlier (needs a timer)
		// mousestate 3 = double click
		// mousestate 4 = drag (not working right)
		case 0:
			if ((s == 1) && (b == 1)) {
				// left mousebutton pressed
				mousestate = 1;

				// do widget click change?
				for (i=0; i<widgets.length; i++) {
					if (widgets[i].intersects(x,y)) {
						widgets[i].change(b,mousestate);
						plzredraw = 1;
					}
				}

			} else {
				mousestate = 0;
			}
			break;
		case 1:
			if (s == 0) {
				// button released
				mousestate = 2;
			} else {
				// button held
				mousestate = 4;
			}
			break;
		case 2:
			if ((s == 1) && (b == 1)) {
				// button pressed
				mousestate = 3;
			} else {
				mousestate = 0;
			}
			break;
		case 3:
			// do widget doubleclick change ?
			for (i=0; i<widgets.length; i++) {

				if (widgets[i].intersects(x,y)) {
					widgets[i].change(b,mousestate); // this might need to be swapped with something else ...
					plzredraw = 1;
				}
			}
			mousestate = 0;
			break;
		case 4:
			// do widget dragchange!
			for (i=0; i<widgets.length; i++) {

				if (widgets[i].intersects(x,y)) {
					widgets[i].change(b,mousestate); // yeah, this is not right :D
					plzredraw = 1;
				}
			}
			mousestate = 0;
			break;
	}

}

ms.motion = function(b, x, y, dx, dy) {

	// dno if i need anything with motion ...

	// (buttonmask, x, y, xrel, yrel)
//	echo("b"+b+" x"+x+" y"+y+" dx"+dx+" dy"+dy);
	// geo.pixel(x,y);
}

register_controller( ms );

// this just sets up a small scene with two toggle buttons on the left, and one bang button on the right

widgets[0] = new Widget("toggle1",10,10,100,100,2);
widgets[0].data = 1;
widgets[1] = new Widget("toggle2",200,10,100,100,2);
widgets[1].data = 0;
widgets[2] = new Widget("bang1",400,10,100,100,1);
widgets[2].data = 0;
plzredraw = 1;

var loopbang;
loopbang = new TriggerController();
register_controller(loopbang);
loopbang.frame = function() {

	// mainloops should be like this :P

	if (plzredraw) {
		for (i=0; i<widgets.length; i++) widgets[i].draw(geo);
		plzredraw = 0;
	}

}

