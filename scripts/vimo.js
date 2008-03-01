// video mouse test script

echo("VideoMouse");
vm = new ViMoController("/dev/ttyUSB0");
register_controller(vm);

// inner wheel
vm.wheel_i = function(dir, hist) {
	var msg = "wi "+dir+" "+hist;
	echo(msg);
}

// outer wheel
vm.wheel_o = function(s, so) { 
	echo("speed: " + s + " / " + so);
}

// ye butts
vm.button = function(button, state, mask, mask_old) {
	echo("VM buttom: " + button + " " + state + " " + mask + "/" + mask_old);
}
