m = new MouseController();
register_controller(m);

// MouseController.button(button, state, x, y)
m.button = function(b, s, x, y) {
	echo("b"+b+" s"+s+" x"+x+" y"+y);
	this.grab(s);
}

// MouseController.motion(buttonmask, x, y, xrel, yrel)
m.motion = function(b, x, y, dx, dy) {
	echo("b"+b+" x"+x+" y"+y+" dx"+dx+" dy"+dy);
}
