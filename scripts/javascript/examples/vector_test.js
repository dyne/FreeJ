// simple test for CairoLayer

scr = new Screen();
if (!scr.initialized) {
    // screen hasn't been initialized yet, let's do it now
    scr.init(400,300);
    add_screen(scr);
} 

vec = new VectorLayer(400, 300);
vec.activate(true);
scr.add_layer(vec);

vec.beginPath();
vec.arc(50.5, 50.5, 80.25, 3.141592654, 1.4);
vec.stroke();
