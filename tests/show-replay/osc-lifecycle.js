include("osc-config.js");
include("scene.js");
set_clear_all();
set_fps(25);
loadScene();

var osc = new OscController(server);
var seen = 0, replacement = 0;
osc.packet = function(sequence) {
    echo("replay: sequence " + sequence);
    ++seen;
    if (seen !== 1) return true;
    if (scenario === "stop-restart") osc.stop();
    if (scenario === "self-removal") {
        if (!rem_controller(osc) || rem_controller(osc))
            throw "self-removal was not applied exactly once";
    }
    if (scenario === "reset") reset();
    if (scenario === "quit") quit();
    echo("replay: callback completed " + scenario);
    return true;
};
osc.add_method("/packet", "i", "packet");
osc.start();
if (!register_controller(osc)) throw "registration failed";
osc.send_to("127.0.0.1", peer);
for (var warmup = 0; warmup < 4; ++warmup) run(0);
osc.send("/replay/ready", "");

// Keep the owner out of poll until the peer sees all 128 queue admissions.
var deadline = Date.now() + 4000;
while (scandir(".").indexOf("./osc-admitted") < 0) {
    if (Date.now() > deadline) throw "OSC admission handshake timed out";
    var next = Date.now() + 1;
    while (Date.now() < next) {}
}
run(0);
if (scenario === "stop-restart") {
    echo("replay: batch " + seen);
    run(0);
    echo("replay: batch " + seen);
    osc.start();
    osc.send("/replay/restarted", "");
    for (var frame = 0; seen < 129 && frame < 50; ++frame) run(0);
    osc.stop();
} else if (scenario === "quit") {
    run(0);
} else {
    var nextReceiver = new OscController(server);
    nextReceiver.packet = function(sequence) {
        echo("replay: replacement " + sequence);
        ++replacement;
        return true;
    };
    nextReceiver.add_method("/packet", "i", "packet");
    nextReceiver.start();
    if (!register_controller(nextReceiver)) throw "replacement registration failed";
    nextReceiver.send_to("127.0.0.1", peer);
    nextReceiver.send("/replay/rebound", "");
    for (var frame = 0; !replacement && frame < 50; ++frame) run(0);
    rem_controller(nextReceiver);
}
echo("replay: OSC " + scenario + " complete");
quit();
