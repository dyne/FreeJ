// freej example script by jaromil
// this simply draws i kind of star
// it also shows the usage of keyboard controller
// press 'q' to quit while running

W=800
H=480

set_resolution(W,H);

// the Param class is stabilizing values between some limits
// this can be used also to adapt the calibration at realtime...
// so far a simple boundary is set on width/height on screen.

include("param.js");

param = new Array();
//                         name,  min_in,  max_in, min_out, max_out, default
param[0] = new Param(this, "px",  100,     150,    0,       W,       W/2);
param[1] = new Param(this, "py",  100,     150,    0,       H,       H/2);
param[2] = new Param(this, "pz",  100,     150,    0,       H,       H/2);



wii = new WiiController();

px = 0; py = 0; pz = 0;
x = 0; y = 0; z = 0;

wii.acceleration = function(ax,ay,az) {
    if(px != ax) {
	splash = true;
	param[0].setValue(ax);
	param[1].setValue(ay);
	param[2].setValue(az);
	x = param[0].out_value;
	y = param[1].out_value;
	z = param[2].out_value;
    	px = ax;
    }
}

if(wii.connect())
    register_controller(wii);

// wii.toggle_accel();


kbd = new KeyboardController();
kbd.pressed_esc = function() { quit(); }
kbd.released_q = function() { quit(); }
register_controller( kbd );

geo = new GeometryLayer();
geo.color(255,255,255,255);
geo.set_blit("alpha");
geo.set_blit_value(0.2);
geo.set_fps();
geo.activate(true);
add_layer(geo);

//drawStar(geo,30,1);

srand();

framec = 24;

bang = new TriggerController();
register_controller(bang);
bang.frame = function() {
    if(framec>0) {
	framec--;
    } else
	if(splash) {
	// paint on geo
	    geo.color(rand()%255, rand()%255, rand()%255, 0xff);
	    geo.ellipse_fill(x,y,z,y);
	    splash = false;
	}
}
