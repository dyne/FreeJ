// Adapted from osc_test.js. The driver fills in isolated UDP port numbers.
var osc = new OscController("@SERVER@");
osc.replay_noargs = function() {
    echo("replay: OSC callback noargs");
    return true;
};
osc.replay_typed = function(i, f, s) {
    if (typeof i !== "number" || typeof f !== "number" || typeof s !== "string")
        throw "incorrect OSC value types";
    echo("replay: OSC callback " + i + "|" + f + "|" + s);
    return true;
};
osc.replay_reordered = function(s, f, i) {
    if (typeof s !== "string" || typeof f !== "number" || typeof i !== "number")
        throw "incorrect OSC value types";
    echo("replay: OSC callback " + s + "|" + f + "|" + i);
    return true;
};
osc.add_method("/test/noargs", "", "replay_noargs");
osc.add_method("/test", "ifs", "replay_typed");
osc.add_method("/reordered", "sfi", "replay_reordered");
osc.start();
register_controller(osc);
osc.send_to("127.0.0.1", "@PEER@");
osc.send("/replay/ready", "");
set_fps(25);
for (var frame = 0; frame < 25; ++frame) run(0);
osc.stop();
echo("replay: OSC quit");
quit();
