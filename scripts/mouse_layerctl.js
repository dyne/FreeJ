m = new MouseController();
register_controller(m);

m.zoomx = 1; m.zoomy = 1; m.maxzoom=4;

// MouseController.button(button, state, x, y)
m.button = function(b, s, x, y) {
	echo("b"+b+" s"+s+" x"+x+" y"+y);
	this.grab(s);

	if (s == 1)
		this.l=selected_layer();
	else
		delete this.l;
	
	return true;
}

// MouseController.motion(buttonmask, x, y, xrel, yrel)
m.motion = function(b, x, y, dx, dy) {
	echo("b"+b+" x"+x+" y"+y+" dx"+dx+" dy"+dy);
	
	var l = this.l
	if (!l)
		return false;
	
	if (b & 1) {
		l.set_position(l.x() + dx, l.y() + dy);
	}
	if (b & 2) {
	}
	if (b & 4) {
		this.zoomx += dx/100; 
		this.zoomy += dy/100;
		if (Math.abs(this.zoomx)>this.maxzoom)
			this.zoomx=this.maxzoom;
		if (Math.abs(this.zoomy)>this.maxzoom)
			this.zoomy=this.maxzoom;
		l.zoom(this.zoomx, this.zoomy);
	}
}
