#!/usr/local/bin/freej -j
// FreeJ - http://freej.dyne.org

// this is a simple script example, executable with FreeJ
// doesn't needs any external data, it's all runtime rendered
// feel free to experiment from this small script,
// much more things can be done ;)

particles = new ParticleLayer();
text = new TextLayer();
scroll = new VScrollLayer();

if(particles) {
    particles.set_blit("alpha");
    add_layer(particles);
    particles.blossom(0); particles.blossom(0);
    particles.set_blit_value(0.4);
}

if(scroll) {
    scroll.speed(1);
    scroll.set_blit("alpha");
    add_layer(scroll);
    scroll.set_blit_value(0.7);
    scroll.append("hello world!");
    cafudda(1);
    scroll.append("this is a short freej script example");
    cafudda(1);
    scroll.append("now we start randomizing everything..");
    cafudda(2);
}

srand();

for( c=0 ; c<100 ; c+=10 ) {
    numba = rand(c); // generate random numbers <c
  msg = "random number " + (c/10) + ": " + numba;
  msg += " (max: " + c + ")";
  scroll.append(msg); // append to the vertical scroller text
  cafudda(2);
}

particles.blossom(1);

cafudda(1);

if(text) {
    add_layer(text);
    text.set_blit("absdiff");
    text.size(40);
    text.spin(1.05, 0.2);
    for( c=0; c<10; c++) {
       //text.font(c+1);
       text.font("/usr/share/fonts/truetype/freefont/FreeSans.ttf");
       numba = rand(c*10); // generate random numbers <(c*10)
       msg = c + ": " + numba;
       text.print(msg); // print it in truetype fonts on text layer
       cafudda(2);
    }
    particles.blossom(0);
    text.size(30);
    text.print("that's all folks!");
    cafudda(10); 
} 

quit();

