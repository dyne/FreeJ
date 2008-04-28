// sample script for wiimote controller


wii = new WiiController();
register_controller(wii);

geo = new GeometryLayer();
geo.activate(true);
geo.start();
geo.set_fps(25);
add_layer(geo);

kbd = new KeyboardController();
register_controller(kbd);

kbd.released_q = function() { quit(); }

kbd.released_1 = function() { wii.toggle_led(1); }
kbd.released_2 = function() { wii.toggle_led(2); }
kbd.released_3 = function() { wii.toggle_led(3); }

echo("connecting wii");
wii.connect();

//register_controller(wii);

wii.acceleration = function(x,y,z) {
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
butts = new Array("button_1", "button_2");

for (i in butts) {
	wii[butts[i]] = function() {
		butt(butts[i]);
	}
}

echo("wii script finish");

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
