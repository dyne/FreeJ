// simple javascript executable with FreeJ

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
}

fastsrand();

for( c=0 ; c<10 ; c++ ) {
  numba = fastrand();
  msg = "random number " + c + ": " + numba;
  scroll.append(msg);
  cafudda(2);
}

if(text) {
    add_layer(text);
    text.set_blit("absdiff");
    text.font(5);
    text.size(40);
    for( c=0; c<10; c++) {
       text.font(c+1);
       numba = fastrand();
       msg = c + ": " + numba;
       text.print(msg);
       cafudda(2);
    }
} 

//if(text) add_layer(text);
//text.set_blit("alpha");
//text.font(5);
//text.set_blit_value(100);
//text.blink();

//lay = new Layer("sub.swf");
//if(lay) add_layer(lay);





//cafudda(50);
quit();

