// simple audio test reading out harmonics
// make sure jack is running and have a look at the connections
// also check the values (samplesize and samplerate) to match your conf
// -jrml

//set_resolution(W,H);

audio = new AudioJack("xine/out_1", 2048, 44100);

img = new ImageLayer();
img.open("../doc/ipernav.png");
img.activate(true);
img.set_fps();
img.start();
add_layer(img);
vtg = new Filter("vertigo");
img.add_filter(vtg);

kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }
kbd.released_r = function() {
    if(reset("freej_equalizer.js")) {
        rem_controller(this);
        echo("reset ok");
    }
    return true;
}

bang = new TriggerController();
bang.frame = function() {

    audio.fft();

    flash = 0;
    phase = (((audio.get_harmonic(8)
           + audio.get_harmonic(1) )
	     / 2 )
	      * 3 ) / 128;
    if(phase<1) phase = 0;
    zoom = (((audio.get_harmonic(8)
           + audio.get_harmonic(16) )
	     / 2 )
	      * 3 ) / 64;
    zoom = (audio.get_harmonic(16)*3)/64;
    if(zoom<1) zoom = 1;
    vtg.set_parameter("PhaseIncrement", phase);
    vtg.set_parameter("ZoomRate", zoom);
}


register_controller(bang);

gc();
