// Adapted from osc_test.js. The driver fills in isolated UDP port numbers.
var osc = new OscController("@SERVER@");
osc.replay_noargs = function() {
    echo("replay: OSC callback noargs");
    return true;
};
osc.replay_typed = function(i, f, s) {
    echo("replay: OSC callback " + i + "|" + f + "|" + s);
    return true;
};
osc.add_method("/test/noargs", "", "replay_noargs");
osc.add_method("/test", "ifs", "replay_typed");
osc.start();
register_controller(osc);
osc.send_to("127.0.0.1", "@PEER@");
osc.send("/replay/ready", "");
set_fps(25);
for (var frame = 0; frame < 25; ++frame) run(0);
osc.stop();
echo("replay: OSC quit");
quit();
