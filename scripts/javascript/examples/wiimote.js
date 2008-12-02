// sample script for wiimote controller



kbd = new KeyboardController();
register_controller(kbd);

kbd.released_q = function() { quit(); }

kbd.released_1 = function() { wii.toggle_led(1); }
kbd.released_2 = function() { wii.toggle_led(2); }
kbd.released_3 = function() { wii.toggle_led(3); }
kbd.released_4 = function() { wii.toggle_led(4); }

wii = new WiiController();

wii.geo = new GeometryLayer(300,100);
wii.geo.set_position(0,0);
add_layer(wii.geo);

echo("connecting wii");
wii.connect();

register_controller(wii);

wii.acceleration = function(x,y,z) {
	draw_accel(this.geo,x,y,z);
}

wii.toggle_accel(true);

function draw_accel(geo, x,y,z) {
    //    echo("accel x" + x + " y " + y + " z " + z);
    geo.color(0,0,0,0);
    geo.clear();
    //    geo.rectangle_fill(0,0,300,300);
    geo.color(255,0,0);
    geo.hline(0,x*2,10);
    geo.hline(0,x*2,11);
    geo.hline(0,x*2,12);
    
    geo.color(0,255,0);
    geo.hline(0,y*2,20);
    geo.hline(0,y*2,21);
    geo.hline(0,y*2,22);
    
    geo.color(0,0,255);
    geo.hline(0,z*2,30);
    geo.hline(0,z*2,31);
    geo.hline(0,z*2,32);
}

function butt(b) {
	echo("wii button " + b);
}

// echo("wii script finish 1");

// wii2 = new WiiController();
// echo("connecting wii2");
// wii2.connect("00:19:1D:66:91:D3");

// echo("connected wii2");
// controllers.add(wii2);
// wii2.toggle_accel(true);
// wii2.toggle_buttons(true);

// wii2.acceleration = function(x,y,z) {
// 	draw_accel(this.geo,x,y,z);
// }

// wii2.button = function(button_no, state, mask, old_mask) {
// 	echo("wii2 button " + button_no + " " + state + " " + mask + " " + old_mask);
// }

// wii2.geo = new GeometryLayer(300,100);
// wii2.geo.set_position(0,100);
// layers.add(wii2.geo);


function wiiup(w) {
	w.active(true);
	w.toggle_accel(true);
	w.toggle_buttons(true);
}

//echo("wii2 script finish 2");
/*
   JSCall("button_1", 0, NULL, &res);
   JSCall("button_2",  0, NULL, &res);
   JSCall("button_A", 0, NULL, &res);
   JSCall("button_B", 0, NULL, &res);
   JSCall("button_minus", 0, NULL, &res);
   JSCall("button_plus", 0, NULL, &res);
   JSCall("button_home", 0, NULL, &res);
   JSCall("button_left", 0, NULL, &res);
   JSCall("button_right", 0, NULL, &res);
   JSCall("button_up", 0, NULL, &res);
   JSCall("button_down", 0, NULL, &res);
*/

/*
   JSCall("acceleration", 3, "uuu",
           input->accel.x, input->accel.y, input->accel.z );
*/

/*
[F] JSCall calling method released_w()
[*] WII test
[F] 592:context_js.cpp:include_javascript
[F] 294:jsparser.cpp:open
[*] int JsParser::open(JSContext*, JSObject*, const char*) eval: 0x1023e3c8
[F] 52:wiimote_ctrl.cpp:js_wii_ctrl_constructor
[F] Controller::Controller() this=0x10296808
[*] WiiMote controller attached
[*] connecting wii
[F] 81:wiimote_ctrl.cpp:js_wii_ctrl_connect argc: 0
 .  Detecting WiiMote (press A+B on it to handshake)  << inv. in console
Wiimote: Connecting to 00:00:00:00:00:00...
Wiimote: Allocated command socket 5.

HANG HERE

Wiimote: Command connection failed (-1).
Wiimote: init
Wiimote: Request STATUS
Wiimote: Command write failed: -1 (expected 3)
Wiimote: Set MODE 0x37 CONTINUOUS
Wiimote: Command write failed: -1 (expected 4)
Wiimote: Turning ON IR
Wiimote: Command write failed: -1 (expected 3)
Wiimote: Set LEDs 0x60
Wiimote: Command write failed: -1 (expected 3)
[*] connected wii
[F] 185:context_js.cpp:register_controller
[F] 410:context.cpp:register_controller
 .  registered WiiCtrl controller
[F] JSvalcmp: 0x10262a48 / (nil)
[*] wii script finish
[F] open evalres: 1
[F] JS: include scripts/wiimote.js
Wiimote: error receiving report: -1 (max 32)
[!] error processing wiimote reports
Wiimote: error receiving report: -1 (max 32)
.....




*/
