#!/usr/local/bin/freej
// FreeJ - http://freej.dyne.org

// this is a simple script example, executable with FreeJ
// doesn't needs any external data, it's all runtime rendered
// feel free to experiment from this small script,
// much more things can be done ;)

particles = new ParticleLayer();
text = new TxtLayer();
scroll = new VScrollLayer();

if(particles) {
    particles.set_blit("alpha");
    add_layer(particles);
    particles.blossom(0); particles.blossom(0);
    particles.set_blit_value(80);
}

if(scroll) {
    scroll.speed(1);
    scroll.set_blit("alpha");
    add_layer(scroll);
    scroll.set_blit_value(150);
    scroll.append("hello world!");
    cafudda(1);
    scroll.append("this is a short freej script example");
    cafudda(1);
    scroll.append("now we start randomizing everything..");
    cafudda(2);
}

fastsrand();

for( c=0 ; c<10 ; c++ ) {
  numba = fastrand();
  msg = "random number " + c + ": " + numba;
  scroll.append(msg);
  cafudda(2);
}

particles.blossom(1);

cafudda(1);

if(text) {
    add_layer(text);
    text.set_blit("absdiff");
    text.size(40);
    for( c=0; c<10; c++) {
       text.font(c+1);
       numba = fastrand();
       msg = c + ": " + numba;
       text.print(msg);
       cafudda(2);
    }
    particles.blossom(0);
    text.size(30);
    text.print("that's all folks!");
    cafudda(10); 
} 

quit();

