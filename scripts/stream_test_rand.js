// freej script to test randomness
// this also shows keyboard interactivity
// and smart use of function pointers
// (C)2005 Denis Jaromil Rojo - GNU GPL 

width =  400;
height = 300;
drawer = draw_triangles;

function draw_pixels(geo) {
  var x, y;

  x = rand()%width;
  y = rand()%height;

  if(x<0) x = -x;
  if(y<0) y = -y;

  geo.pixel(x,y);
}


function draw_triangles(geo) {
  var x1, x2, x3;
  var y1, y2, y3;

  x1 = rand()%width;
  x2 = rand()%width;
  x3 = rand()%width;
  y1 = rand()%height;
  y2 = rand()%height;
  y3 = rand()%height;

  if(x1<0) x1 = -x1;
  if(x2<0) x2 = -x2;
  if(x3<0) x3 = -x3;
  if(y1<0) y1 = -y1;
  if(y2<0) y2 = -y2;
  if(y3<0) y3 = -y3;

  geo.trigon_fill(x1,y1,x2,y2,x3,y3);
}


function draw_ellipses(geo) {
  var x, y;
  var rx, ry;

  x = rand()%width;
  y = rand()%height;
  rx = rand()%(width/2);
  ry = rand()%(height/2);

  if(x<0) x = -x;
  if(y<0) y = -y;
  if(rx<0) rx = -rx;
  if(ry<0) ry = -ry;

  geo.ellipse_fill(x, y, rx, ry);
}

function randomize_color(geo) {
  var r, g, b;

  r = rand() % 255;
  g = rand() % 255;
  b = rand() % 255;

  if(r<0) r = -r;
  if(g<0) g = -g;
  if(b<0) b = -b;

  geo.color(r,g,b,150); 
}

geo = new GeometryLayer(400,300);
geo.set_blit("alpha");
geo.set_blit_value(0.5);
add_layer(geo);

txt = new TextLayer();
txt.print("FreeJ 0.9 test stream");
txt.set_position(20,10);
txt.set_blit("add");
add_layer(txt);

running = true;
kbd = new KeyboardController();
kbd.released_q = function() { running = false; }
kbd.released_p = function() { drawer = draw_pixels; }
kbd.released_t = function() { drawer = draw_triangles; }
kbd.released_e = function() { drawer = draw_ellipses; }
register_controller( kbd );

// create a video encoder object
//    values 1-100         video quality  video bitrate  audio quality  audio_bitrate
encoder = new VideoEncoder(10,             128000,        0,             32000);


encoder.stream_host("giss.tv");
encoder.stream_port(8000);
encoder.stream_title("testing new freej");
encoder.stream_username("source");
encoder.stream_password("2t645");
encoder.stream_mountpoint("freej-test.ogg");
/*
encoder.stream_host("10.66.66.83");
encoder.stream_port(8000);
encoder.stream_title("testing new freej");
encoder.stream_username("source");
encoder.stream_password("hackme");
encoder.stream_mountpoint("freej-test.ogg");
*/

register_encoder(encoder);
encoder.start_stream();
//encoder.start_filesave("prova.ogg");

while(running) {

  randomize_color(geo);

  drawer(geo);

  run(0.001);

}

quit();


