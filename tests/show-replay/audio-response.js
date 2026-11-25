include("timing-config.js");
include("scene.js");
set_clear_all();
set_fps(25);
var base = image("base.png", "base");
base.start();
var marker = image("overlay.png", "audio marker");
marker.set_position(16, 12);
marker.start();
marker.deactivate();
var audio = new AudioJack("replay_pcm:pulse", 2048, 48000);
var encoder = new VideoEncoder(50, 64000, 75, 0);
encoder.add_audio(audio);
register_encoder(encoder);
encoder.start_filesave("response.ogg");
var osc = new OscController(serverPort);
for (var id = 0; id < 4; ++id) {
    var name = "timing" + id;
    osc[name] = function() { return true; };
    osc.add_method("/timing/" + id, "", name);
}
osc.start();
register_controller(osc);
osc.send_to("127.0.0.1", peerPort);
osc.send("/timing/ready", "");
for (var frame = 0; frame < frameCount; ++frame) {
    audio.fft();
    if (audio.get_harmonic(15) > 5) marker.activate();
    else marker.deactivate();
    run(0);
}
osc.stop();
encoder.stop_filesave();
echo("response: recorded and finished");
quit();
