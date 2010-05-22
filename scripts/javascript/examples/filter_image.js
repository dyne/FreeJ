// simple script to test filter functionalities

img = new ImageLayer();
img.open("doc/ipernav.png");
img.activate(true);
img.start();
add_layer(img);
filt = new Filter("Distort0r");
img.add_filter(filt);

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

    a = 1/(rand()%200);
    b = 1/(rand()%200);
    filt.set_parameter("Frequency", a);
    filt.set_parameter("Amplitude", b);
}


register_controller(bang);

