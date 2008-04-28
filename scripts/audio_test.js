// simple audio test reading out harmonics
// make sure jack is running and have a look at the connections
// also check the values (samplesize and samplerate) to match your conf
// -jrml

H=640;
M=H/128;
W=H/(H/80);

set_resolution(W,H);

audio = new AudioJack("xine:out_l", 2048, 44100);

geo = new GeometryLayer();
geo.activate(true);
geo.start();
geo.set_fps();
add_layer(geo);

kbd = new KeyboardController();
register_controller(kbd);
kbd.released_q = function() { quit(); }
kbd.released_r = function() {
    if(reset("audio_test.js")) {
	rem_controller(this);
	echo("reset ok");
    }
    return true;
}

bang = new TriggerController();
bang.frame = function() {

    audio.fft();

    geo.color(0,0,0,50);
    geo.rectangle_fill(0,0,W,H);

    flash = 0;

    for(c=0;c<16;c++) {
	
	hc = audio.get_harmonic(c);

	if(hc>H) hc = H-1; // boundary

	if(hc>(H/2)) flash=hc; // flash 

	geo.color( 0xff, 0xff, 0xff, 0xff );

	geo.vline( c*M, H, H-( hc ));
	geo.vline( (c*M)+1, H, H-( hc ));

	geo.vline( (c*M)+2, 0, hc);
	geo.vline( (c*M)+3, 0, hc);
    }
	   
    if(flash >0) {
	geo.color(0xff,0xff,0xff, 200);
	geo.ellipse_fill(W/2, H/2, (W/2)-10, hc/2);
    }

}
register_controller(bang);

gc();