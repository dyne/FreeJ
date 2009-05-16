
geo = new GeometryLayer();
geo.activate(true);
geo.color(255,255,255,255);
add_layer(geo);


m = new MouseController();
m.activate(true);
// MouseController.button(button, state, x, y)
m.button = function(b, s, x, y) {
    //	echo("b"+b+" s"+s+" x"+x+" y"+y);
	this.grab(s);
}

// MouseController.motion(buttonmask, x, y, xrel, yrel)
m.motion = function(b, x, y, dx, dy) {
    geo.pixel(x,y);
    //    geo.ellipse_fill(x,y,dx,dy);
    //    echo("b"+b+" x"+x+" y"+y+" dx"+dx+" dy"+dy);
}

register_controller(m);
