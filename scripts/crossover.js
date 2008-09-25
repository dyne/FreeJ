// crossover J & J
// from cellsbutton02 yogyakarta performance
// -jrml aug2008

W=640
H=480

set_resolution(W,H);

audio = new AudioJack("alsa_pcm:capture_1", 2048, 44100);


geo = new GeometryLayer();
geo.activate(true);
geo.start();
geo.set_fps();
add_layer(geo);

shift=0;
draw=0;
mode=0;

kbd = new KeyboardController();
register_controller(kbd);

linedraw = function(x0,y0,x1,y1) {
    if(draw==0) geo.line(x0,y0,x1,y1);
    else if(draw==1) geo.aaline(x0,y0,x1,y1);
}


kbd.pressed_p = function() { shift++; }
kbd.pressed_o = function() { shift--; }

kbd.released_a = function() { draw=1; };
kbd.released_s = function() { draw=0; };

kbd.released_b = function() {
    if(mode==1) mode=0;
    else mode=1;
};

kbd.released_q = function() { quit(); }
kbd.released_r = function() {
    if(reset("crossover.js")) {
	rem_controller(this);
	echo("reset ok");
    }
    return true;
};
    
center_x = W/2;
center_y = H/2;

crossfat = 10;

smallcross = function(x,y,len) {
    linedraw(x-len,y-len,x+len,y+len);
    linedraw(x-len,y+len,x+len,y-len);
};

wave = new Array();

bang = new TriggerController();
bang.frame = function() {

    audio.fft();

    geo.color(0,0,0,50);
    geo.rectangle_fill(0,0,W,H);

    geo.color( 0xff, 0xff, 0xff, 0xff );

    if(mode==1) { // small crosses

        hc1 = audio.get_harmonic(2);
	hc2 = audio.get_harmonic(12);

	crossfat = hc2*16;
	if(crossfat<16) crossfat=16;

	crosswide = hc1*8;

	wave.unshift(crosswide);
	if(wave.length < W/16) return;
	/*
	  echo("crossfat = " + crossfat);
	  echo("diagonals = " + diago1 + " x " + diago2); 
	*/
	for(xx=0; xx<W/crossfat; xx++) {
	    
	    for(yy=0; yy<H/crossfat; yy++) {

		smallcross(center_x+(xx*crossfat),
			   center_y+(yy*crossfat),
			   wave[xx]);

		smallcross(center_x-(xx*crossfat),
			   center_y-(yy*crossfat),
			   wave[xx]);

		smallcross(center_x+(xx*crossfat),
			   center_y-(yy*crossfat),
			   wave[xx]);

		smallcross(center_x-(xx*crossfat),
			   center_y+(yy*crossfat),
			   wave[xx]);

	    }
	}
	
	wave.pop();

    } else {	  // wired cross
	
	for(c=0;c<16;c++) {
	    
	    hc = (audio.get_harmonic(c) * 8) + shift*2;
	    
	    p = c*8;
	    
	    xphc = center_x+hc;
	    xmhc = center_x-hc;
	    xpp  = center_x+p;
	    xmp  = center_x-p;
	    
	    yphc = center_y+hc;
	    ymhc = center_y-hc;
	    ypp  = center_y+p;
	    ymp  = center_y-p;
	    
	    linedraw(xpp, ypp, xphc, yphc);
	    linedraw(xpp, ypp, xmhc, ymhc);
	    linedraw(xpp, ypp, xphc, ymhc);
	    linedraw(xpp, ypp, xmhc, yphc);
	    
	    linedraw(xmp, ymp, xphc, yphc);
	    linedraw(xmp, ymp, xmhc, ymhc);
	    linedraw(xmp, ymp, xphc, ymhc);
	    linedraw(xmp, ymp, xmhc, yphc);
	    
	    linedraw(xmp, ypp, xphc, yphc);
	    linedraw(xmp, ypp, xmhc, ymhc);
	    linedraw(xmp, ypp, xphc, ymhc);
	    linedraw(xmp, ypp, xmhc, yphc);
	    
	    linedraw(xpp, ymp, xphc, yphc);
	    linedraw(xpp, ymp, xmhc, ymhc);
	    linedraw(xpp, ymp, xphc, ymhc);
	    linedraw(xpp, ymp, xmhc, yphc);
	
	}
    }
}
register_controller(bang);

gc();
