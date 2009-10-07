// freej script to test randomness
// this also shows keyboard interactivity
// and smart use of function pointers
// (C)2005 Denis Jaromil Rojo - GNU GPL 

// W = get_width();
// H = get_height();
W = 400;
H = 300;

scr = new Screen("sdl");
scr.init(W,H);
add_screen(scr);

function draw_pixels(rand_geo) {
  var x, y;

  x = rand()%W;
  y = rand()%H;

  if(x<0) x = -x;
  if(y<0) y = -y;

  rand_geo.pixel(x,y);
}


function draw_triangles(rand_geo) {
  var x1, x2, x3;
  var y1, y2, y3;

  x1 = rand()%W;
  x2 = rand()%W;
  x3 = rand()%W;
  y1 = rand()%H;
  y2 = rand()%H;
  y3 = rand()%H;

  if(x1<0) x1 = -x1;
  if(x2<0) x2 = -x2;
  if(x3<0) x3 = -x3;
  if(y1<0) y1 = -y1;
  if(y2<0) y2 = -y2;
  if(y3<0) y3 = -y3;

  rand_geo.trigon_fill(x1,y1,x2,y2,x3,y3);
}


function draw_ellipses(rand_geo) {
  var x, y;
  var rx, ry;

  x = rand()%W;
  y = rand()%H;
  rx = rand()%(W/2);
  ry = rand()%(H/2);

  if(x<0) x = -x;
  if(y<0) y = -y;
  if(rx<0) rx = -rx;
  if(ry<0) ry = -ry;

  rand_geo.ellipse_fill(x, y, rx, ry);
}

function randomize_color(rand_geo) {
  var r, g, b;

  r = rand() % 255;
  g = rand() % 255;
  b = rand() % 255;

  if(r<0) r = -r;
  if(g<0) g = -g;
  if(b<0) b = -b;

  rand_geo.color(r,g,b,150); 
}

rand_geo = new GeometryLayer(W,H);
//rand_geo.set_blit("alpha");
//rand_geo.set_blit_value(0.5);
//rand_geo.activate(true);
//geo.set_fps(24);
rand_geo.start();
scr.add_layer(rand_geo);

running = true;
rand_kbd = new KeyboardController();
rand_kbd.released_q = function() { quit(); }
rand_kbd.released_p = function() { drawer = draw_pixels; }
rand_kbd.released_t = function() { drawer = draw_triangles; }
rand_kbd.released_e = function() { drawer = draw_ellipses; }
register_controller( rand_kbd );

drawer = draw_triangles;

rand_bang = new TriggerController();
rand_bang.frame = function() {
  randomize_color(rand_geo);
  drawer(rand_geo);
}
register_controller(rand_bang);



