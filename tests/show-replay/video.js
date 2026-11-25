include("scene.js");
set_clear_all();
set_fps(25);

function present(name, logicalMilliseconds) {
    echo("replay: cue " + name + " " + logicalMilliseconds);
    // First render may precede the layer worker's publication. The observer
    // retains all ticks and verifies the transition and the settled frame.
    for (var tick = 0; tick < 8; ++tick) run(0);
}

var current = loadScene();
present("loaded", 0);
current.tint.set_parameter("Brightness", 0.75);
present("parameter", 40);
current.overlay.deactivate();
current.base.select();
present("hidden", 80);
current.overlay.activate();
current.overlay.select();
present("shown", 120);
current.overlay.down();
present("reordered", 160);

// Load the same script again and exercise replacement through the working
// layer operations. The original example's reset(filename) has its own probe.
current.base.stop();
current.overlay.stop();
include("scene.js");
var reloaded = loadScene();
present("reloaded", 200);
echo("replay: quit");
quit();
