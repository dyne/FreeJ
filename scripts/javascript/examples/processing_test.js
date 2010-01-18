// simple loader of a processing script

include("processing.js");

scr = new Screen("sdl");
if (!scr.is_initialized()) {
    scr.init(800,600);
    add_screen(scr);
} 

script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/ventaglio.pjs");
//script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/simple.processing");
//script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/recursion.pjs");
//script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/sinewave.pjs");
//script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/colorwheel.pjs");
//script = read_file("/home/jaromil/devel/freej/scripts/javascript/examples/setupdraw.pjs");


Processing(script);

